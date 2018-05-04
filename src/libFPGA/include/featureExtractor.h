#ifndef _FEATURE_EXTRACTOR_H_
#define _FEATURE_EXTRACTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <iostream>
#include <fstream>

#include <CL/opencl.h>
#include "hardwareParam.h"
#include "layerConfig.h"
#include "oclUtil.h"
#include "timer.h"
#include "netConfig.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

using namespace ocl_util;

/* 必须设计成单例类 */
class FeatureExtractor
{
  public:
    static FeatureExtractor &getInstance()
    {
        static FeatureExtractor mFeatureExtractor;
        return mFeatureExtractor;
    }
    std::vector<float> extractFeature(const cv::Mat &img);

  private:
    FeatureExtractor()
    {
        initialDevice();
    }

    // FeatureExtractor(const std::string &_kernelFile, const std::string &_weightFile) : kernelFile(_kernelFile), weight_file_path(_weightFile)
    // {
    //     initialDevice();
    // }

    ~FeatureExtractor()
    {
        cleanup();
    }

    std::string kernelFile = "/home/sh/workspace/OpenCLkernel/conv_16_32.aocx";
    const char *vendor_name = "Intel";
    std::string weight_file_path = "/home/sh/data/vgg_face_self/weights.dat";

    // Configuration file instructions
    enum config_item
    {
        layer_type, // "0" -> conv, "1" -> fc

        data_w,
        data_h,
        data_n,
        weight_w,
        weight_h,
        weight_n,
        weight_m,
        bias_size, //memRd Parameters

        memrd_src, //"0"-> data_buf  "1"-> output_buf  "2"->"fc_1_buffer"  "3"->"fc_2_buffer"

        conv_x,
        conv_y,
        conv_z,
        conv_stride,
        conv_padding,
        conv_split,
        conv_relu, //Conv Parameters

        pool_on,
        pool_x,
        pool_y,
        pool_z,
        pool_size,
        pool_stride, // Pooling Parameters

        lrn_on, // lrn on/off control

        memwr_dst //"0"-> data_buf  "1"-> output_buf  "2"->"fc_1_buffer"  "3"->"fc_2_buffer"

    };

    enum input_item
    {

        image_w,
        image_h,
        image_n, // original image size

        batch_size

    };

    enum output_item
    {

        output_w,
        output_h,
        output_n

    };

    enum precision_item
    {

        frac_w,
        frac_din,
        frac_dout

    };

    fpga::Timer t; // Timer used for performance measurement
    float time;

    //------------ Global Functions & Variables ------------//
    cl_uint num_devices = 0;
    cl_context context = NULL;
    cl_program program = NULL;
    scoped_array<cl_device_id> device;
    scoped_array<cl_kernel> knl_memRd;
    scoped_array<cl_kernel> knl_conv;
    scoped_array<cl_kernel> knl_memWr;
    scoped_array<cl_kernel> knl_pool;
    scoped_array<cl_kernel> knl_lrn;
    scoped_array<cl_command_queue> que_memRd;
    scoped_array<cl_command_queue> que_conv;
    scoped_array<cl_command_queue> que_memWr;
    scoped_array<cl_command_queue> que_pool;
    scoped_array<cl_mem> data_buf;
    scoped_array<cl_mem> output_buf;
    scoped_array<cl_mem> weights_buf;
    scoped_array<cl_mem> bias_buf;
    scoped_array<cl_mem> fc_1_buf;
    scoped_array<cl_mem> fc_2_buf;

    DTYPE *weights;
    DTYPE *image;
    DTYPE *data_init;
    DTYPE *weight_conv[MAX_LAYER_NUM];
    DTYPE *bias_conv[MAX_LAYER_NUM];
    DTYPE *output;
    DTYPE *output_one_item;
    DTYPE *output_reorder;
    DTYPE *golden_ref;

    unsigned layer_config_original[LAYER_NUM][NUM_CONFIG_ITEM];

    cl_int status;

    //将图片数据读取带Buffer中
    void loadImageToBuffer(const cv::Mat &img);
    //读取网络参数
    int prepare();
    //从FPGA读取数据
    void readDataBack();
    //数据预处理
    void reorderWeights(DTYPE *weights, DTYPE *weight_buf, unsigned dim1, unsigned dim2, unsigned dim3, unsigned dim4, unsigned dim3_original, unsigned dim4_original, unsigned offset, unsigned padding_offset, unsigned vecSize, unsigned laneNum);
    void reorderBias(DTYPE *dataIn, DTYPE *bias, unsigned offset, unsigned padding_offset, unsigned dim4, unsigned dim4_original, unsigned laneNum);
    void reorderOutput(DTYPE *output, DTYPE *output_reorder, unsigned dim1, unsigned dim2, unsigned dim3);
    void extractOutput(DTYPE *output, DTYPE *output_one_item, unsigned item_num, unsigned batch_size, unsigned dim1, unsigned dim2, unsigned dim3);
    void cleanup();
    bool initialDevice();
    void executeKernel();
};

#endif // !_FEATURE_EXTRACTOR_H