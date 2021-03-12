#ifndef __DATDTREATING_H
#define __DATDTREATING_H
#include "stm32f10x_tim.h"
#include "protocol.h"

void TIM3_Init(void);
void D01_Dispose(uint16_t ADC0);	//D01功能码报警标志位空重载标志位处理
void D81_Dispose(void);

#endif
