#include "layerConfig.h"


unsigned layer_config[LAYER_NUM][NUM_CONFIG_ITEM] = {
	{0,
	 224, 224, 3, 3, 3, 3, 64, 64,
	 0,
	 224, 224, 64, 1, 1, 0, 1,
	 0, 224, 224, 64, 0, 0,
	 0,
	 1}, //Layer-1 (conv1_1)
	{0,
	 224, 224, 64, 3, 3, 64, 64, 64,
	 1,
	 224, 224, 64, 1, 1, 0, 1,
	 1, 112, 112, 64, 2, 2,
	 0,
	 0}, //Layer-2 (conv1_2)
	{0,
	 112, 112, 64, 3, 3, 64, 128, 128,
	 0,
	 112, 112, 128, 1, 1, 0, 1,
	 0, 112, 112, 128, 0, 0,
	 0,
	 1}, //Layer-3 (conv2_1)
	{0,
	 112, 112, 128, 3, 3, 128, 128, 128,
	 1,
	 112, 112, 128, 1, 1, 0, 1,
	 1, 56, 56, 128, 2, 2,
	 0,
	 0}, //Layer-4 (conv2_2)
	{0,
	 56, 56, 128, 3, 3, 128, 256, 256,
	 0,
	 56, 56, 256, 1, 1, 0, 1,
	 0, 56, 56, 256, 0, 0,
	 0,
	 1}, //Layer-5 (conv3_1)
	{0,
	 56, 56, 256, 3, 3, 256, 256, 256,
	 1,
	 56, 56, 256, 1, 1, 0, 1,
	 0, 56, 56, 256, 0, 0,
	 0,
	 0}, //Layer-6 (conv3_2)
	{0,
	 56, 56, 256, 3, 3, 256, 256, 256,
	 0,
	 56, 56, 256, 1, 1, 0, 1,
	 1, 28, 28, 256, 2, 2,
	 0,
	 1}, //Layer-7 (conv3_3)
	{0,
	 28, 28, 256, 3, 3, 256, 512, 512,
	 1,
	 28, 28, 512, 1, 1, 0, 1,
	 0, 28, 28, 512, 0, 0,
	 0,
	 0}, //Layer-8  (conv4_1)
	{0,
	 28, 28, 512, 3, 3, 512, 512, 512,
	 0,
	 28, 28, 512, 1, 1, 0, 1,
	 0, 28, 28, 512, 0, 0,
	 0,
	 1}, //Layer-9  (conv4_2)
	{0,
	 28, 28, 512, 3, 3, 512, 512, 512,
	 1,
	 28, 28, 512, 1, 1, 0, 1,
	 1, 14, 14, 512, 2, 2,
	 0,
	 0}, //Layer-10 (conv4_3)
	{0,
	 14, 14, 512, 3, 3, 512, 512, 512,
	 0,
	 14, 14, 512, 1, 1, 0, 1,
	 0, 14, 14, 512, 0, 0,
	 0,
	 1}, //Layer-11  (conv5_1)
	{0,
	 14, 14, 512, 3, 3, 512, 512, 512,
	 1,
	 14, 14, 512, 1, 1, 0, 1,
	 0, 14, 14, 512, 0, 0,
	 0,
	 0}, //Layer-12  (conv5_2)
	{0,
	 14, 14, 512, 3, 3, 512, 512, 512,
	 0,
	 14, 14, 512, 1, 1, 0, 1,
	 1, 7, 7, 512, 2, 2,
	 0,
	 2}, //Layer-13  (conv5_3)    Note: for last conv layer, outputs are write to fc buffer
	{1,
	 7, 7, 512, 7, 7, 512, 4096, 4096,
	 2,
	 1, 1, 4096, 7, 0, 0, 1,
	 0, 1, 1, 4096, 0, 0,
	 0,
	 3}, //Layer-14  (fc6)
	{1,
	 1, 1, 4096, 1, 1, 4096, 512, 512,
	 3,
	 1, 1, 512, 1, 0, 0, 1,
	 0, 1, 1, 512, 0, 0,
	 0,
	 2}, //Layer-15  (fc7)
		 // {1,
		 // 1, 1, 512, 1, 1, 512, 8630, 8630,
		 // 2,
		 // 1, 1, 8630, 1, 0, 0, 0,
		 // 0, 1, 1, 8630, 0, 0,
		 // 0,
		 // 3}//Layer-16  (fc8)
};

char precision_config[LAYER_NUM][3] = {
	{7, 0, -2},  //Layer-1
	{8, -2, -5}, //Layer-2
	{8, -5, -5}, //Layer-3
	{8, -5, -6}, //Layer-4
	{7, -6, -7}, //Layer-5
	{8, -7, -7}, //Layer-6
	{8, -7, -7}, //Layer-7
	{8, -7, -6}, //Layer-8
	{8, -6, -5}, //Layer-9
	{8, -5, -5}, //Layer-10
	{9, -5, -4}, //Layer-11
	{9, -4, -3}, //Layer-12
	{8, -3, -2}, //Layer-13
	{8, -2, 0},  //Layer-14
	{7, 0, 2},   //Layer-15
				 // { 7,  2,  2}//Layer-16
};

unsigned input_config[4] = {224, 224, 3, 1};

//unsigned output_config[3] = {224, 224, 64};//Layer-1

//unsigned output_config[3] = {56, 56, 128};//Layer-4(pool2)

//unsigned output_config[3] = {28, 28, 256};//Layer-7(pool3)

//unsigned output_config[3] = {28, 28, 512};//Layer-8(relu4_1)

//unsigned output_config[3] = {28, 28, 512};//Layer-9(relu4_2)

//unsigned output_config[3] = {14, 14, 512};//Layer-10(pool4)

//unsigned output_config[3] = {7, 7, 512};//Layer-13(pool5)

//unsigned output_config[3] = {1, 1, 4096};//Layer-14

unsigned output_config[3] = {1, 1, 512}; //Layer-15

// unsigned output_config[3] = {1, 1, 8630};//Layer-16 


/* 
unsigned layer_config[LAYER_NUM][NUM_CONFIG_ITEM] = {
	{0,
	 224, 224, 3, 3, 3, 3, 64, 64,
	 0,
	 224, 224, 64, 1, 1, 0, 1,
	 0, 224, 224, 64, 0, 0,
	 0,
	 1}, //Layer-1 (conv1_1)
	{0,
	 224, 224, 64, 3, 3, 64, 64, 64,
	 1,
	 224, 224, 64, 1, 1, 0, 1,
	 1, 112, 112, 64, 2, 2,
	 0,
	 0}, //Layer-2 (conv1_2)
	{0,
	 112, 112, 64, 3, 3, 64, 128, 128,
	 0,
	 112, 112, 128, 1, 1, 0, 1,
	 0, 112, 112, 128, 0, 0,
	 0,
	 1}, //Layer-3 (conv2_1)
	{0,
	 112, 112, 128, 3, 3, 128, 128, 128,
	 1,
	 112, 112, 128, 1, 1, 0, 1,
	 1, 56, 56, 128, 2, 2,
	 0,
	 0}, //Layer-4 (conv2_2)
	{0,
	 56, 56, 128, 3, 3, 128, 256, 256,
	 0,
	 56, 56, 256, 1, 1, 0, 1,
	 0, 56, 56, 256, 0, 0,
	 0,
	 1}, //Layer-5 (conv3_1)
	{0,
	 56, 56, 256, 3, 3, 256, 256, 256,
	 1,
	 56, 56, 256, 1, 1, 0, 1,
	 0, 56, 56, 256, 0, 0,
	 0,
	 0}, //Layer-6 (conv3_2)
	{0,
	 56, 56, 256, 3, 3, 256, 256, 256,
	 0,
	 56, 56, 256, 1, 1, 0, 1,
	 1, 28, 28, 256, 2, 2,
	 0,
	 1}, //Layer-7 (conv3_3)
	{0,
	 28, 28, 256, 3, 3, 256, 512, 512,
	 1,
	 28, 28, 512, 1, 1, 0, 1,
	 0, 28, 28, 512, 0, 0,
	 0,
	 0}, //Layer-8  (conv4_1)
	{0,
	 28, 28, 512, 3, 3, 512, 512, 512,
	 0,
	 28, 28, 512, 1, 1, 0, 1,
	 0, 28, 28, 512, 0, 0,
	 0,
	 1}, //Layer-9  (conv4_2)
	{0,
	 28, 28, 512, 3, 3, 512, 512, 512,
	 1,
	 28, 28, 512, 1, 1, 0, 1,
	 1, 14, 14, 512, 2, 2,
	 0,
	 0}, //Layer-10 (conv4_3)
	{0,
	 14, 14, 512, 3, 3, 512, 512, 512,
	 0,
	 14, 14, 512, 1, 1, 0, 1,
	 0, 14, 14, 512, 0, 0,
	 0,
	 1}, //Layer-11  (conv5_1)
	{0,
	 14, 14, 512, 3, 3, 512, 512, 512,
	 1,
	 14, 14, 512, 1, 1, 0, 1,
	 0, 14, 14, 512, 0, 0,
	 0,
	 0}, //Layer-12  (conv5_2)
	{0,
	 14, 14, 512, 3, 3, 512, 512, 512,
	 0,
	 14, 14, 512, 1, 1, 0, 1,
	 1, 7, 7, 512, 2, 2,
	 0,
	 2}, //Layer-13  (conv5_3)    Note: for last conv layer, outputs are write to fc buffer
	{1,
	 7, 7, 512, 7, 7, 512, 4096, 4096,
	 2,
	 1, 1, 4096, 7, 0, 0, 1,
	 0, 1, 1, 4096, 0, 0,
	 0,
	 3}, //Layer-14  (fc6)
	{1,
	 1, 1, 4096, 1, 1, 4096, 4096, 4096,
	 3,
	 1, 1, 4096, 1, 0, 0, 1,
	 0, 1, 1, 4096, 0, 0,
	 0,
	 2}, //Layer-15  (fc7)
		 // {1,
		 // 1, 1, 512, 1, 1, 512, 8630, 8630,
		 // 2,
		 // 1, 1, 8630, 1, 0, 0, 0,
		 // 0, 1, 1, 8630, 0, 0,
		 // 0,
		 // 3}//Layer-16  (fc8)
};

char precision_config[LAYER_NUM][3] = {
	{7, 0, -2},  //Layer-1
	{8, -2, -5}, //Layer-2
	{8, -5, -5}, //Layer-3
	{8, -5, -6}, //Layer-4
	{7, -6, -7}, //Layer-5
	{8, -7, -7}, //Layer-6
	{8, -7, -7}, //Layer-7
	{8, -7, -6}, //Layer-8
	{8, -6, -5}, //Layer-9
	{8, -5, -5}, //Layer-10
	{9, -5, -4}, //Layer-11
	{9, -4, -3}, //Layer-12
	{8, -3, -2}, //Layer-13
	{8, -2, 0},  //Layer-14
	{7, 0, 2},   //Layer-15
				 // { 7,  2,  2}//Layer-16
};

unsigned input_config[4] = {224, 224, 3, 1};

//unsigned output_config[3] = {224, 224, 64};//Layer-1

//unsigned output_config[3] = {56, 56, 128};//Layer-4(pool2)

//unsigned output_config[3] = {28, 28, 256};//Layer-7(pool3)

//unsigned output_config[3] = {28, 28, 512};//Layer-8(relu4_1)

//unsigned output_config[3] = {28, 28, 512};//Layer-9(relu4_2)

//unsigned output_config[3] = {14, 14, 512};//Layer-10(pool4)

//unsigned output_config[3] = {7, 7, 512};//Layer-13(pool5)

//unsigned output_config[3] = {1, 1, 4096};//Layer-14

unsigned output_config[3] = {1, 1, 4096}; //Layer-15

// unsigned output_config[3] = {1, 1, 8630};//Layer-16  
 */
