/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/ 
#include "board.h"
#include "rtthread.h"
#include <rthw.h>
#include <string.h>
#include "rtc.h"
#include "flash_25q128.h"
#include "protocol.h"
#include "mpu6050.h"
#include <math.h>
#include "adc.h"
#include "DataTreating.h"
#include "Flash_app.h"
#include "calculate.h"

extern vu16 ADC_DMA_IN0; //声明外部变量
extern char Usart_Rx_Buf[USART_RBUFF_SIZE];

/* 定义线程控制块 */
static rt_thread_t usart_thread = RT_NULL;
static rt_thread_t mpu6050_thread = RT_NULL;
static rt_thread_t task3_thread = RT_NULL;

/* 定义信号量控制块 */
rt_sem_t test_sem = RT_NULL;

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void usart_thread_entry(void* parameter);
static void mpu6050_thread_entry(void* parameter);
static void task3_thread_entry(void* parameter);

/* 传感器角度数据修正值（消除芯片固定误差） */
int32_t X_ACCEL_OFFSET=0;
int32_t Y_ACCEL_OFFSET=0;
int32_t Z_ACCEL_OFFSET=0;

/* 传感器加速度数据修正值 */
int32_t X_G_OFFSET=32840;
int32_t Y_G_OFFSET=32840;
int32_t Z_G_OFFSET=32760;

uint16_t ADC0_filter;
uint32_t FLASH_SIZE = 16*1024*1024;		//FLASH 大小为16M字节;
uint32_t FlashCount = 0;				//25q128存储有数据的数量
uint8_t datatemp[1024] = {0};
u8 Buffer[1024] ={0};	//写入25q128缓存

int16_t aacx,aacy,aacz;		//加速度传感器原始数据
int16_t gyrox,gyroy,gyroz;	//陀螺仪原始数据 
float X_G,Y_G,Z_G; 		
double X_Angle,Y_Angle,Z_Angle;

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
    Parameter_Init();		//从flash读出参数
  /* 创建一个信号量 */
	test_sem = rt_sem_create("test_sem",/* 消息队列名字 */
                     0,     /* 信号量初始值，默认有一个信号量 */
                     RT_IPC_FLAG_FIFO); /* 信号量模式 FIFO(0x00)*/
  if (test_sem == RT_NULL)	//信号量创建失败
	return 0;
    rt_thread_delay(1000);
	usart_thread =                          /* 线程控制块指针 */
    rt_thread_create( "usart",              /* 线程名字 */
                      usart_thread_entry,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      5,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (usart_thread != RT_NULL)
        rt_thread_startup(usart_thread);
    else
        return -1;
	mpu6050_thread =                          /* 线程控制块指针 */
    rt_thread_create( "mpu6050",              /* 线程名字 */
                      mpu6050_thread_entry,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      5,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (mpu6050_thread != RT_NULL)
        rt_thread_startup(mpu6050_thread);
    else
        return -1;
	
	task3_thread =                          /* 线程控制块指针 */
    rt_thread_create( "task3",              /* 线程名字 */
                      task3_thread_entry,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      5,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (task3_thread != RT_NULL)
        rt_thread_startup(task3_thread);
    else
        return -1;
}
  
/**@brief      线程定义
* @return      无
* @note          \n
*/
static void usart_thread_entry(void* parameter)
{
	rt_err_t uwRet = RT_EOK;
	RTC_Init();		//RTC初始化	，一定要初始化成功
	RS485_Rx;
    /* 任务都是一个无限循环，不能返回 */
	
  while (1)
  {
	uwRet = rt_sem_take(test_sem,	/* 获取串口中断的信号量 */
                        0); 	  /* 等待时间：0 */
    if(RT_EOK == uwRet)
    {
		if(strncmp(Usart_Rx_Buf,"HBTQ",4) == 0)			//比较包头是否相同
			PacketHandlingModule(Usart_Rx_Buf);		//数据处理
		if(strncmp(Usart_Rx_Buf,"read25q128",10) == 0)	
		{
			RS485_Tx;
			rt_kprintf("5s后数据输出串口波特率修改成115200\n");
			USART_Config_1(115200);
			rt_thread_delay(5000);
			rt_hw_interrupt_disable();	//临界区
			uint32_t k = 5;
			FlashCount = 0;
			SPI_Flash_Read((u8*)datatemp,0,4);
			FlashCount = datatemp[0];
			FlashCount = FlashCount<<8;
			FlashCount |= datatemp[1];
			FlashCount = FlashCount<<8;
			FlashCount |= datatemp[2];
			FlashCount = FlashCount<<8;
			FlashCount |= datatemp[3];
			memset(datatemp,0,1024);/* 清零 */ 

			while((k < FLASH_SIZE) && (k <FlashCount))
			{ 
				SPI_Flash_Read(datatemp,k,1024);		//读取25q128
				Usart_SendArray(USART1,datatemp,1024);	//串口发送
				k += 1024;	
			}
			USART_Config_1(9600);
			rt_hw_interrupt_enable(0); //退出临界区 
		}
		if(strncmp(Usart_Rx_Buf,"read6050",8) == 0)	
		{	int16_t z=0;
			RS485_Tx;
			rt_hw_interrupt_disable();	//临界区
			rt_kprintf("平面修正值:\n");
			z =Flash_Read_twoByte(X_ACCEL_ADDR);
			rt_kprintf("x= %d ",z);
			z = Flash_Read_twoByte(Y_ACCEL_ADDR);
			rt_kprintf("y= %d ",z);
			z = Flash_Read_twoByte(Z_ACCEL_ADDR); 
			rt_kprintf("z= %d \n",z);
			
			rt_kprintf("山坡值:\n");
			z =Flash_Read_twoByte(X_45ACCEL);
			rt_kprintf("x= %d ",z);
			z =Flash_Read_twoByte(Y_45ACCEL);
			rt_kprintf("y= %d ",z);
			z =Flash_Read_twoByte(Z_45ACCEL);
			rt_kprintf("z= %d \n",z);	
			rt_hw_interrupt_enable(0); //退出临界区
		}
		if(strncmp(Usart_Rx_Buf,"eliminate25q128",15) == 0)	
		{
			uint8_t	buf_cle[5] = {0};
			buf_cle[4] = '\n';
			SPI_Flash_Write((u8*)buf_cle,0,5);		//数量清零
			RS485_Tx;
			rt_kprintf("eliminate25q128OK\r\n");
		}
		RS485_Rx;
		memset(Usart_Rx_Buf,0,USART_RBUFF_SIZE);/* 清零 */
    } 
  }
}


/**@brief      线程定义
* @return      无
* @note          \n
*/
static void mpu6050_thread_entry(void* parameter)
{
	MPU_Init();					//初始化MPU6050
	ADC_Configuration();
	
	X_ACCEL_OFFSET = Flash_Read_fourByte(X_ACCEL_ADDR);
	Y_ACCEL_OFFSET = Flash_Read_fourByte(Y_ACCEL_ADDR);
	Z_ACCEL_OFFSET = Flash_Read_fourByte(Z_ACCEL_ADDR);
	
	//AngleCalibration();			//平面角度校准为90°

	while(1)
	{
		//得到加速度传感器数据
		aacx = MPU6050_Get_Data(MPU_ACCEL_XOUTH_REG);
		aacy = MPU6050_Get_Data(MPU_ACCEL_YOUTH_REG);
		aacz = MPU6050_Get_Data(MPU_ACCEL_ZOUTH_REG);
		
		//得到陀螺仪数据
		gyrox = MPU6050_Get_Data(MPU_GYRO_XOUTH_REG);
		gyroy = MPU6050_Get_Data(MPU_GYRO_YOUTH_REG);
		gyroz = MPU6050_Get_Data(MPU_GYRO_ZOUTH_REG);
		
		X_G = ((float)gyrox + (float)X_G_OFFSET)/2048.0;
		Y_G = ((float)gyroy + (float)Y_G_OFFSET)/2048.0;
		Z_G = ((float)gyroz + (float)Z_G_OFFSET)/2048.0;

		X_Angle = acos((aacx+X_ACCEL_OFFSET) / 16384.0)* 57.29577;
		Y_Angle = acos((aacy+Y_ACCEL_OFFSET) / 16384.0)* 57.29577;
		Z_Angle = acos((aacz+Z_ACCEL_OFFSET) / 16384.0)* 57.29577; 
		
		rt_thread_delay(1000);
	}
}

/**@brief       mpu6050修正值计算
* @param[in]    无
* @param[out]   无
* @return       无
*/
void AngleCalibration()
{
	uint16_t t = 0;
	//得到加速度传感器数据
	aacx = MPU6050_Get_Data(MPU_ACCEL_XOUTH_REG);
	aacy = MPU6050_Get_Data(MPU_ACCEL_YOUTH_REG);
	aacz = MPU6050_Get_Data(MPU_ACCEL_ZOUTH_REG);
	X_ACCEL_OFFSET = 0;
	Y_ACCEL_OFFSET = 0;
	Z_ACCEL_OFFSET = 0;
	X_Angle = acos((aacx+X_ACCEL_OFFSET) / 16384.0)* 57.29577;
	Y_Angle = acos((aacy+Y_ACCEL_OFFSET) / 16384.0)* 57.29577;
	Z_Angle = acos((aacz+Z_ACCEL_OFFSET) / 16384.0)* 57.29577; 
	
	if(X_Angle > 90)
	{
		X_ACCEL_OFFSET += (X_Angle - 90)*300;
	}
	else
	{
		X_ACCEL_OFFSET -= (90 - X_Angle)*300;
	}
	
	if(Y_Angle > 90)
	{
		Y_ACCEL_OFFSET += (Y_Angle - 90)*300;
	}
	else
	{
		Y_ACCEL_OFFSET -= (90 -Y_Angle)*300;
	}
	
	if(Z_Angle > 90)
	{
		Z_ACCEL_OFFSET += (Z_Angle - 90)*300;
	}
	else
	{
		Z_ACCEL_OFFSET -= (90 - Z_Angle)*300;
	}
	
	aacx = MPU6050_Get_Data(MPU_ACCEL_XOUTH_REG);
	while(((X_Angle > 90.1) ||(X_Angle < 89.9)) && (t < 20000))
	{
		if(X_Angle > 90)
		{
			X_ACCEL_OFFSET+=10;
		}
		else
		{
			X_ACCEL_OFFSET-=10;
		}
		X_Angle = acos((aacx+X_ACCEL_OFFSET) / 16384.0)* 57.29577;
		t++;
		if((t%100) == 0)
			aacx = MPU6050_Get_Data(MPU_ACCEL_XOUTH_REG);
	}
	
	t = 0;
	aacy = MPU6050_Get_Data(MPU_ACCEL_YOUTH_REG);
	while(((Y_Angle > 90.1) ||(Y_Angle < 89.9)) && (t < 20000))
	{

		if(Y_Angle > 90)
		{
			Y_ACCEL_OFFSET+=10;
		}
		else
		{
			Y_ACCEL_OFFSET-=10;
		}
		Y_Angle = acos((aacy+Y_ACCEL_OFFSET) / 16384.0)* 57.29577;
		t++;
		if((t%100) == 0)
			aacy = MPU6050_Get_Data(MPU_ACCEL_YOUTH_REG);
	}
	
	t = 0;
	aacz = MPU6050_Get_Data(MPU_ACCEL_ZOUTH_REG);
	while(((Z_Angle > 90.1) ||(Z_Angle < 89.9)) && (t < 20000))
	{

		if(Z_Angle > 90)
		{
			Z_ACCEL_OFFSET+=10;
		}
		else
		{
			Z_ACCEL_OFFSET-=10;
		}
		Z_Angle = acos((aacz+Z_ACCEL_OFFSET) / 16384.0)* 57.29577;
		t++;
		if((t%100) == 0)
			aacz = MPU6050_Get_Data(MPU_ACCEL_ZOUTH_REG);
	}
	
	Flash_Write_fourByte(X_ACCEL_ADDR,X_ACCEL_OFFSET);
	Flash_Write_fourByte(Y_ACCEL_ADDR,Y_ACCEL_OFFSET);
	Flash_Write_fourByte(Z_ACCEL_ADDR,Z_ACCEL_OFFSET);
	
}

/**@brief       滤波
* @param[in]    in ：AD原始值
* @return       滤波后结果
* @note        \n
*/
uint32_t filter(uint32_t in)
{
	uint8_t i=0,j=0;
	uint8_t FilteringLevel = 4;	
	uint32_t buf1 = 0,bufz[20]={0};
	static uint32_t buf_P[26]={0},buf2;
	static uint8_t signs = 0,initial = 0;
	
	if(signs >= 26)
		signs = 0;
	
	if(initial < 20)
	{
		buf2 = 0;
		initial++;
		buf_P[signs]=in;
		for(i = 0;i <= signs;)
		{
			buf2+=buf_P[i++];
		}
		signs++;
		return in;
	}
			
	buf_P[signs]=in;			//有效值
	
	j = signs++;
	for(i = 0;i<20;i++)
	{
		bufz[i] = buf_P[j];
		if(j <= 0)
			j = 26;
		j--;
	}
	
	for(i=0; i<20; i++)
	{
		for(j=0; j<20-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}

	buf2 = 0;
	if(FilteringLevel == 1)
	{
		buf2+=bufz[2]+bufz[3]+bufz[4]+bufz[5]+bufz[6]+bufz[7]+bufz[8]+bufz[9]+bufz[10]
		+bufz[11]+bufz[12]+bufz[13]+bufz[14]+bufz[15]+bufz[16]+bufz[17];
		buf2 = buf2 /16.0;
	}
	else if(FilteringLevel == 2)
	{
		buf2+=bufz[4]+bufz[5]+bufz[6]+bufz[7]+bufz[8]+bufz[9]+bufz[10]
		+bufz[11]+bufz[12]+bufz[13]+bufz[14]+bufz[15];
		buf2 = buf2 /12.0;
	}
	else if(FilteringLevel == 3)
	{
		buf2+=bufz[6]+bufz[7]+bufz[8]+bufz[9]+bufz[10]
		+bufz[11]+bufz[12]+bufz[13];
		buf2 = buf2 /8.0;
	}
	else if(FilteringLevel == 6)					//抑制那种周期性电容值突然变小
	{
		buf2+=bufz[14]+bufz[8]+bufz[9]+bufz[10]
		+bufz[11]+bufz[12]+bufz[13];
		buf2 = buf2 /7.0;
	}
	else if(FilteringLevel == 7)					//抑制那种周期性电容值突然变大
	{
		buf2+=bufz[6]+bufz[7]+bufz[8]+bufz[9]+bufz[10]
		+bufz[11]+bufz[5];
		buf2 = buf2 /7.0;
	}
	else
	{
		buf2+=bufz[8]+bufz[9]+bufz[10]+bufz[11];
		buf2 = buf2 /4.0;
	}
	return buf2;
}


/**@brief      线程定义
* @return       无
* @note          \n
*/
static void task3_thread_entry(void* parameter)
{
	uint8_t t = 0,t1 = 0;
	uint32_t WriteAddr = 0;		//写入地址
	uint16_t NumByte = 0;		//写入数量
	SPI_Flash_Init();  			//SPI FLASH 初始化
	TIM3_Init();
	D81_Dispose();				//标定参数初始化
	
	SPI_Flash_Read((u8*)datatemp,0,4);
	WriteAddr = datatemp[0];
	WriteAddr = WriteAddr<<8;
	WriteAddr |= datatemp[1];
	WriteAddr = WriteAddr<<8;
	WriteAddr |= datatemp[2];
	WriteAddr = WriteAddr<<8;
	WriteAddr |= datatemp[3];
	if((WriteAddr == 0) || (WriteAddr > FLASH_SIZE))
		WriteAddr = 5;
	memset(datatemp,0,4);		/* 清零 */ 

	while(1)
	{
		ADC0_filter = filter(ADC_DMA_IN0);		//滤波
		if(calendar.sec != t)
		{
			if(WriteAddr <= (FLASH_SIZE-1000))
			{	
				if(calendar.min != t1)
				{	
					NumByte += sprintf((char*)Buffer+NumByte,"%d-%d-%d:%d",calendar.w_month,calendar.w_date,calendar.hour,calendar.min);
					NumByte += sprintf((char*)Buffer+NumByte,"T=%d\n",ton);
					t1 = calendar.min;
					NumByte += sprintf((char*)Buffer+NumByte,"%d %d %d %d %d %d %d %d %d\n",calendar.sec,ADC_DMA_IN0,ADC0_filter,
					(uint8_t)X_Angle,(uint8_t)Y_Angle,(uint8_t)Z_Angle,(uint16_t)(X_G*100),(uint16_t)(Y_G*100),(uint16_t)(Z_G*100));
				}
				else
				{
					NumByte += sprintf((char*)Buffer+NumByte,"%d %d %d %d %d %d %d %d %d\n",calendar.sec,ADC_DMA_IN0,ADC0_filter,
					(uint8_t)X_Angle,(uint8_t)Y_Angle,(uint8_t)Z_Angle,(uint16_t)(X_G*100.0),(uint16_t)(Y_G*100.0),(uint16_t)(Z_G*100.0));
				}
				if(NumByte >950)
				{
					
					rt_hw_interrupt_disable();	//临界区
					SPI_Flash_Write(Buffer,WriteAddr,NumByte);		//写入记录的数据
					memset(Buffer,0,1024);								/* 清零 */ 
					
					WriteAddr += NumByte;
					Buffer[0] = WriteAddr>>24;
					Buffer[1] = WriteAddr>>16;
					Buffer[2] = WriteAddr>>8;
					Buffer[3] = WriteAddr;
					SPI_Flash_Write((u8*)Buffer,0,4);
					rt_hw_interrupt_enable(0); //退出临界区 
					NumByte = 0;
				}
			}
			t = calendar.sec;
		}
		D01_Dispose(ADC0_filter);
		ton = WeightProcessing(ADC0_filter);
		rt_thread_delay(200);
	}
}
