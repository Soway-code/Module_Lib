#ifndef _USART_H
#define _USART_H
#include "stdio.h"



//#define MB_ASCII_MODE          0x01
#define MB_RTU_MODE            0x00
#define MAX_USART1_DATA_LEN    255


//extern u8  *USART_RX_BUF; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
//extern u16 UART1_RXBuffLen;
#include "type.h"

#define TX_ON    GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define TX_OFF   GPIO_ResetBits(GPIOA, GPIO_Pin_8)

void UART1_Init(const UartCommTypeDef *pUartStructure);
u32 UART1_SendData( u8* UART1_SendBuff, u32 Len );
void UART1_IRQService( void ) ;
s32 UART1_RecvData( u8* UART1_RecvBuff, u32 Len );

void uprintf(const char *fmt,...);//调试用
#endif


