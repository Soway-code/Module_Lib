#ifndef _SPI_H
#define _SPI_H

#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "BSP.h"

#define SPI_Div_2  	0
#define SPI_Div_4  	1
#define SPI_Div_8  	2
#define SPI_Div_16 	3
#define SPI_Div_32	4
#define SPI_Div_64	5
#define SPI_Div_128	6
#define SPI_Div_256	7

void SPI1_Init(void);
void SPI1_SetSpeed(u8 SpeedSet);

//初始化SPI外设
void SPIx_Configuration(SPI_TypeDef *SPIx);

//SPI外设BRR分频配置
void SPIx_BRR_Div(SPI_TypeDef *SPIx, uint8_t SPI_Div);

//SPI外设一字节通信
uint8_t SPIx_WRByte(SPI_TypeDef *SPIx, uint8_t Txdata);
u8 SPI1_ReadWriteByte(u8 TxData);


#endif
