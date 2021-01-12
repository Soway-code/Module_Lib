#ifndef _BSP_H_
#define _BSP_H_

#include "string.h"
#include "stm32f10x.h"
#include "stm32f10x_sys.h"
#include "sys.h" 
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_adc.h"
#include "stdio.h"
#include "stm32f10x_gpio.h"
#include "Flash_app.h"
#include "stm32f10x_flash.h"
#include "task.h"
//#include "Bsp_init.h"
//#include "nvic.h"
////#include "delay.h"
////#include "usart.h"
////#include "usart3.h"
////#include "led.h"
#include "SoftwareIIC.h"
#include "AT24Cxx.h"

//#include "DA.h"
//#include "DI.h"
//#include "modbus.h"
#include "type.h"
#include "algorithm.h"
//#include "sys_cfg.h"
//#include "Led.h"
#include "Analog.h"
//#include "Modbus.h"
//#include "UartMaster.h"
//#include "LNGProtocol.h"
#include "LoadTask.h"
//#include  "Weight.h"
////#include "modbus_asc.h"
////#include "modbus_ascii.h"

//#include  <includes.h>

//extern OS_TCB          AppTaskMBTCB;
//extern OS_TCB          AppDITCB;
//extern OS_TCB          AppAnalogTCB;


extern void RunVar_Init(void);

//void Dev_Init();

#endif
