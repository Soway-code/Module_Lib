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


double ldVolutage,ldVolutag[8];
long ulResult;
extern volatile uint16_t MB_DATA_04[610];
uint32_t buf1,rd485t = 1;
uint16_t redeeming[9];


static struct rt_thread task1_thread;			 	//定义线程控制块 
static rt_uint8_t rt_task1_thread_stack[1024];		//定义线程栈

static struct rt_thread task2_thread;				//定义线程控制块 
static rt_uint8_t rt_task2_thread_stack[1024];		//定义线程栈

static struct rt_thread task3_thread;				//定义线程控制块 
static rt_uint8_t rt_task3_thread_stack[1024];		//定义线程栈

//函数声明
static void task1_thread_entry(void* parameter);
static void task2_thread_entry(void* parameter);
static void task3_thread_entry(void* parameter);

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
	IWDG_Init();
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
	rt_thread_startup(&task3_thread); 					// 启动线程，开启调度 
	rt_thread_mdelay(50);/* 延时50个tick */		
	rt_thread_startup(&task2_thread); 					// 启动线程，开启调度 
	rt_thread_startup(&task1_thread); 					// 启动线程，开启调度 			
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

//		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//		while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));//转换结束标志位
//		aa=ADC_GetConversionValue(ADC1);//返回最近一次ADCx规则组的转换结果
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



/**@brief       线程3 
* @return       无
*/
static void task3_thread_entry(void* parameter)
{	
	static uint8_t i=0;
	while(1)
	{
		IWDG_Feed();//喂狗程序
		rt_thread_mdelay(10);/* 延时1000个tick */	
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
	
//		rt_thread_mdelay(1000);/* 延时1000个tick */	
//		GPIO_SetBits(GPIOA,GPIO_Pin_8);
	}
}



