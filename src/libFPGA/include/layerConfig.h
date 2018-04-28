#ifndef _LAYER_CONFIG_H_
#define _LAYER_CONFIG_H_

#include "hardwareParam.h"
#include "netConfig.h"

#define NUM_CONFIG_ITEM 25

extern unsigned layer_config[LAYER_NUM][NUM_CONFIG_ITEM];

extern char precision_config[LAYER_NUM][3];

extern unsigned input_config[4];

extern unsigned output_config[3] ; //Layer-15

#endif // !_LAYER_CONFIG_H_
