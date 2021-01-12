/*****************************************************************************
* @file    delay.c 
* @author  ktfe
* @version V1.0.0
* @date    22/04/2013
* @brief   72MHz
*****************************************************************************/

#include "delay.h"

void delay_us(unsigned int us)
{													
	unsigned int _dcnt; 
	_dcnt=(us*8); 				
	while(_dcnt-- > 0) 	
	{ continue; }				
}

void delay_ms(unsigned int ms)
{										
	unsigned long _dcnt;	
	_dcnt=(ms*8000); 			
	while(_dcnt-- > 0) 		
	{ continue; }					
}
