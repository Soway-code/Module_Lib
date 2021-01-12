/*****************************************************************************
* @file    Project/Template/stm32f10x_sys.c 
* @author  ktfe
* @date    14/03/2012
* @brief   system init program body
*****************************************************************************/

#include "stm32f10x_sys.h"

//系统芯片控制逻辑软复位
void Soft_Reset(void)
{   
	SCB->AIRCR = 0X05FA0000|(u32)0x04;	  
} 
//开中断
void Enable_IRQ(void)
{
	__ASM volatile ("cpsie i");
}
//关中断
void Disable_IRQ(void)
{
	__ASM volatile ("cpsid i");
}
//JTAG模式设置,用于设置JTAG的模式
//mode:jtag,swd模式设置;
//0:全使能;  1:使能SWD;  2:全关闭;		  
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp = mode;
	temp <<= 25;
	RCC->APB2ENR |= 0x00000001;     //开启辅助时钟	   
	AFIO->MAPR &= 0XF8FFFFFF;       //清除MAPR的[26:24]
	AFIO->MAPR |= temp;             //设置jtag模式
	RCC->APB2ENR &= 0xfffffffe;     //关闭辅助时钟
}

