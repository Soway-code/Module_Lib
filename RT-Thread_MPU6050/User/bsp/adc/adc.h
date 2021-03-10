#ifndef __ADC_H
#define __ADC_H 			   
#include "sys.h"


#define ADC1_DR_Address    ((uint32_t)0x4001244C) //ADC1这个外设的地址（查参考手册得出）

#define ADCPORT		GPIOA	//定义ADC接口
#define ADC_CH0		GPIO_Pin_0	//定义ADC接口 电压电位器

void ADC_DMA_Init(void);
void ADC_GPIO_Init(void);
void ADC_Configuration(void);

#endif





























