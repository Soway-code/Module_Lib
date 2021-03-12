
#include "led.h"

unsigned long ulLedStatue = 0;


void LED_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;  

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    LED_Close();
}

void LED_Open(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}

void LED_Close(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
}



