
typedef signed char DTYPE;

//----------- Design Parameters --------------//
// select what platform is used
const char *vendor_name = "Intel";

#define DEVICE_TYPE CL_DEVICE_TYPE_ACCELERATOR

// SW System parameters
#define DMA_ALIGNMENT 64
#define MAX_LAYER_NUM 16
#define MAX_BATCH_SIZE 16

#define IN_BUF_SIZE 256 * 256 * 64 // Note: the buffer size should be large enough to hold all temperary results
#define OUT_BUF_SIZE 256 * 256 * 64
#define FC_BUF_SIZE 32768 * MAX_BATCH_SIZE

#define MEAN_DATA_WIDTH 256
#define MEAN_DATA_HEIGHT 256
#define MEAN_DATA_CHANNEl 3
#define PICTURE_NUM 50000
#define MAX_PIC_NUM 50000
const char *mean_data_file_path = "../data/imagenet/mean_data.dat";
const char *synset_word_file_path = "../data/imagenet/synset_words.txt";
const char *LabelPath = "../data/imagenet/val.txt";
// char picture_file_path_head[100] = "/home/sh/data/ILSVRC2012_img_val/ILSVRC2012_val_";
// char picture_file_path[100];
int label[MAX_PIC_NUM] = {0};
char label_buf[MAX_PIC_NUM][1024] = {0};
// char synset_buf[1000][1024] = {0};
DTYPE searchTop[1024];
// float accuracy1 = 0;
// float accuracy5 = 0;

// int true_count = 0;
// int false_count = 0;

// VGG16
// Original problem size
// File size is in num of DTYPE numbers
#define IMAGE_FILE_SIZE (224 * 224 * 3)
#define WEIGHTS_FILE_SIZE 138455872 //fc8-1024
#define LAYER_NUM 16
#define CONV_NUM 13

const char *weight_file_path = "/home/sh/data/data_vgg/weights.dat";
const char *input_file_path = "/home/sh/data/data_vgg/image.dat";
const char *ref_file_path = "/home/sh/data/data_vgg/fc8.dat";
const char *dump_file_path = "./result_dump.txt";
