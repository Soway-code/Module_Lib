#ifndef _LED_H_
#define _LED_H_

#include "stm32f0xx.h"
#include  <includes.h>

void LED_Init(void);
void LED_Open(void);
void LED_Close(void);


extern unsigned long ulLedStatue;
void LED_Double_Light(void *p_tmr, void *p_arg);


#endif


