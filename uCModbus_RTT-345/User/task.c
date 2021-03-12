/**@file        task.c
* @brief        任务模块
* @details      创建任务与执行任务
* @author       马灿林
* @date         2020-08-14
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/08/01  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#include "BSP.h"
#include "iwdg.h"
#include "ADXL345.h"
#include "spi.h"
#include "flash.h"
#include <rtthread.h>
#include <rthw.h>
#include "rtc.h" 
extern unsigned char	ADXL345_BUF[6];  
int  dis_data[3]; 
double ldVolutage,ldVolutag[8];
long ulResult;
extern volatile uint16_t MB_DATA_04[610];
uint32_t buf1,rd485t = 1;
uint16_t redeeming[9];


static struct rt_thread task1_thread;			 	//定义线程控制块 
static rt_uint8_t rt_task1_thread_stack[512];		//定义线程栈

static struct rt_thread task2_thread;				//定义线程控制块 
static rt_uint8_t rt_task2_thread_stack[512];		//定义线程栈

static struct rt_thread task3_thread;				//定义线程控制块 
static rt_uint8_t rt_task3_thread_stack[512];		//定义线程栈

static struct rt_thread task4_thread;				//定义线程控制块 
static rt_uint8_t rt_task4_thread_stack[512];		//定义线程栈

//static struct rt_thread task5_thread;				//定义线程控制块 
//static rt_uint8_t rt_task5_thread_stack[512];		//定义线程栈

//函数声明
static void task1_thread_entry(void* parameter);
static void task2_thread_entry(void* parameter);
static void task3_thread_entry(void* parameter);
static void task4_thread_entry(void* parameter);
//static void task5_thread_entry(void* parameter);

#define BANK0_WRITE_START_ADDR  ((uint32_t)0x08010000)

void RS485()
{
	GPIO_InitTypeDef GPIO_InitStructure;
    //RCC->APB2ENR|=1<<4;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	
}
/**@brief       任务的初始化
* @return       函数执行结果1
*/
uint8_t task_init(void) 
{
	Target_Init();	//系统配置
	MB_Init(1000);
	
	EEProm_Init();
	Rd_Dev_Param();				//读取参数包括modbus初始化
	//IWDG_Init();
	rt_thread_init(&task1_thread, 						//线程控制块 
					"task1", 							//线程名字 
					task1_thread_entry, 				// 线程入口函数 
					RT_NULL, 							// 线程入口函数参数 
					&rt_task1_thread_stack[0],			// 线程栈起始地址 
					sizeof(rt_task1_thread_stack), 		// 线程栈大小 
					3, 									// 线程的优先级 
					20); 								// 线程时间片 
	
	rt_thread_init(&task2_thread, 						// 线程控制块 
					"task2", 							// 线程名字 
					task2_thread_entry, 				// 线程入口函数 
					RT_NULL, 							// 线程入口函数参数 
					&rt_task2_thread_stack[0],			// 线程栈起始地址 
					sizeof(rt_task2_thread_stack), 		// 线程栈大小 
					4, 									// 线程的优先级 
					20); 								// 线程时间片 
					
	rt_thread_init(&task3_thread, 						// 线程控制块 
					"task3", 							// 线程名字 
					task3_thread_entry, 				// 线程入口函数 
					RT_NULL, 							// 线程入口函数参数 
					&rt_task3_thread_stack[0],			// 线程栈起始地址 
					sizeof(rt_task3_thread_stack), 		// 线程栈大小 
					5, 									// 线程的优先级 
					20); 								// 线程时间片 
					
	rt_thread_init(&task4_thread, 						// 线程控制块 
					"task4", 							// 线程名字 
					task4_thread_entry, 				// 线程入口函数 
					RT_NULL, 							// 线程入口函数参数 
					&rt_task4_thread_stack[0],			// 线程栈起始地址 
					sizeof(rt_task4_thread_stack), 		// 线程栈大小 
					5, 									// 线程的优先级 
					20); 
					
//	rt_thread_init(&task5_thread, 						// 线程控制块 
//					"task5", 							// 线程名字 
//					task5_thread_entry, 				// 线程入口函数 
//					RT_NULL, 							// 线程入口函数参数 
//					&rt_task5_thread_stack[0],			// 线程栈起始地址 
//					sizeof(rt_task5_thread_stack), 		// 线程栈大小 
//					7, 									// 线程的优先级 
//					20); 
					
	rt_thread_startup(&task3_thread); 					// 启动线程，开启调度 
	rt_thread_mdelay(50);/* 延时50个tick */		
	rt_thread_startup(&task2_thread); 					// 启动线程，开启调度 
	rt_thread_startup(&task1_thread); 					// 启动线程，开启调度 	
	rt_thread_startup(&task4_thread); 					// 启动线程，开启调度 
//	rt_thread_startup(&task5_thread); 					// 启动线程，开启调度 					
	return 1;
}
uint16_t aa=0;
/**@brief       线程1
* @return       无
*/
static void task1_thread_entry(void* parameter)
{	
	RS485();
	App_ADC_Task();

	rt_thread_mdelay(200);/* 延时200个tick */
			
}

/**@brief       线程2 modbus处理
* @return       无
*/
static void task2_thread_entry(void* parameter)
{
	while(1)
	{
		MB_OS_RxTask();
		rt_thread_mdelay(5);/* 延时200个tick */	
		
	}
}
char devid=0;
u32 FLASH_SIZE;

static char buf12[1024];
/**@brief       线程3 
* @return       无
*/
static void task3_thread_entry(void* parameter)
{	
	static uint8_t t = 0;
	
	char buf1[5]={0};		//缓存写入读取字节数
	static uint32_t sum = 0,sum1 = 0;
	uint8_t zf[3];	//正负号
	ADXL345_IIC_Init();
	Init_ADXL345();
	SPI_Flash_Init();  		//SPI FLASH 初始化
	RTC_Init();		//RTC初始化	，一定要初始化成功
	FLASH_SIZE=16*1024*1024;	//FLASH 大小为16M字节	
	
//	buf1[4]='\n';
//	SPI_Flash_Write((u8*)buf1,0,5);
	
	SPI_Flash_Read((u8*)buf1,0,4);
	sum = buf1[0];
	sum = sum<<8;
	sum |= buf1[1];
	sum = sum<<8;
	sum |= buf1[2];
	sum = sum<<8;
	sum |= buf1[3];
	if(sum == 0)
		sum = 5;
	
	devid=Single_Read_ADXL345(0X00);
	while(1)
	{
		if(calendar.sec != t)
		{
			Multiple_read1_ADXL345();       	//连续读出数据，存储在BUF中
			if(sum <= (FLASH_SIZE-1000))
			{
				if(ADXL345_BUF[1] == 1)
					zf[0] = '+';
				else
					zf[0] = '-';
				if(ADXL345_BUF[3] == 1)
					zf[1] = '+';
				else
					zf[1] = '-';
				if(ADXL345_BUF[5] == 1)
					zf[2] = '+';
				else
					zf[2] = '-';
				sum1 += sprintf(buf12+sum1,"%d%d-%d:%d:%d %d %c%d %c%d %c%d\n",calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec,
				4095,zf[0],ADXL345_BUF[0],zf[1],ADXL345_BUF[2],zf[2],ADXL345_BUF[4]);
				
				if(sum1 >950)
				{
					SPI_Flash_Write((u8*)buf12,sum,sum1);	
					sum += sum1;
					buf1[0] = sum>>24;
					buf1[1] = sum>>16;
					buf1[2] = sum>>8;
					buf1[3] = sum;
					SPI_Flash_Write((u8*)buf1,0,4);
					sum1 = 0;
				}
			}
/*			bu1[0] = shu>>24;
			bu1[1] = shu>>16;
			bu1[2] = shu>>8;
			bu1[3] = shu;
			SPI_Flash_Write((u8*)bu1,0,4);
			SPI_Flash_Read((u8*)bu2,0,4);
			shu1 = bu2[0];
			shu1 = shu1<<8;
			shu1 |= bu2[1];
			shu1 = shu1<<8;
			shu1 |= bu2[2];
			shu1 = shu1<<8;
			shu1 |= bu2[3];*/
			
//			rt_hw_interrupt_disable();	//临界区
//			GPIO_SetBits(GPIOA,GPIO_Pin_8);
//		//	printf("%s",buf12);
//			rt_hw_interrupt_enable(0); //退出临界区 
			
			 t = calendar.sec;
		}
		rt_thread_mdelay(50);/* 延时50个tick */	
	}
}

/**@brief       线程4
* @return       无
*/
static void task4_thread_entry(void* parameter)
{
	static uint8_t i=0;
	
	while(1)
	{
		//IWDG_Feed();//喂狗程序
		rt_thread_mdelay(10);/* 延时10个tick */	
		if((rd485t == 1 )|| (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == 1))
		{
			i++;
		}
		else
			i = 0;
		if(i >= 10)
		{	
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);
			rd485t=0;
		}

	}

}


///**@brief       线程5
//* @return       无
//*/
//static void task5_thread_entry(void* parameter)
//{

//	calendar.sec=0;
//	while(1)
//	{
//		rt_thread_mdelay(100);
//	
//	}
//}

int fputc(int ch,FILE *p)			 //printf输出打印函数
{
	USART_SendData(USART1,(u8)ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	return ch;
}