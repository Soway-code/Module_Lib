#ifndef __IWDG_H
#define __IWDG_H	 
#include "sys.h"
#include "stm32f10x_iwdg.h"

#define pre		IWDG_Prescaler_64 //��Ƶֵ��Χ��4,8,16,32,64,128,256
#define rlr		625 //��װ��ֵ��Χ��0��0xFFF��4095��//1S


void IWDG_Init(void);
void IWDG_Feed(void);
		 				    
#endif
