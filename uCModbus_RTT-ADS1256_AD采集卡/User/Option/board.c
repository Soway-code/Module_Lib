/*****************************************************************************
* @file    Board.c 
* @author  ktfe
* @version V3.0.0
* @date    06/09/2012
* @brief   STM32F103RBT6板载配置(配置时钟和GPIO)
*****************************************************************************/

#include "Board.h"
#include "delay.h"
#include "stm32f10x_sys.h"

void LCD_init(void)
{
	JTAG_Set(1); //使用SW模式,释放PB部分IO口
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	GPIOB->CRL &= 0x00000000;
	GPIOB->CRL |= 0x33333333;
	GPIOB->CRH &= 0x00000000;
	GPIOB->CRH |= 0x33333333;
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	GPIOC->CRH &= 0x00000000;
	GPIOC->CRH |= 0x33333333;
}
void BEEP_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRH &= 0xfffffff0;
	GPIOA->CRH |= 0x00000003;
	BEEP = 0;
}
void LED_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	#ifdef	LED11
		GPIOC->CRL &= 0xfff0ffff;
		GPIOC->CRL |= 0x00030000;
	#endif
	#ifdef	LED22
		GPIOC->CRL &= 0xff0fffff;
		GPIOC->CRL |= 0x00300000;
	#endif
	#ifdef	LED33
		GPIOC->CRL &= 0xf0ffffff;
		GPIOC->CRL |= 0x03000000;
	#endif
	#ifdef	LED44
		GPIOC->CRL &= 0x0fffffff;
		GPIOC->CRL |= 0x30000000;
	#endif
}
void KEY_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	#ifdef	KEY11
		GPIOC->CRL &= 0xfffffff0;
		GPIOC->CRL |= 0x00000008;
		GPIOC->ODR |= 0x00000001;//上拉输入
	#endif
	#ifdef	KEY22
		GPIOC->CRL &= 0xffffff0f;
		GPIOC->CRL |= 0x00000080;
		GPIOC->ODR |= 0x00000002;//上拉输入
	#endif
	#ifdef	KEY33
		GPIOC->CRL &= 0xfffff0ff;
		GPIOC->CRL |= 0x00000800;
		GPIOC->ODR |= 0x00000004;//上拉输入
	#endif
	#ifdef	KEY44
		GPIOC->CRL &= 0xffff0fff;
		GPIOC->CRL |= 0x00008000;
		GPIOC->ODR |= 0x00000008;//上拉输入
	#endif
}
void NAV_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRL &= 0xfff00000;
	GPIOA->CRL |= 0x00044444;
}
void Board_init(void)
{
	#ifdef	LCD
		LCD_init();
	#endif
	#ifdef	LED
		LED_init();
	#endif
	#ifdef	BELL
		BEEP_init();
	#endif
	#ifdef	KEY
		KEY_init();
	#endif
	#ifdef	NAV
		NAV_init();
	#endif
}
uint8_t Key_check(void)
{
	if((GPIOA->IDR & 0x001f) != 0x001f)	//检测到按键
	{
		delay_ms(10);	//消抖
		if((GPIOA->IDR & 0x001f) != 0x001f)
		{
			return 1;
		}
	}
	return 0;
}
KeyState Key_event(void)
{
	KeyState key_value;
	
	switch(GPIOA->IDR & 0x001f)
	{
		case 0x000f : key_value = LEFT; break;	//Nav_LEFT
		case 0x0017 : key_value = RIGHT; break;	//Nav_RIGHT
		case 0x001b : key_value = CONTER; break;	//Nav_CONTER
		case 0x001d : key_value = DOWN; break;	//Nav_DOWN
		case 0x001e : key_value = UP; break;	//Nav_UP
		default : break;
	}
	while((GPIOA->IDR & 0x001f) != 0x001f);//等待按键松开
	return key_value;
}
