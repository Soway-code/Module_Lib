/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                          (c) Copyright 2003-2009; Micrium, Inc.; Weston, FL               
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            uC/Modbus
*
*                                      MODBUS BOARD SUPPORT PACKAGE
*                                         STM32F103 (ARM7)
*
* Filename    : mb_bsp.c
* Version     : V2.12
* Programmers : Ktfe
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "cpu.h"
#include "mb.h"
#include "BSP.h"

#define 	COM1		USART1 
#define 	COM2		USART2 
#define 	COM3		USART3 

extern uint32_t rd485t;
/*
*********************************************************************************************************
*                                             MB_CommExit()
*
* Description : This function is called to terminate Modbus communications.  All Modbus channels are close.
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void 
MB_CommExit (void) 
{ 
	CPU_INT08U   ch;
	MODBUS_CH   *pch;

	pch = &MB_ChTbl[0];
	for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {
			MB_CommTxIntDis(pch);
			MB_CommRxIntDis(pch);
			pch++;
	}
}

/*
*********************************************************************************************************
*                                           MB_CommPortCfg()
*
* Description : This function initializes the serial port to the desired baud rate and the UART will be
*               configured for N, 8, 1 (No parity, 8 bits, 1 stop).
*
* Argument(s) : pch        is a pointer to the Modbus channel
*               port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a sepcific Modbus channel.
*               baud       is the desired baud rate for the serial port.
*               parity     is the desired parity and can be either:
*
*                          MODBUS_PARITY_NONE
*                          MODBUS_PARITY_ODD
*                          MODBUS_PARITY_EVEN
*
*               bits       specifies the number of bit and can be either 7 or 8.
*               stops      specifies the number of stop bits and can either be 1 or 2
*
* Return(s)   : none.
*
* Caller(s)   : MB_CfgCh()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void 
MB_CommPortCfg (	MODBUS_CH *pch, 			// a pointer to the communication channel to configure;
									CPU_INT08U port_nbr, 	// 使用的串口物理号
									CPU_INT32U baud, 			// 波特率
									CPU_INT08U bits, 			// 位数
									CPU_INT08U parity, 		// 校验方式
									CPU_INT08U stops )		// 停止位数
{
	NVIC_InitTypeDef NVIC_InitStruct;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	USART_InitTypeDef USART_InitStructure;
	USART_TypeDef * COM;
	
	if ( pch != (MODBUS_CH *)0 ) 
	{
		pch->PortNbr        = port_nbr;			// Store configuration in channel
		pch->BaudRate       = baud;
		pch->Parity         = parity;
		pch->Bits           = bits;
		pch->Stops          = stops;
		
		USART_StructInit(&USART_InitStructure);
		USART_InitStructure.USART_BaudRate = baud;
		
		switch ( bits )		//数据位数
		{
			case 8 : USART_InitStructure.USART_WordLength = USART_WordLength_8b; break;
			case 9 : USART_InitStructure.USART_WordLength = USART_WordLength_9b; break;
			default : break;
		}
		switch ( parity )	//校验方式
		{
			case MODBUS_PARITY_ODD : USART_InitStructure.USART_Parity = USART_Parity_Odd; break;
			case MODBUS_PARITY_EVEN : USART_InitStructure.USART_Parity = USART_Parity_Even; break;
			case MODBUS_PARITY_NONE : USART_InitStructure.USART_Parity = USART_Parity_No; break;
			default : break;
		}
		switch ( stops )	//停止位
		{
			case 1 : USART_InitStructure.USART_StopBits = USART_StopBits_1; break;
      case 2 : USART_InitStructure.USART_StopBits = USART_StopBits_2; break;
      default : break;
		}
		switch ( port_nbr )	//物理端口
		{
			case 1 : 
			{
				RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
				GPIOA->CRH &= 0xfffff00f;
				GPIOA->CRH |= 0x000004b0;	//PA10、PA9
				RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
				COM = COM1;
				NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
				NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
				NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
			}	break; 
			
			case 2 : 
			{
				RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
				GPIOA->CRL &= 0xffff00ff;
				GPIOA->CRL |= 0x00004b00;	//PA3、PA2
				RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
				COM = COM2;
				NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
				NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
				NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
			}	break; 
			
			case 3 : 
			{
				RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
				GPIOB->CRH &= 0xffff00ff;
				GPIOB->CRH |= 0x00004b00;	//PB11、PB10
				RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
				COM = COM3;
				NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
				NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
				NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
			}	break; 
			
			default : break;
		}

		USART_ClockStructInit(&USART_ClockInitStruct);
		USART_ClockInitStruct.USART_Clock = USART_Clock_Disable;
		USART_ClockInitStruct.USART_CPOL = USART_CPOL_Low;
		USART_ClockInitStruct.USART_CPHA = USART_CPHA_2Edge;
		USART_ClockInitStruct.USART_LastBit = USART_LastBit_Disable;
		USART_ClockInit(COM, &USART_ClockInitStruct);
		USART_Init(COM, &USART_InitStructure);
		
		USART_ITConfig(COM, USART_IT_RXNE, ENABLE);			//接收中断
		USART_ITConfig(COM, USART_IT_TC, ENABLE);				//发送完成中断
		USART_Cmd(COM, ENABLE);													//使能外设
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);
	}
}

/*
*********************************************************************************************************
*                                       MB_CommRxTxISR_Handler()
*
* Description : This function is the ISR for either a received or transmitted character.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is a ISR
*
* Note(s)     : (1) The pseudo-code for this function should be:  
*
*               if (Rx Byte has been received) {
*                  c = get byte from serial port;
*                  Clear receive interrupt;
*                  pch->RxCtr++;                      Increment the number of bytes received
*                  MB_RxByte(pch, c);                 Pass character to Modbus to process
*              }
*
*              if (Byte has been transmitted) {
*                  pch->TxCtr++;                      Increment the number of bytes transmitted
*                  MB_TxByte(pch);                    Send next byte in response
*                  Clear transmit interrupt           Clear Transmit Interrupt flag
*              }
*********************************************************************************************************
*/
void MB_CommRxTxISR_1_Handler (void) 
{ 
	CPU_INT08U c; 
	CPU_INT08U ch; 
	MODBUS_CH *pch; 
	
	pch = &MB_ChTbl[0];
	for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) 
	{ 
		if ( pch->PortNbr == 1 ) 
		{ 
			if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET ) 
			{ 
				c = USART_ReceiveData(USART1); 
				USART_ClearFlag(USART1, USART_FLAG_RXNE);
				pch->RxCtr++; 
				MB_RxByte(pch, c); 	// Pass byte to Modbus to process 
			} 
			if ( USART_GetFlagStatus(USART1, USART_FLAG_TC) == SET ) 
			{ 		
					
				pch->TxCtr++; 
				MB_TxByte(pch); 		// Send next byte in response 
				USART_ClearFlag(USART1, USART_FLAG_TC); 
				
			} 
			break; 
		} 
		else 
		{ 
			pch++; 
		} 
		
	}
	//GPIO_ResetBits(GPIOA,GPIO_Pin_8);
}

void MB_CommRxTxISR_2_Handler (void) 
{ 
	CPU_INT08U c; 
	CPU_INT08U ch; 
	MODBUS_CH *pch; 
	
	pch = &MB_ChTbl[0];
	for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) 
	{ 
		if ( pch->PortNbr == 2 ) 
		{ 
			if ( USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET ) 
			{ 
				c = USART_ReceiveData(USART2); 
				USART_ClearFlag(USART2, USART_FLAG_RXNE);
				pch->RxCtr++; 
				MB_RxByte(pch, c); 	// Pass byte to Modbus to process 
			} 
			if ( USART_GetFlagStatus(USART2, USART_FLAG_TC) == SET ) 
			{ 
				pch->TxCtr++; 
				MB_TxByte(pch); 		// Send next byte in response 
				USART_ClearFlag(USART2, USART_FLAG_TC); 
			} 
			break; 
		} 
		else 
		{ 
			pch++; 
		} 
	}
}

void MB_CommRxTxISR_3_Handler (void) 
{ 
	CPU_INT08U c; 
	CPU_INT08U ch; 
	MODBUS_CH *pch; 
	
	pch = &MB_ChTbl[0];
	for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) 
	{ 
		if ( pch->PortNbr == 3 ) 
		{ 
			if ( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET ) 
			{ 
				c = USART_ReceiveData(USART3); 
				USART_ClearFlag(USART3, USART_FLAG_RXNE);
				pch->RxCtr++; 
				MB_RxByte(pch, c); 	// Pass byte to Modbus to process 
			} 
			if ( USART_GetFlagStatus(USART3, USART_FLAG_TC) == SET ) 
			{ 
				
				pch->TxCtr++; 
				MB_TxByte(pch); 		// Send next byte in response 
				USART_ClearFlag(USART3, USART_FLAG_TC); 
			} 
			break; 
		} 
		else 
		{ 
			pch++; 
		} 
	}
}

//物理中断入口
void USART1_IRQHandler (void)
{
	MB_CommRxTxISR_1_Handler();
}
void USART2_IRQHandler (void)
{
	MB_CommRxTxISR_2_Handler();
}
void USART3_IRQHandler (void)
{
	MB_CommRxTxISR_3_Handler();
}

/*
*********************************************************************************************************
*                                             MB_CommTx1()
*
* Description : This function is called to obtain the next byte to send from the transmit buffer.  When
*               all bytes in the reply have been sent, transmit interrupts are disabled and the receiver
*               is enabled to accept the next Modbus request.
*
* Argument(s) : c     is the byte to send to the serial port
*
* Return(s)   : none.
*
* Caller(s)   : MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void MB_CommTx1 (	MODBUS_CH  *pch, 
									CPU_INT08U c	) 
{ 
//	if(rd485t == 0)
//	{
//		rd485t = 1;
//		GPIO_SetBits(GPIOA,GPIO_Pin_8);
//		rt_thread_mdelay(60);/* 延时1000个tick */	
//	}
//	
	if((rd485t == 0 )|| (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == 0))
	{	
		GPIO_SetBits(GPIOA,GPIO_Pin_8);
	}	
	switch (pch->PortNbr) 
	{ 
		case 1 : USART_SendData(USART1, c); break;
		case 2 : USART_SendData(USART2, c); break;
		case 3 : USART_SendData(USART3, c); break;
		default : break;
	} 
//	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
}

/*
*********************************************************************************************************
*                                         MB_CommTxIntEn()
*
* Description : This function enables Tx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_Tx()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void MB_CommTxIntEn(MODBUS_CH *pch) 
{ 
	switch (pch->PortNbr) 
	{ 
		case 1 : USART_ITConfig(USART1, USART_IT_TC, ENABLE); break;
		case 2 : USART_ITConfig(USART2, USART_IT_TC, ENABLE); break;
		case 3 : USART_ITConfig(USART3, USART_IT_TC, ENABLE); break;
		default : break;
	} 
}
void MB_CommRxIntEn(MODBUS_CH *pch) 
{ 
	switch (pch->PortNbr) 
	{ 
		case 1 : USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); break;
		case 2 : USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); break;
		case 3 : USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); break;
		default : break;
	} 
}

/*
*********************************************************************************************************
*                                         MB_CommTxIntDis()
*
* Description : This function disables Tx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommExit()
*               MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void MB_CommTxIntDis(MODBUS_CH *pch) 
{ 
	switch (pch->PortNbr) 
	{ 
		case 1 : USART_ITConfig(USART1, USART_IT_TC, DISABLE); break;
		case 2 : USART_ITConfig(USART2, USART_IT_TC, DISABLE); break;
		case 3 : USART_ITConfig(USART3, USART_IT_TC, DISABLE); break;
		default : break;
	} 
}

void MB_CommRxIntDis(MODBUS_CH *pch) 
{ 
	switch (pch->PortNbr) 
	{ 
		case 1 : USART_ITConfig(USART1, USART_IT_RXNE, DISABLE); break;
		case 2 : USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); break;
		case 3 : USART_ITConfig(USART3, USART_IT_RXNE, DISABLE); break;
		default : break;
	} 
}

/*
*********************************************************************************************************
*                                           MB_RTU_TmrInit()
*
* Description : This function is called to initialize the RTU timeout timer.
*
* Argument(s) : freq          Is the frequency of the modbus RTU timer interrupt.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrInit (void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* TIM4 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = (uint16_t) MB_RTU_Freq; 	//自动重装值,1000表示中断频率为1K
	TIM_TimeBaseStructure.TIM_Prescaler = 72;			 							//预分频系数,72分频为1MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //两级占先 八级副优先
	/* Enable the TIM4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
	
	/* TIM IT ENABLE */
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	/* TIM4 ENABLE Counter */
	TIM_Cmd(TIM4,  ENABLE);
	
	MB_RTU_TmrResetAll(); 			// Reset all the RTU timers, we changed freq. 
}
#endif
/*
*********************************************************************************************************
*                                           MB_RTU_TmrExit()
*
* Description : This function is called to disable the RTU timeout timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit()
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrExit (void)
{
	TIM_Cmd(TIM4, DISABLE);
	TIM_SetCounter(TIM4,0x0000); 
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}
#endif
/*
*********************************************************************************************************
*                                       MB_RTU_TmrISR_Handler()
*
* Description : This function handles the case when the RTU timeout timer expires.
*
* Arguments   : none.
*
* Returns     : none.
*
* Caller(s)   : This is a ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrISR_Handler (void)
{
	MB_RTU_TmrCtr++;         // Indicate that we had activities on this interrupt
  MB_RTU_TmrUpdate();      // Check for RTU timers that have expired
}

void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  {
    /* Clear TIM4 Capture Compare1 interrupt pending bit*/
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		MB_RTU_TmrISR_Handler();
  }
}
#endif
