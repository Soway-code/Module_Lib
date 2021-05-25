#ifndef _SPI_H
#define _SPI_H

#include "stm32f10x.h"

#define SPI_Div_2  	0
#define SPI_Div_4  	1
#define SPI_Div_8  	2
#define SPI_Div_16 	3
#define SPI_Div_32	4
#define SPI_Div_64	5
#define SPI_Div_128	6
#define SPI_Div_256	7

//��ʼ��SPI����
void SPIx_Configuration(SPI_TypeDef *SPIx);

//SPI����BRR��Ƶ����
void SPIx_BRR_Div(SPI_TypeDef *SPIx, uint8_t SPI_Div);

//SPI����һ�ֽ�ͨ��
uint8_t SPIx_WRByte(SPI_TypeDef *SPIx, uint8_t Txdata);

#endif