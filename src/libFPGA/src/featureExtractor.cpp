#include "featureExtractor.h"

using namespace cv;
using namespace std;

FeatureExtractor::FeatureExtractor()
{
    initialDevice();
}

FeatureExtractor::~FeatureExtractor()
{
    cleanup();
}

std::vector<float> FeatureExtractor::extractFeature(const cv::Mat &img)
{
    loadImageToBuffer(img);
    executeKernel();
    readDataBack();
    return vector<float>(output_reorder, output_reorder + output_config[output_w] * output_config[output_h] * output_config[output_n]);
}

bool FeatureExtractor::initialDevice()
{
    cl_platform_id platform_id = NULL;

    unsigned int weight_buf_size;

    // Define the kernel names used
    const char *knl_name_memRd = "memRead";
    const char *knl_name_conv = "coreConv";
    const char *knl_name_Pool = "maxPool";
    const char *knl_name_memWr = "memWrite";
    const char *knl_name_lrn = "lrn";

    // Connect to the desired platform
    platform_id = findPlatform(vendor_name);
    if (platform_id == NULL)
    {
        printf("ERROR: Unable to find the desired OpenCL platform.\n");
        return false;
    }

    // Query the available OpenCL device
    device.reset(getDevices(platform_id, DEVICE_TYPE, &num_devices));
    printf("\nPlatform: %s\n", getPlatformName(platform_id).c_str());
    printf("Using %d device(s)\n", num_devices);
    for (unsigned i = 0; i < num_devices; ++i)
    {
        printf("  Device %d: %s\n", i, getDeviceName(device[i]).c_str());
        displayDeviceInfo(device[i]);
    }

    // Create the context.
    context = clCreateContext(NULL, num_devices, device, NULL, NULL, &status);
    checkError(status, "Failed to create context");

    // Create Program Objects
    const char *kernel_file_name = kernelFile;

    // Create the program for all device. All devices execute the same kernel.
    program = createProgramFromFile(context, (const char *)kernel_file_name, device, num_devices);

    // Create per-device objects.
    que_memRd.reset(num_devices);
    que_conv.reset(num_devices);
    que_memWr.reset(num_devices);
    que_pool.reset(num_devices);
    knl_memRd.reset(num_devices);
    knl_conv.reset(num_devices);
    knl_memWr.reset(num_devices);
    knl_pool.reset(num_devices);
    knl_lrn.reset(num_devices);
    // For each layer a group of buffers are created to store the weights and bias
    weights_buf.reset(num_devices * LAYER_NUM);
    bias_buf.reset(num_devices * LAYER_NUM);
    // Two buffers (data and output) are used as ping-pong buffers for conv layers
    data_buf.reset(num_devices * MAX_BATCH_SIZE);
    output_buf.reset(num_devices * MAX_BATCH_SIZE);
    // Two buffers are used as ping-pong buffers for fc layers
    fc_1_buf.reset(num_devices);
    fc_2_buf.reset(num_devices);

    // Prepare compute data
    status = prepare();
    if (status == 1)
    {
        printf("Allocate memory for data and weights failed !!!\n");
        return false;
    }

    // Create qeues, kernels and mem objs
    for (unsigned i = 0; i < num_devices; ++i)
    {
        // Command queue
        que_memRd[i] = clCreateCommandQueue(context, device[i], CL_QUEUE_PROFILING_ENABLE, &status);
        checkError(status, "Failed to create command queue 0");
        que_conv[i] = clCreateCommandQueue(context, device[i], CL_QUEUE_PROFILING_ENABLE, &status);
        checkError(status, "Failed to create command queue 1");
        que_memWr[i] = clCreateCommandQueue(context, device[i], CL_QUEUE_PROFILING_ENABLE, &status);
        checkError(status, "Failed to create command queue 2");
        que_pool[i] = clCreateCommandQueue(context, device[i], CL_QUEUE_PROFILING_ENABLE, &status);
        checkError(status, "Failed to create command queue 3");

        // Kernel
        knl_memRd[i] = clCreateKernel(program, knl_name_memRd, &status);
        checkError(status, "Failed to create memRd kernel");

        knl_conv[i] = clCreateKernel(program, knl_name_conv, &status);
        checkError(status, "Failed to create conv kernel");

        knl_pool[i] = clCreateKernel(program, knl_name_Pool, &status);
        checkError(status, "Failed to create pooling kernel");

        knl_memWr[i] = clCreateKernel(program, knl_name_memWr, &status);
        checkError(status, "Failed to create memWr kernel");

        knl_lrn[i] = clCreateKernel(program, knl_name_lrn, &status);
        checkError(status, "Failed to create lrn kernel");

        // Mems
        // Create weight and bias buffers for each layer
        for (unsigned j = 0; j < LAYER_NUM; ++j)
        {

            weight_buf_size = layer_config[j][weight_w] * layer_config[j][weight_h] * layer_config[j][weight_n] * layer_config[j][weight_m];

            // Weights buffers for each layer
            weights_buf[i * LAYER_NUM + j] = clCreateBuffer(context, CL_MEM_READ_ONLY, weight_buf_size * sizeof(DTYPE), NULL, &status);
            checkError(status, "Failed to create buffer for weights in layer");

            // Bias buffers for each layer
            bias_buf[i * LAYER_NUM + j] = clCreateBuffer(context, CL_MEM_READ_ONLY, layer_config[j][bias_size] * sizeof(DTYPE), NULL, &status);
            checkError(status, "Failed to create buffer for bias in layer");

            // Initializing all weights buffers, blocking write is used
            status = clEnqueueWriteBuffer(que_memRd[i], weights_buf[i * LAYER_NUM + j], CL_TRUE, 0, weight_buf_size * sizeof(DTYPE), weight_conv[j], 0, NULL, NULL);
            checkError(status, "Failed to transfer weight");

            status = clEnqueueWriteBuffer(que_memRd[i], bias_buf[i * LAYER_NUM + j], CL_TRUE, 0, layer_config[j][bias_size] * sizeof(DTYPE), bias_conv[j], 0, NULL, NULL);
            checkError(status, "Failed to transfer bias");
        }

        // Create data buffers for each batch item
        for (unsigned j = 0; j < input_config[batch_size]; ++j)
        {
            // Input data buffers
            data_buf[i * input_config[batch_size] + j] = clCreateBuffer(context, CL_MEM_READ_WRITE, IN_BUF_SIZE * sizeof(DTYPE), NULL, &status);
            checkError(status, "Failed to create buffer for data in layer");

            // Output results buffers
            output_buf[i * input_config[batch_size] + j] = clCreateBuffer(context, CL_MEM_READ_WRITE, OUT_BUF_SIZE * sizeof(DTYPE), NULL, &status);
            checkError(status, "Failed to create buffer for output");
        }
        // Allocate fc buffers
        fc_1_buf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, FC_BUF_SIZE * sizeof(DTYPE), NULL, &status);
        checkError(status, "Failed to create buffer for data in fc layer");

        fc_2_buf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, FC_BUF_SIZE * sizeof(DTYPE), NULL, &status);
        checkError(status, "Failed to create buffer for data in fc layer");
    }
    return true;
}

void FeatureExtractor::executeKernel()
{
    size_t knl_memWr_global_size[3];
    size_t knl_memWr_local_size[3];
    size_t knl_lrn_global_size[3];
    size_t knl_lrn_local_size[3];

    unsigned int conv_output_num;
    unsigned int conv_loop_cnt;
    unsigned int conv_control;
    unsigned int pool_input_num;
    unsigned int pool_line_size;
    unsigned char pool_bypass;
    unsigned char batch_size_in_dim;
    unsigned char batch_indx_dim1;
    unsigned char batch_indx_dim2;
    // Execute the kernel
    scoped_array<cl_event> memRd_event(num_devices);
    scoped_array<cl_event> conv_event(num_devices);
    scoped_array<cl_event> pool_event(num_devices);
    scoped_array<cl_event> memWr_event(num_devices);
    scoped_array<cl_event> lrn_event(num_devices);

    // Recorde the excution time of each operation for each layer

    unsigned iter_num;
    unsigned short out_dim1xbatch;
    unsigned int out_dim1x2xbatch;
    unsigned char padding_offset;
    unsigned char conv_group_num_dim1, conv_group_num_dim2;
    unsigned char conv_win_size_dim1, conv_win_size_dim2;
    unsigned int conv_win_size_dim1x2x3;
    unsigned char conv_group_rem_dim1, conv_group_rem_dim2;
    unsigned int conv_group_rem_dim1x2x3;
    unsigned short data_dim1x2;
    unsigned char weight_dim1x2;
    unsigned int weight_dim1x2x3;
    unsigned short weight_dim4_div_LaneNum;
    unsigned int pic_num = 1;

    // Kernel excutions main loops
    for (unsigned i = 0; i < num_devices; ++i)
    {

        // loadImageToBuffer(pic_num);

        // Recorde the start time
        t.start();

        // Each iteration excutes one layer convolution
        // MemRd -> Conv(Relu) -> (MaxPool) -> MemWr -> (Lrn)
        for (unsigned char j = 0; j < LAYER_NUM; ++j)
        {

            if (j < CONV_NUM)
                iter_num = input_config[batch_size]; // for conv layers, process by batch_size time
            else
                iter_num = 1; // for FC layers, process only one time

            // Each iteration process one item in batch
            for (unsigned k = 0; k < iter_num; ++k)
            {
                // Set Arguments
                //
                // Set knl_memRd arguments.
                unsigned argi = 0;

                // Convolution tasks (conv_x,conv_y) are divided into multiple groups
                conv_group_num_dim1 = ceil((float)layer_config[j][conv_x] / CONV_GP_SIZE_X);
                conv_group_num_dim2 = ceil((float)layer_config[j][conv_y] / CONV_GP_SIZE_Y);
                if (layer_config[j][conv_x] == 1)
                {
                    conv_win_size_dim1 = layer_config[j][weight_w];
                    conv_group_rem_dim1 = layer_config[j][weight_w];
                }
                else
                {
                    conv_win_size_dim1 = layer_config[j][weight_w] + (CONV_GP_SIZE_X - 1) * layer_config[j][conv_stride];
                    if (layer_config[j][conv_x] % CONV_GP_SIZE_X == 0)
                        conv_group_rem_dim1 = CONV_GP_SIZE_X * layer_config[j][weight_w];
                    else
                        conv_group_rem_dim1 = layer_config[j][conv_x] % CONV_GP_SIZE_X * layer_config[j][weight_w];
                }
                conv_win_size_dim2 = layer_config[j][weight_h];
                conv_group_rem_dim2 = layer_config[j][weight_h];
                conv_win_size_dim1x2x3 = conv_win_size_dim1 * conv_win_size_dim2 * layer_config[j][weight_n];
                conv_group_rem_dim1x2x3 = conv_group_rem_dim1 * conv_group_rem_dim2 * layer_config[j][weight_n];

                weight_dim4_div_LaneNum = layer_config[j][weight_m] / LANE_NUM;
                data_dim1x2 = layer_config[j][data_w] * layer_config[j][data_h];
                weight_dim1x2 = layer_config[j][weight_w] * layer_config[j][weight_h];
                weight_dim1x2x3 = layer_config[j][weight_w] * layer_config[j][weight_h] * layer_config[j][weight_n];

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][data_w]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][data_h]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_ushort), &data_dim1x2);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][weight_w]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][weight_h]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_ushort), &layer_config[j][weight_n]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_ushort), &weight_dim4_div_LaneNum);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &weight_dim1x2);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uint), &weight_dim1x2x3);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][conv_x]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                //status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][conv_y]);
                //checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][conv_stride]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][conv_padding]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &layer_config[j][conv_split]);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &conv_group_num_dim1);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &conv_group_num_dim2);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &conv_group_rem_dim1);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                //status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &conv_group_rem_dim2);
                //checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uint), &conv_group_rem_dim1x2x3);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &conv_win_size_dim1);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uchar), &conv_win_size_dim2);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_uint), &conv_win_size_dim1x2x3);
                checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);
                // Select the kernel input mem object source
                // data_buf -> conv1 -> output_buf -> lrn1 -> data_buf -> conv2 -> output_buf -> lrn2 -> data_buf
                // -> conv3 -> output_buf -> conv4 -> output_buf -> ...
                if (layer_config[j][memrd_src] == 0)
                {
                    status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_mem), &data_buf[i * input_config[batch_size] + k]);
                    checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);
                }
                else if (layer_config[j][memrd_src] == 1)
                {
                    status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_mem), &output_buf[i * input_config[batch_size] + k]);
                    checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);
                }
                else if (layer_config[j][memrd_src] == 2)
                {
                    status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_mem), &fc_1_buf[i]);
                    checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);
                }
                else // 3
                {
                    status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_mem), &fc_2_buf[i]);
                    checkError(status, "Failed to set argument %d of kernel memRd", argi - 1);
                }

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_mem), &weights_buf[i * LAYER_NUM + j]);
                checkError(status, "Failed to set argument %d kernel memRd", argi - 1);

                status = clSetKernelArg(knl_memRd[i], argi++, sizeof(cl_mem), &bias_buf[i * LAYER_NUM + j]);
                checkError(status, "Failed to set argument %d kernel memRd", argi - 1);

                //  Set knl_conv arguments.
                argi = 0;

                conv_loop_cnt = layer_config[j][weight_w] * layer_config[j][weight_h] * layer_config[j][weight_n] / VEC_SIZE;
                conv_output_num = layer_config[j][conv_x] * layer_config[j][conv_y] * layer_config[j][weight_m] / LANE_NUM; // new weight_m is divisible by LANE_NUM
                conv_control = (layer_config[j][conv_relu] & 0x01) | (((~layer_config[j][pool_on]) & 0x01) << 1);

                status = clSetKernelArg(knl_conv[i], argi++, sizeof(cl_uint), &conv_output_num);
                checkError(status, "Failed to set argument %d of kernel conv", argi - 1);

                status = clSetKernelArg(knl_conv[i], argi++, sizeof(cl_uint), &conv_loop_cnt);
                checkError(status, "Failed to set argument %d of kernel conv", argi - 1);

                status = clSetKernelArg(knl_conv[i], argi++, sizeof(cl_uint), &conv_control);
                checkError(status, "Failed to set argument %d of kernel conv", argi - 1);

                status = clSetKernelArg(knl_conv[i], argi++, sizeof(cl_char), &precision_config[j][frac_w]);
                checkError(status, "Failed to set argument %d of kernel conv", argi - 1);

                status = clSetKernelArg(knl_conv[i], argi++, sizeof(cl_char), &precision_config[j][frac_din]);
                checkError(status, "Failed to set argument %d of kernel conv", argi - 1);

                status = clSetKernelArg(knl_conv[i], argi++, sizeof(cl_char), &precision_config[j][frac_dout]);
                checkError(status, "Failed to set argument %d of kernel conv", argi - 1);

                //  Set knl_pool arguments.
                if (layer_config[j][pool_on])
                {
                    argi = 0;

                    pool_input_num = layer_config[j][conv_x] * layer_config[j][conv_y] * layer_config[j][weight_m] / LANE_NUM; // new weight_m is divisible by LANE_NUM
                    pool_line_size = layer_config[j][conv_x];
                    status = clSetKernelArg(knl_pool[i], argi++, sizeof(cl_uint), &pool_input_num);
                    checkError(status, "Failed to set argument %d of kernel pool", argi - 1);

                    status = clSetKernelArg(knl_pool[i], argi++, sizeof(cl_uchar), &pool_line_size);
                    checkError(status, "Failed to set argument %d of kernel pool", argi - 1);

                    status = clSetKernelArg(knl_pool[i], argi++, sizeof(cl_uchar), &layer_config[j][pool_size]);
                    checkError(status, "Failed to set argument %d of kernel pool", argi - 1);

                    status = clSetKernelArg(knl_pool[i], argi++, sizeof(cl_uchar), &layer_config[j][pool_stride]);
                    checkError(status, "Failed to set argument %d of kernel pool", argi - 1);
                }

                //  Set knl_memWr arguments.
                argi = 0;
                unsigned char batch_size_in_dim_log;
                unsigned char mask = 0xff;
                unsigned char memWr_dim1, memWr_dim2;
                unsigned short memWr_dim3;

                pool_bypass = (~layer_config[j][pool_on]) & 0x01;

                if (layer_config[j][pool_on] == 1)
                {
                    memWr_dim1 = layer_config[j][pool_x];
                    memWr_dim2 = layer_config[j][pool_y];
                    memWr_dim3 = layer_config[j][pool_z];
                }
                else
                {
                    memWr_dim1 = layer_config[j][conv_x];
                    memWr_dim2 = layer_config[j][conv_y];
                    memWr_dim3 = layer_config[j][conv_z];
                }

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_uchar), &memWr_dim1);
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_uchar), &memWr_dim2);
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_ushort), &memWr_dim3); // pool_z equals original weight_m
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                if (j == (CONV_NUM - 1))
                { // For last Conv Layer, combine all batch data into one fc buffer
                    if (input_config[batch_size] == 1)
                    {
                        batch_size_in_dim = 1;
                        batch_indx_dim1 = 0;
                        batch_indx_dim2 = 0;
                    }
                    else
                    {
                        batch_size_in_dim = log(input_config[batch_size]) / log(2);
                        batch_size_in_dim_log = log(batch_size_in_dim) / log(2);
                        batch_indx_dim1 = k & (~((mask >> batch_size_in_dim_log) << batch_size_in_dim_log));
                        batch_indx_dim2 = k >> batch_size_in_dim_log;
                    }
                }
                else
                { // Normal WR Operations
                    batch_size_in_dim = 1;
                    batch_indx_dim1 = 0;
                    batch_indx_dim2 = 0;
                }

                out_dim1xbatch = memWr_dim1 * batch_size_in_dim;
                out_dim1x2xbatch = memWr_dim1 * memWr_dim2 * batch_size_in_dim * batch_size_in_dim;
                padding_offset = (layer_config[j][weight_m] - layer_config_original[j][weight_m]) / 2;

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_ushort), &out_dim1xbatch);
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_uint), &out_dim1x2xbatch);
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_uchar), &batch_indx_dim1);
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_uchar), &batch_indx_dim2);
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_uchar), &pool_bypass);
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_uchar), &padding_offset);
                checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);

                // Select the kernel output mem object source
                if (layer_config[j][memwr_dst] == 0)
                {
                    status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_mem), &data_buf[i * input_config[batch_size] + k]);
                    checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);
                }
                else if (layer_config[j][memwr_dst] == 1)
                {
                    status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_mem), &output_buf[i * input_config[batch_size] + k]);
                    checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);
                }
                else if (layer_config[j][memwr_dst] == 2)
                {
                    status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_mem), &fc_1_buf[i]);
                    checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);
                }
                else // 3
                {
                    status = clSetKernelArg(knl_memWr[i], argi++, sizeof(cl_mem), &fc_2_buf[i]);
                    checkError(status, "Failed to set argument %d of kernel memWr", argi - 1);
                }

                //  Set knl_lrn arguments.
                if (layer_config[j][lrn_on])
                {
                    argi = 0;

                    status = clSetKernelArg(knl_lrn[i], argi++, sizeof(cl_uchar), &layer_config[j][pool_x]);
                    checkError(status, "Failed to set argument %d of kernel lrn", argi - 1);

                    status = clSetKernelArg(knl_lrn[i], argi++, sizeof(cl_uchar), &layer_config[j][pool_y]);
                    checkError(status, "Failed to set argument %d of kernel lrn", argi - 1);

                    status = clSetKernelArg(knl_lrn[i], argi++, sizeof(cl_char), &precision_config[j][frac_dout]);
                    checkError(status, "Failed to set argument %d of kernel lrn", argi - 1);

                    status = clSetKernelArg(knl_lrn[i], argi++, sizeof(cl_mem), &output_buf[i * input_config[batch_size] + k]);
                    checkError(status, "Failed to set argument %d of kernel lrn", argi - 1);

                    status = clSetKernelArg(knl_lrn[i], argi++, sizeof(cl_mem), &data_buf[i * input_config[batch_size] + k]);
                    checkError(status, "Failed to set argument %d of kernel lrn", argi - 1);
                }

                // Excutes Kernel
                //
                // if (k == 0 && pic_num == 1)
                //     printf("\nExecuting Layer %d:\n", j + 1);

                // kernel memRd
                // if (k == 0 && pic_num == 1)
                //     printf("\nLaunching single work-item kernel winbuffer\n");

                status = clEnqueueTask(que_memRd[i], knl_memRd[i], 0, NULL, &memRd_event[i]);
                checkError(status, "Failed to launch kernel memRD kernel");

                // kernel conv
                // if (k == 0 && pic_num == 1)
                //     printf("\nLaunching single work-item kernel Conv\n");

                status = clEnqueueTask(que_conv[i], knl_conv[i], 0, NULL, &conv_event[i]);
                checkError(status, "Failed to launch kernel conv kernel");

                // kernel pool
                if (layer_config[j][pool_on])
                {
                    status = clEnqueueTask(que_pool[i], knl_pool[i], 0, NULL, &pool_event[i]);
                    checkError(status, "Failed to launch kernel pooling");
                    // if (k == 0 && pic_num == 1)
                    //     printf("\nLaunching single work-item kernel Pooling\n");
                }

                // kernel memWr
                knl_memWr_global_size[0] = memWr_dim1;
                knl_memWr_global_size[1] = memWr_dim2;
                knl_memWr_global_size[2] = layer_config[j][weight_m]; // pool_z equals original weight_m, new weight_m is divisible by LANE_NUM
                knl_memWr_local_size[0] = 1;
                knl_memWr_local_size[1] = 1;
                knl_memWr_local_size[2] = LANE_NUM;

                // if (k == 0 && pic_num == 1)
                //     printf("\nLaunching kernel MemWr with local size: %d, %d, %d  (global size: %d, %d, %d)\n",
                //            (int)knl_memWr_local_size[0], (int)knl_memWr_local_size[1], (int)knl_memWr_local_size[2],
                //            (int)knl_memWr_global_size[0], (int)knl_memWr_global_size[1], (int)knl_memWr_global_size[2]);

                status = clEnqueueNDRangeKernel(que_memWr[i], knl_memWr[i], 3, NULL, knl_memWr_global_size, knl_memWr_local_size, 0, NULL, &memWr_event[i]);

                checkError(status, "Failed to launch kernel memWr");

                // kernel lrn
                if (layer_config[j][lrn_on])
                {

                    knl_lrn_global_size[0] = layer_config[j][pool_x];
                    knl_lrn_global_size[1] = layer_config[j][pool_y];
                    knl_lrn_global_size[2] = layer_config[j][pool_z] / VEC_SIZE;
                    knl_lrn_local_size[0] = 1;
                    knl_lrn_local_size[1] = 1;
                    knl_lrn_local_size[2] = layer_config[j][pool_z] / VEC_SIZE;

                    // if (k == 0 && pic_num == 1)
                    //     printf("\nLaunching kernel lrn with local size: %d, %d, %d  (global size: %d, %d, %d)\n", (int)knl_lrn_local_size[0], (int)knl_lrn_local_size[1], (int)knl_lrn_local_size[2], (int)knl_lrn_global_size[0], (int)knl_lrn_global_size[1], (int)knl_lrn_global_size[2]);

                    status = clEnqueueNDRangeKernel(que_memWr[i], knl_lrn[i], 3, NULL, knl_lrn_global_size, knl_lrn_local_size, 0, NULL, &lrn_event[i]);
                    checkError(status, "Failed to launch kernel lrn");
                }

                // Wait for all kernel to finish
                if (layer_config[j][lrn_on])
                {
                    status = clWaitForEvents(num_devices, lrn_event);
                    checkError(status, "Failed to finish lrn event");
                }
                else
                {
                    status = clWaitForEvents(num_devices, memWr_event);
                    checkError(status, "Failed to finish memWR event");
                }

                // Must release event object to avoid performance degeneration !!!
                clReleaseEvent(memRd_event[i]);
                checkError(status, "Failed to release memRD event object");
                clReleaseEvent(conv_event[i]);
                checkError(status, "Failed to release Conv event object");
                clReleaseEvent(memWr_event[i]);
                checkError(status, "Failed to release memWR event object");
                if (layer_config[j][pool_on])
                {
                    status = clReleaseEvent(pool_event[i]);
                    checkError(status, "Failed to release pool event object");
                }
                if (layer_config[j][lrn_on])
                {
                    status = clReleaseEvent(lrn_event[i]);
                    checkError(status, "Failed to release lrn event object");
                }

            } // end of batch iteration

        } // end of layer iteration

        t.stop();
        time = t.get_time_s();
        // printf("Total runtime: %fs \n\n", time);

        // readDataBack();

    } // end of board iteration

    // Release resource
    // cleanup();
}

void FeatureExtractor::readDataBack()
{
    unsigned int read_buf_size;
    scoped_array<cl_event> finish_event(num_devices);
    // Read back the results from the device to verify the output
    // Note：only device0 is used here
    if (num_devices != 1)
        printf("Warnning: only the result from device0 will be verified!!!\n\n");

    // Select whith item you would like to compare with the golden ref
    // Item num start from 0
    unsigned batch_item_num = 0;
    if (batch_item_num > (input_config[batch_size] - 1))
    {
        printf("Error: wrong configuration，can't verify the item since it is layer than batch size !!!\n\n");
    }

    if (LAYER_NUM < CONV_NUM)
    { // verify conv results
        read_buf_size = output_config[output_w] * output_config[output_h] * output_config[output_n];
    }
    else // verify the last conv and all fc results
        read_buf_size = output_config[output_w] * output_config[output_h] * output_config[output_n] * input_config[batch_size];

    // For the last conv layer and all fc layers, read result from one of the fc buffers
    if (layer_config[LAYER_NUM - 1][memwr_dst] == 2)
    {
        // printf("\nCopyed all batched results from fc_1 buffers.\n");
        status = clEnqueueReadBuffer(que_memWr[0], fc_1_buf[0], CL_FALSE, // read from device0
                                     0, sizeof(DTYPE) * read_buf_size, (void *)output, 0, NULL, &finish_event[0]);
        checkError(status, "Failed to set transfer output data");
    }
    else if (layer_config[LAYER_NUM - 1][memwr_dst] == 3)
    {
        // printf("\nCopyed all batched results from fc_2 buffers.\n");
        status = clEnqueueReadBuffer(que_memWr[0], fc_2_buf[0], CL_FALSE, // read from device0
                                     0, sizeof(DTYPE) * read_buf_size, (void *)output, 0, NULL, &finish_event[0]);
        checkError(status, "Failed to set transfer output data");
    }
    // For other layers, read results from data and output buffers
    else if (layer_config[LAYER_NUM - 1][memwr_dst] ^ layer_config[LAYER_NUM - 1][lrn_on])
    { // if lrn is used, the mem dst is changed back to src
        printf("\nCopyed one result from NO.%d output buffers.\n", batch_item_num);
        status = clEnqueueReadBuffer(que_memWr[0], output_buf[batch_item_num], CL_FALSE, // read from device0
                                     0, sizeof(DTYPE) * read_buf_size, (void *)output, 0, NULL, &finish_event[0]);
        checkError(status, "Failed to set transfer output data");
    }
    else
    {
        printf("\nCopyed one results from NO.%d data buffers.\n", batch_item_num);
        status = clEnqueueReadBuffer(que_memWr[0], data_buf[batch_item_num], CL_FALSE, // read from device0
                                     0, sizeof(DTYPE) * read_buf_size, (void *)output, 0, NULL, &finish_event[0]);
        checkError(status, "Failed to set transfer output data");
    }

    // Wait for reads to finish
    clWaitForEvents(1, &finish_event[0]);
    clReleaseEvent(finish_event[0]);
    checkError(status, "Failed to release finish event object");

    if (LAYER_NUM >= CONV_NUM)
    { //Select with batch item you would like to verify from the last conv and all fc output
        // printf("Selected item = %d from the combined batch results in fc buffers\n", batch_item_num);
        extractOutput(output, output_one_item, batch_item_num, input_config[batch_size], output_config[output_w], output_config[output_h], output_config[output_n]);
    }
    else
    {
        if (layer_config[LAYER_NUM - 1][pool_on] == 1)
            extractOutput(output, output_one_item, 0, 1, layer_config[LAYER_NUM - 1][pool_x], layer_config[LAYER_NUM - 1][pool_y], layer_config[LAYER_NUM - 1][pool_z]);
        else
            extractOutput(output, output_one_item, 0, 1, layer_config[LAYER_NUM - 1][conv_x], layer_config[LAYER_NUM - 1][conv_y], layer_config[LAYER_NUM - 1][conv_z]);
    }

    reorderOutput(output_one_item, output_reorder, output_config[output_w], output_config[output_h], output_config[output_n]);
}

void FeatureExtractor::loadImageToBuffer(const cv::Mat &img)
{
    // load picture from files
    Mat img2;
    resize(img, img2, Size(layer_config_original[0][data_w], layer_config_original[0][data_h]));
    // convert to 8-bit fixed-point
    img2.convertTo(img2, CV_8SC3);
    // reorder channel sequence from RGB to GBR
    DTYPE *data_ptr = (DTYPE *)img2.data;
    unsigned int w, h, c;
    unsigned int k = 0;
    for (h = 0; h < layer_config_original[0][data_h]; h++)
    {
        for (w = 0; w < layer_config_original[0][data_w]; w++)
        {
            for (c = 0; c < layer_config_original[0][data_n]; c++)
            {
                image[c * layer_config_original[0][data_w] * layer_config_original[0][data_h] + h * layer_config_original[0][data_w] + w] = data_ptr[k];
                k++;
            }
        }
    }

    // // load binary from files
    // unsigned file_size;
    // // load image from binary files
    // ifstream bin_file_r;
    // bin_file_r.open("/home/sh/data/data_vgg/image.dat", ios::in | ios::binary);

    // if (bin_file_r.is_open())
    // {
    //     //Get file size
    //     bin_file_r.seekg(0, bin_file_r.end);
    //     file_size = bin_file_r.tellg();
    //     bin_file_r.seekg(0, bin_file_r.beg);

    //     bin_file_r.read((char *)image, sizeof(DTYPE) * IMAGE_FILE_SIZE);
    //     printf("\n%d bytes image data read from binary files\n", file_size);
    //     if (IMAGE_FILE_SIZE != (file_size / (sizeof(DTYPE))))
    //         printf("Warning: image file size does not match user configuration !!!\n");
    //     bin_file_r.close();
    // }
    // else
    //     printf("Image file does not exits !!!\n");

    // Vectorize the input image by a factor of VEC_SIZE
    for (unsigned n = 0; n < layer_config[0][data_n] / VEC_SIZE; n++)
    {
        for (unsigned i = 0; i < layer_config[0][data_h]; i++)
        {
            for (unsigned j = 0; j < layer_config[0][data_w]; j++)
            {
                for (unsigned k = 0; k < VEC_SIZE; k++)
                {
                    if ((n * VEC_SIZE + k) < layer_config_original[0][data_n])
                    { //  when layer_config[0][data_n] > layer_config_original[0][data_n], only copy valid pixels
                        data_init[n * VEC_SIZE * layer_config[0][data_h] * layer_config[0][data_w] + i * layer_config[0][data_w] * VEC_SIZE + j * VEC_SIZE + k] = (DTYPE)image[(n * VEC_SIZE + k) * layer_config[0][data_h] * layer_config[0][data_w] + i * layer_config[0][data_w] + j];
                    }
                }
            }
        }
    }
    for (unsigned i = 0; i < num_devices; ++i)
    {
        // Create data buffers for each batch item
        for (unsigned j = 0; j < input_config[batch_size]; ++j)
        {
            // Load image data into buffers
            status = clEnqueueWriteBuffer(que_memRd[i], data_buf[i * input_config[batch_size] + j], CL_TRUE, 0, (layer_config[0][data_w] * layer_config[0][data_h] * layer_config[0][data_n]) * sizeof(DTYPE), data_init, 0, NULL, NULL);
            checkError(status, "Failed to transfer input image");
        }
    }
}

// Read all input data and golden ref data
int FeatureExtractor::prepare()
{
    // Load Image data, CNN net weights and golden_results
    ifstream bin_file_r;
    unsigned file_size;
    unsigned weight_size;
    unsigned output_size;
    unsigned godref_size;
    int ptr = 0; // original weight and bias offset for each layer

    unsigned char conv_win_size_dim1, conv_win_size_dim2;

    unsigned padding_offset[LAYER_NUM];

    // Parameter initialization and safty check
    for (unsigned ll = 0; ll < LAYER_NUM; ll++)
    {

        // First, backup the original layer configurations
        for (unsigned ii = 0; ii < NUM_CONFIG_ITEM; ii++)
        {
            layer_config_original[ll][ii] = layer_config[ll][ii];
        }

        // Second, perform padding on dim4, when it is not divisible by LANE_NUM
        if (layer_config[ll][weight_m] % LANE_NUM != 0)
        {
            printf("\nWarnning: layer-%d requires padding zero-value feature maps for give param LANE_NUM=%d\n", ll + 1, LANE_NUM);
            layer_config[ll][weight_m] = ceil((float)layer_config[ll][weight_m] / LANE_NUM) * LANE_NUM;
            layer_config[ll][bias_size] = layer_config[ll][weight_m];
            printf("      original num of feature maps is %d, new value is %d\n", layer_config_original[ll][weight_m], layer_config[ll][weight_m]);

            // padding of weight on dim4 is needed
            padding_offset[ll] = layer_config[ll][weight_m] - layer_config_original[ll][weight_m];
            // check if evenly padding on two sides is possible
            if (((layer_config[ll][weight_m] / LANE_NUM) % 2 != 0) & (layer_config[ll][conv_split] == 1))
            {
                printf("Error: could not perform padding for split mode, weight_m/LANE_NUM must be divisible by 2 !!!\n\n");
                return 1;
            }
            else
            { // padding zeros evenly on two sides of dim4
                padding_offset[ll] = padding_offset[ll] / 2;
                printf("      padding_offset=%d (layer=%d)\n\n", padding_offset[ll], ll + 1);
            }
        }
        else
        {
            padding_offset[ll] = 0;
        }

        // Check parameters
        if (ll == 0)
        { // check parameters for layer-1
            if (input_config[image_w] != layer_config_original[ll][data_w] || input_config[image_h] != layer_config_original[ll][data_h] || input_config[image_n] != layer_config_original[ll][data_n] || input_config[image_n] != layer_config_original[ll][weight_n])
            {
                printf("Error: incorrect layer configuration for layer-%d !!!\n", ll + 1);
                //return 1;
            }

            if ((layer_config_original[ll][weight_n] != input_config[image_n]))
            {
                printf("\nError: incorrect layer configuration for layer-%d !!!\n", ll + 1);
                //return 1;
            }
        }
        else
        { // other layers

            // Currently weight_n must be divisible by VEC_SIZE (for first layer, padding is performed when weight_n is not divisible by VEC_SIZE)
            if ((layer_config[ll][weight_n] % VEC_SIZE) != 0)
            {
                printf("\nError: incorrect setting of parameter VEC_SIZE !!!\n");
                return 1;
            }
            if ((layer_config_original[ll][data_n] != layer_config_original[ll - 1][conv_z]))
            {
                printf("\nError: incorrect setting of convolution input/output size for layer-%d!!!\n", ll + 1);
                return 1;
            }
        }
        if ((layer_config_original[ll][conv_x] != (layer_config_original[ll][data_w] - layer_config_original[ll][weight_w] + 2 * layer_config_original[ll][conv_padding]) / layer_config_original[ll][conv_stride] + 1) || (layer_config_original[ll][conv_y] != (layer_config_original[ll][data_h] - layer_config_original[ll][weight_h] + 2 * layer_config_original[ll][conv_padding]) / layer_config_original[ll][conv_stride] + 1) || (layer_config_original[ll][conv_z] != layer_config_original[ll][weight_m]))
        {
            printf("\nError: incorrect setting of convolution output size or filter params for layer-%d!!!\n", ll + 1);
            return 1;
        }
        if (layer_config_original[ll][pool_on] && ((layer_config_original[ll][pool_x] != (layer_config_original[ll][conv_x] - layer_config_original[ll][pool_size]) / layer_config_original[ll][pool_stride] + 1) || (layer_config_original[ll][pool_y] != (layer_config_original[ll][conv_y] - layer_config_original[ll][pool_size]) / layer_config_original[ll][pool_stride] + 1) || (layer_config_original[ll][pool_z] != layer_config_original[ll][conv_z])))
        {
            printf("\nError: incorrect setting of pooling input/output size for layer-%d!!!\n", ll + 1);
            return 1;
        }

        if (layer_config[ll][conv_x] == 1)
        { // when only one group for FC layer
            conv_win_size_dim1 = layer_config[ll][weight_w];
        }
        else
        {
            conv_win_size_dim1 = layer_config[ll][weight_w] + (CONV_GP_SIZE_X - 1) * layer_config[ll][conv_stride];
        }
        conv_win_size_dim2 = layer_config[ll][weight_h];
        // check win_buffer size
        if (conv_win_size_dim1 * conv_win_size_dim2 * layer_config[ll][weight_n] / VEC_SIZE > WIN_BUF_SIZE)
        {

            printf("Error: required win_buffer size is %d, configured size is %d \n", conv_win_size_dim1 * conv_win_size_dim2 * layer_config[ll][weight_n] / VEC_SIZE, WIN_BUF_SIZE);
            return 1;
        }
        // check weight_buffer size
        if (layer_config[ll][weight_w] * layer_config[ll][weight_h] * layer_config[ll][weight_n] / VEC_SIZE > WEIGHT_BUF_SIZE)
        {

            printf("Error: required weight_buffer size is %d, configured size is %d \n", layer_config[ll][weight_w] * layer_config[ll][weight_h] * layer_config[ll][weight_n] / VEC_SIZE, WEIGHT_BUF_SIZE);
            return 1;
        }
    }

    // image and weight files
    weights = (DTYPE *)alignedMalloc(sizeof(DTYPE) * WEIGHTS_FILE_SIZE, DMA_ALIGNMENT);
    image = (DTYPE *)alignedMalloc(sizeof(DTYPE) * IMAGE_FILE_SIZE, DMA_ALIGNMENT);

    // input data buffers
    // padding the input RGB image with extra number of zeros channels, so that data_n/weight_n is divisible by VEC_SIZE
    layer_config[0][weight_n] = ceil((float)layer_config[0][weight_n] / VEC_SIZE) * VEC_SIZE;
    layer_config[0][data_n] = layer_config[0][weight_n];

    data_init = (DTYPE *)alignedMalloc(sizeof(DTYPE) * layer_config[0][data_w] * layer_config[0][data_h] * layer_config[0][data_n], DMA_ALIGNMENT);
    memset(data_init, 0, sizeof(DTYPE) * layer_config[0][data_w] * layer_config[0][data_h] * layer_config[0][data_n]); // fill non-RGB dims with 0

    // final results
    if (LAYER_NUM >= CONV_NUM) // For last conv and all fc layers, all batch results are read back
        output_size = output_config[output_w] * output_config[output_h] * output_config[output_n] * input_config[batch_size];
    else // For other conv layers, only one item of
        output_size = output_config[output_w] * output_config[output_h] * output_config[output_n];

    godref_size = output_config[output_w] * output_config[output_h] * output_config[output_n];

    output = (DTYPE *)alignedMalloc(sizeof(DTYPE) * output_size, DMA_ALIGNMENT);          // vectorized results
    output_one_item = (DTYPE *)alignedMalloc(sizeof(DTYPE) * godref_size, DMA_ALIGNMENT); // one item extracted from batch results
    golden_ref = (DTYPE *)alignedMalloc(sizeof(DTYPE) * godref_size, DMA_ALIGNMENT);
    output_reorder = (DTYPE *)alignedMalloc(sizeof(DTYPE) * godref_size, DMA_ALIGNMENT); // reordered results for verifying

    if (weights == NULL || image == NULL || golden_ref == NULL || data_init == NULL || output == NULL || output_one_item == NULL || output_reorder == NULL)
    {
        printf("Not enough memory !!!");
        alignedFree(weights);
        alignedFree(image);
        alignedFree(data_init);
        alignedFree(golden_ref);
        alignedFree(output_one_item);
        alignedFree(output);
        alignedFree(output_reorder);

        return 1;
    }

    // weights and bias	buffers
    for (int j = 0; j < LAYER_NUM; j++)
    {

        weight_size = (layer_config[j][weight_w] * layer_config[j][weight_h] * layer_config[j][weight_n] * layer_config[j][weight_m]);
        weight_conv[j] = (DTYPE *)alignedMalloc(sizeof(DTYPE) * weight_size, DMA_ALIGNMENT);
        bias_conv[j] = (DTYPE *)alignedMalloc(sizeof(DTYPE) * layer_config[j][bias_size], DMA_ALIGNMENT);

        memset(weight_conv[j], 0, sizeof(DTYPE) * weight_size);              // reset all value (include padding value) to zero
        memset(bias_conv[j], 0, sizeof(DTYPE) * layer_config[j][bias_size]); // reset all value (include padding value) to zero

        if (weight_conv[j] == NULL || bias_conv[j] == NULL)
        {
            printf("Not enough memory !!!");
            for (int i = 0; i <= j; i++)
            {
                alignedFree(weight_conv[i]);
                alignedFree(bias_conv[i]);
            }
            return 1;
        }
    }

    // Weights
    bin_file_r.open(weight_file_path, ios::in | ios::binary);

    if (bin_file_r.is_open())
    {
        //Get file size
        bin_file_r.seekg(0, bin_file_r.end);
        file_size = bin_file_r.tellg();
        bin_file_r.seekg(0, bin_file_r.beg);

        bin_file_r.read((char *)weights, sizeof(DTYPE) * WEIGHTS_FILE_SIZE);
        // printf("\n%d total weights read \n", file_size / ((int)sizeof(DTYPE)));
        if (WEIGHTS_FILE_SIZE != (file_size / (sizeof(DTYPE))))
            printf("Warning: weight file size does not match user configuration !!!\n");
        bin_file_r.close();
    }
    else
        printf("Weights file does not exits !!!\n");

    // Layer-1
    reorderWeights(weights, weight_conv[0], layer_config[0][weight_w], layer_config[0][weight_h], layer_config[0][weight_n], layer_config[0][weight_m], layer_config_original[0][weight_n], layer_config_original[0][weight_m], ptr, padding_offset[0], VEC_SIZE, LANE_NUM);
    ptr += layer_config[0][weight_w] * layer_config[0][weight_h] * layer_config_original[0][weight_n] * layer_config_original[0][weight_m];
    reorderBias(weights, bias_conv[0], ptr, padding_offset[0], layer_config[0][bias_size], layer_config_original[0][bias_size], LANE_NUM);
    ptr += layer_config_original[0][bias_size];

    // Other layers
    for (unsigned j = 1; j < LAYER_NUM; j++)
    {

        if (ptr + layer_config[j][weight_w] * layer_config[j][weight_h] * layer_config_original[j][weight_n] * layer_config_original[j][weight_m] > WEIGHTS_FILE_SIZE)
        {
            printf("Error：exceed weight file size !!!\n");
            return 1;
        }

        reorderWeights(weights, weight_conv[j], layer_config[j][weight_w], layer_config[j][weight_h], layer_config[j][weight_n], layer_config[j][weight_m], layer_config_original[j][weight_n], layer_config_original[j][weight_m], ptr, padding_offset[j], VEC_SIZE, LANE_NUM);
        ptr += layer_config[j][weight_w] * layer_config[j][weight_h] * layer_config_original[j][weight_n] * layer_config_original[j][weight_m];
        reorderBias(weights, bias_conv[j], ptr, padding_offset[j], layer_config[j][bias_size], layer_config_original[j][bias_size], LANE_NUM);
        ptr += layer_config_original[j][bias_size];
    }

    return 0;
}

void FeatureExtractor::reorderWeights(DTYPE *weights, DTYPE *weight_buf, unsigned dim1, unsigned dim2, unsigned dim3, unsigned dim4, unsigned dim3_original, unsigned dim4_original, unsigned offset, unsigned padding_offset, unsigned vecSize, unsigned laneNum)
{

    DTYPE *copy_with_padding;

    // First, copy the data into new buffer and padding in dim3/dim4 with zeros if needed
    copy_with_padding = (DTYPE *)malloc(sizeof(DTYPE) * dim1 * dim2 * dim3 * dim4);
    if (copy_with_padding == NULL)
    {
        printf("Error: not enough memory when padding weight!!!");
        free(copy_with_padding);
    }
    memset(copy_with_padding, 0, sizeof(DTYPE) * dim1 * dim2 * dim3 * dim4);

    for (unsigned m = 0; m < dim4_original; m++)
    {
        for (unsigned n = 0; n < dim3_original; n++)
        {
            for (unsigned i = 0; i < dim2; i++)
            {
                for (unsigned j = 0; j < dim1; j++)
                {
                    copy_with_padding[(padding_offset * dim1 * dim2 * dim3) + m * dim1 * dim2 * dim3 + n * dim1 * dim2 + i * dim1 + j] = (DTYPE)weights[offset + m * dim1 * dim2 * dim3_original + n * dim1 * dim2 + i * dim1 + j];
                }
            }
        }
    }

    // Second, perform vectorization in dim3 by VEC_SIZE and at the same time, perform vectorization in dim4 by a factor of LANE_NUM
    for (unsigned m = 0; m < (dim4 / laneNum); m++)
    {
        for (unsigned n = 0; n < (dim3 / vecSize); n++)
        {
            for (unsigned i = 0; i < dim2; i++)
            {
                for (unsigned j = 0; j < dim1; j++)
                {
                    for (unsigned ll = 0; ll < laneNum; ll++)
                    {
                        for (unsigned k = 0; k < vecSize; k++)
                        {
                            weight_buf[m * dim1 * dim2 * dim3 * laneNum + n * dim1 * dim2 * vecSize * laneNum + i * dim1 * vecSize * laneNum + j * vecSize * laneNum + ll * vecSize + k] = (DTYPE)copy_with_padding[(m * laneNum + ll) * dim3 * dim2 * dim1 + (n * vecSize + k) * dim1 * dim2 + i * dim1 + j];
                        }
                    }
                }
            }
        }
    }

    // release resource
    free(copy_with_padding);
}

void FeatureExtractor::reorderBias(DTYPE *dataIn, DTYPE *bias, unsigned offset, unsigned padding_offset, unsigned dim4, unsigned dim4_original, unsigned laneNum)
{

    DTYPE *copy_with_padding;

    // first copy the data into new buffer with zero paddings
    copy_with_padding = (DTYPE *)malloc(sizeof(DTYPE) * dim4);
    if (copy_with_padding == NULL)
    {
        printf("Not enough memory when reordering bias!!!");
        free(copy_with_padding);
    }
    memset(copy_with_padding, 0, sizeof(DTYPE) * dim4);
    // padding evenly on two sides of weight_m
    memcpy(copy_with_padding + padding_offset, dataIn + offset, sizeof(DTYPE) * dim4_original);
    // second, perform vectorization by factor of LANE_NUM
    for (unsigned m = 0; m < (dim4 / laneNum); m++)
    {
        for (unsigned ll = 0; ll < laneNum; ll++)
        {
            bias[m * laneNum + ll] = (DTYPE)copy_with_padding[m * laneNum + ll];
        }
    }
    // release resource
    free(copy_with_padding);
}

// Extract one item from batch results
void FeatureExtractor::extractOutput(DTYPE *output, DTYPE *output_one_item, unsigned item_num, unsigned batch_size, unsigned dim1, unsigned dim2, unsigned dim3)
{

    unsigned char mask = 0xff;
    unsigned char batch_size_in_dim;
    unsigned char batch_size_in_dim_log;
    unsigned char batch_indx_dim1;
    unsigned char batch_indx_dim2;

    if (batch_size == 1)
    {
        batch_size_in_dim = 1;
        batch_indx_dim1 = 0;
        batch_indx_dim2 = 0;
    }
    else
    {
        batch_size_in_dim = log(batch_size) / log(2);
        batch_size_in_dim_log = log(batch_size_in_dim) / log(2);
        batch_indx_dim1 = item_num & (~((mask >> batch_size_in_dim_log) << batch_size_in_dim_log));
        batch_indx_dim2 = item_num >> batch_size_in_dim_log;
        printf("Batch Size=%d, verifying NO.%d batch item (indx= %d, %d) ...\n", batch_size, item_num, batch_indx_dim1, batch_indx_dim2);
    }

    for (unsigned k = 0; k < (dim3 / VEC_SIZE); k++)
    {
        for (unsigned i = 0; i < dim2; i++)
        {
            for (unsigned j = 0; j < dim1; j++)
            {
                for (unsigned vv = 0; vv < VEC_SIZE; vv++)
                {
                    output_one_item[k * dim2 * dim1 * VEC_SIZE + i * dim1 * VEC_SIZE + j * VEC_SIZE + vv] = output[k * dim2 * dim1 * batch_size_in_dim * batch_size_in_dim * VEC_SIZE + (i + batch_indx_dim2 * dim2) * batch_size_in_dim * dim1 * VEC_SIZE + (j + batch_indx_dim1 * dim1) * VEC_SIZE + vv];
                }
            }
        }
    }
}

// Re-ordering the vectorized output into scalar form
void FeatureExtractor::reorderOutput(DTYPE *output, DTYPE *output_reorder, unsigned dim1, unsigned dim2, unsigned dim3)
{

    for (unsigned i = 0; i < dim2; i++)
    {
        for (unsigned j = 0; j < dim1; j++)
        {
            for (unsigned k = 0; k < (dim3 / VEC_SIZE); k++)
            {
                for (unsigned vv = 0; vv < VEC_SIZE; vv++)
                {
                    output_reorder[(k * VEC_SIZE + vv) * dim2 * dim1 + i * dim1 + j] = output[k * dim2 * dim1 * VEC_SIZE + i * dim1 * VEC_SIZE + j * VEC_SIZE + vv];
                }
            }
        }
    }
}

// Release all memory resources here
void FeatureExtractor::cleanup()
{

    // Release the opencl runtime resource allocated
    for (unsigned i = 0; i < num_devices; ++i)
    {
        if (knl_memRd && knl_memRd[i])
        {
            clReleaseKernel(knl_memRd[i]);
        }
        if (knl_conv && knl_conv[i])
        {
            clReleaseKernel(knl_conv[i]);
        }
        if (knl_memWr && knl_memWr[i])
        {
            clReleaseKernel(knl_memWr[i]);
        }
        if (knl_pool && knl_pool[i])
        {
            clReleaseKernel(knl_pool[i]);
        }
        if (knl_lrn && knl_lrn[i])
        {
            clReleaseKernel(knl_lrn[i]);
        }
        if (que_memRd && que_memRd[i])
        {
            clReleaseCommandQueue(que_memRd[i]);
        }
        if (que_conv && que_conv[i])
        {
            clReleaseCommandQueue(que_conv[i]);
        }
        if (que_memWr && que_memWr[i])
        {
            clReleaseCommandQueue(que_memWr[i]);
        }
        if (que_pool && que_pool[i])
        {
            clReleaseCommandQueue(que_pool[i]);
        }
        if (data_buf && data_buf[i])
        {
            clReleaseMemObject(data_buf[i]);
        }
        if (output_buf && output_buf[i])
        {
            clReleaseMemObject(output_buf[i]);
        }
        if (weights_buf && weights_buf[i])
        {
            clReleaseMemObject(weights_buf[i]);
        }
        if (bias_buf && bias_buf[i])
        {
            clReleaseMemObject(bias_buf[i]);
        }
        if (fc_1_buf && fc_1_buf[i])
        {
            clReleaseMemObject(fc_1_buf[i]);
        }
        if (fc_2_buf && fc_2_buf[i])
        {
            clReleaseMemObject(fc_2_buf[i]);
        }
    }

    if (program)
    {
        clReleaseProgram(program);
    }
    if (context)
    {
        clReleaseContext(context);
    }

    alignedFree(weights);
    alignedFree(image);
    alignedFree(data_init);
    for (int j = 0; j < LAYER_NUM; j++)
    {
        alignedFree(weight_conv[j]);
        alignedFree(bias_conv[j]);
    }
    alignedFree(golden_ref);
    alignedFree(output);
    alignedFree(output_reorder);
    alignedFree(output_one_item);
}
