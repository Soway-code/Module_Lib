#ifndef __CALCULATE_H
#define __CALCULATE_H
#include "stdint.h"
uint8_t WeightProcessing_init(const uint16_t *adc_in,const uint16_t *weight_in,const uint8_t count);
uint16_t WeightProcessing(uint16_t adc_in);
#endif
