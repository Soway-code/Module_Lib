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
*                             ������ͷ�ļ�
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

extern vu16 ADC_DMA_IN0; //�����ⲿ����
extern char Usart_Rx_Buf[USART_RBUFF_SIZE];

/* �����߳̿��ƿ� */
static rt_thread_t usart_thread = RT_NULL;
static rt_thread_t mpu6050_thread = RT_NULL;
static rt_thread_t task3_thread = RT_NULL;

/* �����ź������ƿ� */
rt_sem_t test_sem = RT_NULL;

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void usart_thread_entry(void* parameter);
static void mpu6050_thread_entry(void* parameter);
static void task3_thread_entry(void* parameter);

/* �������Ƕ���������ֵ������оƬ�̶��� */
int32_t X_ACCEL_OFFSET=0;
int32_t Y_ACCEL_OFFSET=0;
int32_t Z_ACCEL_OFFSET=0;

/* ���������ٶ���������ֵ */
int32_t X_G_OFFSET=32840;
int32_t Y_G_OFFSET=32840;
int32_t Z_G_OFFSET=32760;

uint16_t ADC0_filter;
uint32_t FLASH_SIZE = 16*1024*1024;		//FLASH ��СΪ16M�ֽ�;
uint32_t FlashCount = 0;				//25q128�洢�����ݵ�����
uint8_t datatemp[1024] = {0};
u8 Buffer[1024] ={0};	//д��25q128����

int16_t aacx,aacy,aacz;		//���ٶȴ�����ԭʼ����
int16_t gyrox,gyroy,gyroz;	//������ԭʼ���� 
float X_G,Y_G,Z_G; 		
double X_Angle,Y_Angle,Z_Angle;

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{	
    Parameter_Init();		//��flash��������
  /* ����һ���ź��� */
	test_sem = rt_sem_create("test_sem",/* ��Ϣ�������� */
                     0,     /* �ź�����ʼֵ��Ĭ����һ���ź��� */
                     RT_IPC_FLAG_FIFO); /* �ź���ģʽ FIFO(0x00)*/
  if (test_sem == RT_NULL)	//�ź�������ʧ��
	return 0;
    rt_thread_delay(1000);
	usart_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "usart",              /* �߳����� */
                      usart_thread_entry,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      5,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (usart_thread != RT_NULL)
        rt_thread_startup(usart_thread);
    else
        return -1;
	mpu6050_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "mpu6050",              /* �߳����� */
                      mpu6050_thread_entry,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      5,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (mpu6050_thread != RT_NULL)
        rt_thread_startup(mpu6050_thread);
    else
        return -1;
	
	task3_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "task3",              /* �߳����� */
                      task3_thread_entry,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      5,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (task3_thread != RT_NULL)
        rt_thread_startup(task3_thread);
    else
        return -1;
}
  
/**@brief      �̶߳���
* @return      ��
* @note          \n
*/
static void usart_thread_entry(void* parameter)
{
	rt_err_t uwRet = RT_EOK;
	RTC_Init();		//RTC��ʼ��	��һ��Ҫ��ʼ���ɹ�
	RS485_Rx;
    /* ������һ������ѭ�������ܷ��� */
	
  while (1)
  {
	uwRet = rt_sem_take(test_sem,	/* ��ȡ�����жϵ��ź��� */
                        0); 	  /* �ȴ�ʱ�䣺0 */
    if(RT_EOK == uwRet)
    {
		if(strncmp(Usart_Rx_Buf,"HBTQ",4) == 0)			//�Ƚϰ�ͷ�Ƿ���ͬ
			PacketHandlingModule(Usart_Rx_Buf);		//���ݴ���
		if(strncmp(Usart_Rx_Buf,"read25q128",10) == 0)	
		{
			RS485_Tx;
			rt_kprintf("5s������������ڲ������޸ĳ�115200\n");
			USART_Config_1(115200);
			rt_thread_delay(5000);
			rt_hw_interrupt_disable();	//�ٽ���
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
			memset(datatemp,0,1024);/* ���� */ 

			while((k < FLASH_SIZE) && (k <FlashCount))
			{ 
				SPI_Flash_Read(datatemp,k,1024);		//��ȡ25q128
				Usart_SendArray(USART1,datatemp,1024);	//���ڷ���
				k += 1024;	
			}
			USART_Config_1(9600);
			rt_hw_interrupt_enable(0); //�˳��ٽ��� 
		}
		if(strncmp(Usart_Rx_Buf,"read6050",8) == 0)	
		{	int16_t z=0;
			RS485_Tx;
			rt_hw_interrupt_disable();	//�ٽ���
			rt_kprintf("ƽ������ֵ:\n");
			z =Flash_Read_twoByte(X_ACCEL_ADDR);
			rt_kprintf("x= %d ",z);
			z = Flash_Read_twoByte(Y_ACCEL_ADDR);
			rt_kprintf("y= %d ",z);
			z = Flash_Read_twoByte(Z_ACCEL_ADDR); 
			rt_kprintf("z= %d \n",z);
			
			rt_kprintf("ɽ��ֵ:\n");
			z =Flash_Read_twoByte(X_45ACCEL);
			rt_kprintf("x= %d ",z);
			z =Flash_Read_twoByte(Y_45ACCEL);
			rt_kprintf("y= %d ",z);
			z =Flash_Read_twoByte(Z_45ACCEL);
			rt_kprintf("z= %d \n",z);	
			rt_hw_interrupt_enable(0); //�˳��ٽ���
		}
		if(strncmp(Usart_Rx_Buf,"eliminate25q128",15) == 0)	
		{
			uint8_t	buf_cle[5] = {0};
			buf_cle[4] = '\n';
			SPI_Flash_Write((u8*)buf_cle,0,5);		//��������
			RS485_Tx;
			rt_kprintf("eliminate25q128OK\r\n");
		}
		RS485_Rx;
		memset(Usart_Rx_Buf,0,USART_RBUFF_SIZE);/* ���� */
    } 
  }
}


/**@brief      �̶߳���
* @return      ��
* @note          \n
*/
static void mpu6050_thread_entry(void* parameter)
{
	MPU_Init();					//��ʼ��MPU6050
	ADC_Configuration();
	
	X_ACCEL_OFFSET = Flash_Read_fourByte(X_ACCEL_ADDR);
	Y_ACCEL_OFFSET = Flash_Read_fourByte(Y_ACCEL_ADDR);
	Z_ACCEL_OFFSET = Flash_Read_fourByte(Z_ACCEL_ADDR);
	
	//AngleCalibration();			//ƽ��Ƕ�У׼Ϊ90��

	while(1)
	{
		//�õ����ٶȴ���������
		aacx = MPU6050_Get_Data(MPU_ACCEL_XOUTH_REG);
		aacy = MPU6050_Get_Data(MPU_ACCEL_YOUTH_REG);
		aacz = MPU6050_Get_Data(MPU_ACCEL_ZOUTH_REG);
		
		//�õ�����������
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

/**@brief       mpu6050����ֵ����
* @param[in]    ��
* @param[out]   ��
* @return       ��
*/
void AngleCalibration()
{
	uint16_t t = 0;
	//�õ����ٶȴ���������
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

/**@brief       �˲�
* @param[in]    in ��ADԭʼֵ
* @return       �˲�����
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
			
	buf_P[signs]=in;			//��Чֵ
	
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
	else if(FilteringLevel == 6)					//�������������Ե���ֵͻȻ��С
	{
		buf2+=bufz[14]+bufz[8]+bufz[9]+bufz[10]
		+bufz[11]+bufz[12]+bufz[13];
		buf2 = buf2 /7.0;
	}
	else if(FilteringLevel == 7)					//�������������Ե���ֵͻȻ���
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


/**@brief      �̶߳���
* @return       ��
* @note          \n
*/
static void task3_thread_entry(void* parameter)
{
	uint8_t t = 0,t1 = 0;
	uint32_t WriteAddr = 0;		//д���ַ
	uint16_t NumByte = 0;		//д������
	SPI_Flash_Init();  			//SPI FLASH ��ʼ��
	TIM3_Init();
	D81_Dispose();				//�궨������ʼ��
	
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
	memset(datatemp,0,4);		/* ���� */ 

	while(1)
	{
		ADC0_filter = filter(ADC_DMA_IN0);		//�˲�
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
					
					rt_hw_interrupt_disable();	//�ٽ���
					SPI_Flash_Write(Buffer,WriteAddr,NumByte);		//д���¼������
					memset(Buffer,0,1024);								/* ���� */ 
					
					WriteAddr += NumByte;
					Buffer[0] = WriteAddr>>24;
					Buffer[1] = WriteAddr>>16;
					Buffer[2] = WriteAddr>>8;
					Buffer[3] = WriteAddr;
					SPI_Flash_Write((u8*)Buffer,0,4);
					rt_hw_interrupt_enable(0); //�˳��ٽ��� 
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
