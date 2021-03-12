/**@file        usart.h
* @brief        
* @details      protocol.c的头文件,声明了数据包数据处理的API函数
* @author      	马灿林
* @date         2021-1-8
* @version      V1.0.0
* @copyright    2021-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/1/8    <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#ifndef __USART_H
#define	__USART_H

#include "stm32f10x.h"
#include <stdio.h>


// 串口对应的DMA请求通道
#define  USART_RX_DMA_CHANNEL     DMA1_Channel5
// 外设寄存器地址
#define  USART_DR_ADDRESS        (&DEBUG_USARTx->DR)
// 一次发送的数据量
#define  USART_RBUFF_SIZE            200 

// 串口1-USART1
#define  DEBUG_USARTx                   USART1
#define  DEBUG_USART_CLK                RCC_APB2Periph_USART1
#define  DEBUG_USART_APBxClkCmd         RCC_APB2PeriphClockCmd
#define  DEBUG_USART_BAUDRATE           9600

// USART GPIO 引脚宏定义
#define  DEBUG_USART_GPIO_CLK           (RCC_APB2Periph_GPIOA)
#define  DEBUG_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd
    
#define  DEBUG_USART_TX_GPIO_PORT       GPIOA   
#define  DEBUG_USART_TX_GPIO_PIN        GPIO_Pin_9
#define  DEBUG_USART_RX_GPIO_PORT       GPIOA
#define  DEBUG_USART_RX_GPIO_PIN        GPIO_Pin_10

#define  DEBUG_USART_IRQ                USART1_IRQn
#define  DEBUG_USART_IRQHandler         USART1_IRQHandler

#define RS485_Rx	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
#define RS485_Tx	GPIO_SetBits(GPIOA,GPIO_Pin_8);
void USART_Config(void);
void USART_Config_1(uint32_t DEBUG);
void USARTx_DMA_Config(void);
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num);
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);
void Uart_DMA_Rx_Data(void);
#endif /* __USART_H */
