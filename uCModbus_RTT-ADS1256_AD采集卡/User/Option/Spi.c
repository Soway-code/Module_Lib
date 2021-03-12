/*****************************************************************************
* @file    Project/spi.c 
* @author  ktfe
* @version V1.0.0
* @date    25/08/2012
* @brief   Main program body
*****************************************************************************/

#include "Spi.h"
#include "stm32f10x_spi.h"

//初始化SPI外设
void SPIx_Configuration(SPI_TypeDef *SPIx)
{
	SPI_InitTypeDef SPI_InitStruct;
	
	/*SPI通信所用GPIO初始化*/
	if(SPIx==SPI1)
	{
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	//I/O port A clock enable
		GPIOA->CRL &= 0x000fffff;	 					//配置SPI1通信的3个IO口
		GPIOA->CRL |= 0xb8b00000; 
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	//SPI 1 clock enable
	}
	if(SPIx==SPI2)
	{
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;	//I/O port B clock enable
		GPIOB->CRH &= 0x000fffff;						//配置SPI1通信的3个IO口
		GPIOB->CRH |= 0xb8b00000;
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;	//SPI 2 clock enable
	}		
	/*SPI初始化*/
	SPI_StructInit(&SPI_InitStruct);	//恢复默认配置
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//SPI传输模式
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;					//主模式
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;			//位宽
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;						//SCLK静态电平
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;						//采样沿
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;							//NSS管理
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;	//传输速率
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;			//起始位
	SPI_InitStruct.SPI_CRCPolynomial = 7;								//CRC校验
	SPI_Init(SPIx, &SPI_InitStruct);										//初始化
	SPI_Cmd(SPIx, ENABLE);
}
//SPI外设BRR分频配置
void SPIx_BRR_Div(SPI_TypeDef *SPIx, uint8_t SPI_Div)
{
	SPIx->CR1 &= 0xff87;
	SPIx->CR1 |= SPI_Div<<3;	//SPI速率设置
	SPIx->CR1 |= SPI_CR1_SPE; //SPI设备使能
}
//SPI外设一字节通信
uint8_t SPIx_WRByte(SPI_TypeDef *SPIx, uint8_t Txdata)
{
	uint8_t index = 0;
	while((SPIx->SR&1<<1)==0)
	{
		index++;
		if(index>200)return 0;
	}
	SPIx->DR = Txdata;
	index = 0;
	while((SPIx->SR&1)==0)
	{
		index++;
		if(index>200)return 0;
	}
	return SPIx->DR;	
}
