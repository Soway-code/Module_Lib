/*************************************************************************************
                               This file is the GD32F103 
				GD seiral mpu USART via DMA process communicate head file
*************************************************************************************/
#ifndef __USART_H
#define	__USART_H

//Includes
#include "Includes.h"

//Define macrco
#define	NUM_COM	2

//Define Communicate UART parameter
//#define MODBUS_RTU      1
//#define MODBUS_ASCII    2

#define	RS485CTL_SRC_CLK	RCU_GPIOA
#define	RS485CTL_PORT		GPIOA
#define	RS485CTL_PIN		GPIO_PIN_8
#define RS485_RCV_ENABLE()	GPIO_BC( RS485CTL_PORT ) = RS485CTL_PIN;
#define RS485_TXD_ENABLE()	GPIO_BOP( RS485CTL_PORT ) = RS485CTL_PIN;

//USART0 ADDRESS
#define	USART0_ADDR_BASE	0x40013800
#define	USART0_ADDR_DATA	0x40013804
#define	READ_USART0_DATA()	(*(volatile uint32_t *)(uint32_t)(0x40013804))

//USART1 ADDRESS
#define	USART1_ADDR_BASE	0x40014400
#define	USART1_ADDR_DATA	0x40004404

typedef struct 
{
	uint8_t	SlaveAddr;
	uint8_t	BaudRate;
    uint8_t	Parity;
    uint8_t	ModbusType;  //1-RTU 2-ASCII
}S_COMM_PARA,*PS_COMM_PARA;

typedef struct{
    uint8_t* prcv;
    uint16_t rcvsize;
    uint8_t* psnd;
    uint16_t sndsize;
}S_COMM_BUFPARA, *PS_COMM_BUFPARA;

typedef void (*ModbusParseFunc)(uint8_t *psrc, uint32_t len, uint8_t type );
typedef struct{
    uint8_t*             pbuf;
    uint32_t             bufsize;
	ModbusRecvDataFunc recvdata;
    ModbusParseFunc pparse;	//1
}S_PARSE_TAG, *PS_PARSE_TAG;

//Functions declare
bool USART_ConfigPaserParameters( PS_PARSE_TAG );
bool USART_CommunicateInitial( uint8_t com_sn, PS_COMM_BUFPARA, PS_COMM_PARA );
bool USART_CommunicateParaConfig( uint8_t, PS_COMM_PARA );
bool My_USART_DMA_SendData(uint16_t datlen);
bool USART_SendDataViaUSART0DmaChnIsBusy( void );

//Timer3 init
void USART_Timer3_Configuation( uint8_t dlyid );

//--------------------------------modbus part-----------------------------------------------------
void GD32F10X_Modbus_UpgradeResponseProcess( uint8_t* psrc, uint16_t len );
uint32_t GD32F10X_Modbus_ModbusTimer( uint8_t** pRecvBuff);
uint16_t My_USART_GetOneFrameDMAData( uint8_t* pRecvBuff, uint16_t RvBuffSize );
//Interrupt function
void USART0_IRQHandler( void );
void TIMER3_IRQHandler( void );

#endif
