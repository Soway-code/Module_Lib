#ifndef __ADC_H
#define __ADC_H 			   
#include "sys.h"


#define ADC1_DR_Address    ((uint32_t)0x4001244C) //ADC1�������ĵ�ַ����ο��ֲ�ó���

#define ADCPORT		GPIOA	//����ADC�ӿ�
#define ADC_CH0		GPIO_Pin_0	//����ADC�ӿ� ��ѹ��λ��

void ADC_DMA_Init(void);
void ADC_GPIO_Init(void);
void ADC_Configuration(void);

#endif




























