#include "featureExtractor.h"

using namespace std;
int dim = 8630;
// char synset_buf[1000][1024] = {0};
// const char *ref_file_path = "";

// void softmax(vector<float> output_reorder, vector<float> output)
// {
//     int i;
//     float data_max = 0.0;
//     float data_exp;
//     float sum_exp = 0.0;
//     for (i = 0; i < dim; i++)
//     {
//         if (data_max < output_reorder[i])
//             data_max = output_reorder[i];
//     }
//     for (i = 0; i < dim; i++)
//     {
//         data_exp = exp((float)output_reorder[i] - data_max);
//         sum_exp += data_exp;
//     }
//     for (i = 0; i < dim; i++)
//     {
//         data_exp = exp((float)output_reorder[i] - data_max);
//         output[i] = data_exp / sum_exp * 100.0;
//     }
// }

// int getProb(vector<float> output)
// {
//     // Synset_words
//     int nn = 0;
//     FILE *fp = fopen("/home/sh/workspace/PipeCNN/data/imagenet/synset_words.txt", "r");
//     if (!fp)
//     {
//         printf("Synset word file does not exits !!!\n");
//         return 1;
//     }
//     while (!feof(fp))
//     {
//         fgets(synset_buf[nn], 1024, fp);
//         nn++;
//     }
//     fclose(fp);

//     int m = 0;
//     float max = output[0];

//     // find the class with the highest score
//     for ( int i = 0; i < dim; i++)
//     {
//         if (max < output[i])
//         {
//             max = output[i];
//             m = i;
//         }
//     }

//     // replace the last two ASCII charactor with space
//     int ii = strlen(synset_buf[m]);
//     synset_buf[m][ii - 2] = 32;
//     synset_buf[m][ii - 1] = 32;

//     printf("\nThe inference result is %s (the prob is %5.2f) \n\n", synset_buf[m], max);

//     return m;
// }

// void verify(vector<float> output_reorder)
// {
//     ifstream bin_file_r;
//     // golden_output
//     int godref_size = dim;
//     bin_file_r.open(ref_file_path, ios::in | ios::binary);
//     DTYPE *golden_ref = (DTYPE *)alignedMalloc(sizeof(DTYPE) * godref_size, DMA_ALIGNMENT);

//     if (bin_file_r.is_open())
//     {
//         //Get file size
//         bin_file_r.seekg(0, bin_file_r.end);
//         int file_size = bin_file_r.tellg();
//         bin_file_r.seekg(0, bin_file_r.beg);

//         bin_file_r.read((char *)golden_ref, sizeof(DTYPE) * godref_size);
//         printf("%d total output reference read \n\n", file_size / ((int)sizeof(DTYPE)));
//         if (godref_size != (file_size / (sizeof(DTYPE))))
//             printf("Warning: golden reference file size does not match !!!\n");
//         bin_file_r.close();
//     }
//     else
//         printf("Golden file does not exits !!!\n");
//     printf("\nStart verifying results ...\n");
//     unsigned int err_num;
//     float std_err; // standard errors
//      int batch_item_size;
//     // Compare each results with the golden reference data
//     batch_item_size = dim;
//     err_num = 0;
//     for ( int j = 0; j < batch_item_size; j++)
//     {
//         std_err = abs(output_reorder[j] - golden_ref[j]);
//         // if (std_err > 0)
//         {
//             err_num++;
//             if (err_num < 20)
//                 printf("Item=%d is wrong (result=%f, reference=%f)\n", j, (float)output_reorder[j], (float)golden_ref[j]);
//         }
//     }
//     if (err_num > 0)
//         printf("Totally %d Wrong Results\n", err_num);

//     softmax(output_reorder, output_reorder);
//     getProb(output_reorder);
// }

int main()
{
    FeatureExtractor mFeatureExtractor;
    while (1)
    {
        cv::Mat img = cv::imread("/home/sh/workspace/PipeCNN/data/picture/cat.jpg");
        vector<float> result = mFeatureExtractor.extractFeature(img);
        dim = result.size();
        cout << flush << "\r Feature dim: " << dim << endl;;
    }

    // verify(result);
    return 0;
}