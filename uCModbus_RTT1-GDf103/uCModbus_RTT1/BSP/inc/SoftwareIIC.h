#ifndef __SOFTWAREIIC_H__
#define __SOFTWAREIIC_H__

#include "stm32f10x.h"

#define AT24C01			127
#define AT24C02			255
#define AT24C04			511
#define AT24C08			1023
#define AT24C16			2047
#define AT24C32			4095
#define AT24C64	    	        8191
#define AT24C128		16383
#define AT24C256		32767
#define EE_TYPE			AT24C16

#define EEPROM_ADDRESS       0xA0


//iic配置为SCL为PB6/SDA为PB7
//#define SDA_IN()  {GPIOB->MODER &= ~(0x03<<14);}		//配置PB7为输入模式
//#define SDA_OUT() {GPIOB->MODER |= 0x01<<14; GPIOB->OTYPER |= 0x00<<14; GPIOB->OSPEEDR |= 0x11<<14;}//配置PB7为输出模式

#define Set_IIC_SCL		GPIO_SetBits(GPIOB, GPIO_Pin_6)
#define Reset_IIC_SCL		GPIO_ResetBits(GPIOB, GPIO_Pin_6)
#define Set_IIC_SDA		GPIO_SetBits(GPIOB, GPIO_Pin_7)
#define Reset_IIC_SDA		GPIO_ResetBits(GPIOB, GPIO_Pin_7)
#define READ_SDA		GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)
#define Write_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_7, x?Bit_SET:Bit_RESET)

//IIC所有操作函数
void Delay_Ms(u32 cnt);
void Delay_Us(u32 cnt);
void IIC_Init(void);
u8 I2C1_ReadByte(u16 DriverAddr, u16 ReadAddr);						//指定地址读取一个字节
void I2C1_WriteByte(uint16_t DriverAddr, u16 WriteAddr, u8 DataToWrite);		//指定地址写入一个字节
void I2C1_WriteNBytes(uint16_t DriverAddr, u16 WriteAddr, u16 NumToWrite, const u8 *pBuffer);  //从指定地址开始读出指定长度的数据
void I2C1_WNBytesMul3T(uint16_t DriverAddr, u16 WriteAddr, u16 NumToWrite, const u8 *pBuffer); //从指定地址开始读出指定长度的数据
void I2C1_ReadNBytes(u16 DriverAddr, u16 ReadAddr, u16 NumToRead, u8 *pBuffer);		//从指定地址开始写入指定长度的数据
uint16_t I2C1_Read2Bytes(u16 DriverAddr, u16 ReadAddr);
#endif
