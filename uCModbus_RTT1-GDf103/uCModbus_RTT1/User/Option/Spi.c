/*****************************************************************************
* @file    Project/spi.c 
* @author  ktfe
* @version V1.0.0
* @date    25/08/2012
* @brief   Main program body
*****************************************************************************/

#include "Spi.h"
#include "stm32f10x_spi.h"

//��ʼ��SPI����
void SPIx_Configuration(SPI_TypeDef *SPIx)
{
	SPI_InitTypeDef SPI_InitStruct;
	
	/*SPIͨ������GPIO��ʼ��*/
	if(SPIx==SPI1)
	{
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	//I/O port A clock enable
		GPIOA->CRL &= 0x000fffff;	 					//����SPI1ͨ�ŵ�3��IO��
		GPIOA->CRL |= 0xb8b00000; 
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	//SPI 1 clock enable
	}
	if(SPIx==SPI2)
	{
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;	//I/O port B clock enable
		GPIOB->CRH &= 0x000fffff;						//����SPI1ͨ�ŵ�3��IO��
		GPIOB->CRH |= 0xb8b00000;
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;	//SPI 2 clock enable
	}		
	/*SPI��ʼ��*/
	SPI_StructInit(&SPI_InitStruct);	//�ָ�Ĭ������
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//SPI����ģʽ
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;					//��ģʽ
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;			//λ��
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;						//SCLK��̬��ƽ
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;						//������
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;							//NSS����
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;	//��������
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;			//��ʼλ
	SPI_InitStruct.SPI_CRCPolynomial = 7;								//CRCУ��
	SPI_Init(SPIx, &SPI_InitStruct);										//��ʼ��
	SPI_Cmd(SPIx, ENABLE);
}
//SPI����BRR��Ƶ����
void SPIx_BRR_Div(SPI_TypeDef *SPIx, uint8_t SPI_Div)
{
	SPIx->CR1 &= 0xff87;
	SPIx->CR1 |= SPI_Div<<3;	//SPI��������
	SPIx->CR1 |= SPI_CR1_SPE; //SPI�豸ʹ��
}
//SPI����һ�ֽ�ͨ��
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
