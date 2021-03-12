#ifndef __ADXL345_H
#define __ADXL345_H	 
#include "BSP.h"
#include "stdint.h"

/*端口定义*/ 
#define Set_SCL		GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define Reset_SCL		GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define Set_SDA		GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define Reset_SDA		GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define DataBit_SDA		GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8)
#define WriteBit_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, x?Bit_SET:Bit_RESET)

//#define 	RCC_ADXL345_RCC     RCC_APB2Periph_GPIOB
//#define 	RCC_ADXL345		GPIOB
//#define	  	SCL 			GPIO_Pin_9      //IIC时钟引脚定义
//#define 	SDA				GPIO_Pin_8      //IIC数据引脚定义

#define	SlaveAddress   0xA6	  //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
                              //ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A

void ADXL345_IIC_Init(void);
void Init_ADXL345(void);
uint8_t Single_Read_ADXL345(uint8_t REG_Address);
void Multiple_read_ADXL345(void);
void Multiple_read1_ADXL345(void);
#endif
