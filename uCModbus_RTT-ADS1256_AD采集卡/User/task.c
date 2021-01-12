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

#include "task.h"
#include "stdio.h"
#include "ADS1256.h"
#include "stm32f10x_gpio.h"
#include "flash.h"
#include "DataProcessing.h"

uint32_t ldVolutage,ldVolutag[8];
long ulResult;
extern volatile uint16_t MB_DATA_04[610];
uint32_t buf1;
uint16_t redeeming[9];
uint32_t v_buf[8];	//8个通道的滤波后的电压标定时使用
uint32_t CalibrationValue[88];	//标定的参数

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


//配置参数使用的管脚初始化
void ConfigurationGpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); 
//GPIO_Mode_IPU 上拉输入	19 B1  17 A7  15 A5  13  A3
	
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_7; 	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_7|GPIO_Pin_5|GPIO_Pin_3; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);

//  32 A11    41 B5       43  B7
//  18  B0    16  A6   14 A4      12  A2
// GPIO_Mode_Out_PP 推挽输出 低电平
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_4|GPIO_Pin_2; 
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_0; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_5|GPIO_Pin_0); 
	GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_4|GPIO_Pin_2); 
}
void ReadsTheStatus(uint32_t *baud,uint8_t *node_addr)
{
	uint8_t buf[4]={0};
//处理波特率
	buf[0]= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11);
	buf[1]= GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7);
	if((buf[0]==0)&&(buf[1]==1))
	{
		*baud = 9600;
	}
	else if((buf[0]==1)&&(buf[1]==0))
	{
		*baud = 115200;
	}
	else
		*baud = 9600;
	
//处理站点号
	buf[3]= GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
	buf[2]= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7);	
	buf[1]= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
	buf[0]= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3);
	buf[0]= buf[0]<< 1;
	buf[0]|= buf[1];
	buf[0]= buf[0]<< 1;
	buf[0]|= buf[2];
	buf[0]= buf[0]<< 1;
	buf[0]|= buf[3];
	*node_addr = buf[0]+100;
}

/**@brief       任务的初始化
* @return       函数执行结果1
*/
uint8_t task_init(void) 
{
	Target_Init();	//系统配置
	MB_Init(1000);
	ConfigurationGpioInit();
	Init_ADS1256_GPIO(); //初始化ADS1256 GPIO管脚 
	GPIO_SetBits(GPIOB, GPIO_Pin_11 );  
	ADS1256_Init();
	
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

/**@brief       线程1读取数据
* @return       无
*/
static void task1_thread_entry(void* parameter)
{
	uint8_t i=0,ii=0;
	for(i = 0;i<8;)
	{
		redeeming[i] = FLASH_R(ADD_I0+(i*2));
		i++;
	}
	redeeming[8] = FLASH_R(SIGN);
	if(redeeming[8] != 0x55AA)
	{
		FLASH_W(SIGN,0x55AA);
	}
	while(1)
	{
		for(i=0;i < 8;i++)
		{
			ulResult = ADS_sum( (i << 4) | ADS1256_MUXN_AINCOM);	
			if( ulResult & 0x800000 )
			{
				ulResult = ~(unsigned long)ulResult;
				ulResult &= 0x7fffff;
				ulResult += 1;
				ulResult = -ulResult;
			}
			ldVolutage = (double)ulResult*0.59604644775390625;
			ii=((i & 0x07)?(i & 0x07) - 1:7);
			ldVolutag[ii] = ldVolutage;		
			
		}
		//通道数据放入寄存器			
		for(i=0;i < 8;)		
		{
			if(redeeming[i] > 0x7D00)
				ldVolutag[i] = (ldVolutag[i] + ((redeeming[i] - 0X7D00)*10));		//补偿+
			else if(redeeming[i] < 0x7D00)
				ldVolutag[i] = (ldVolutag[i] - ((0x7D00 - redeeming[i])*10));		//补偿-
//			MB_DATA_04[100+(i*10)] = ((uint32_t)ldVolutag[i]>>16);
//			MB_DATA_04[101+(i*10)] = (uint16_t)((uint32_t)ldVolutag[i]&0x0000FFFF);
//			buf1 =(ldVolutag[i] -((uint32_t)ldVolutag[i]))*1000000;
//			MB_DATA_04[102+(i*10)] = ((uint32_t)buf1>>16);
//			MB_DATA_04[103+(i*10)] = (uint16_t)((uint32_t)buf1&0x0000FFFF);
			i++;
		}
		rt_thread_mdelay(100);/* 延时300个tick */		
	}
}

/**@brief       线程2 modbus处理
* @return       无
*/
static void task2_thread_entry(void* parameter)
{	
	uint8_t i = 0;
	for(;i<=10;i++)
	{
		CalibrationValue[i] = Flash_Read_fourByte(CH0_0 +(i*0x04));
		CalibrationValue[i+11] = Flash_Read_fourByte(CH1_0 +(i*0x04));
		CalibrationValue[i+22] = Flash_Read_fourByte(CH2_0 +(i*0x04));
		CalibrationValue[i+33] = Flash_Read_fourByte(CH3_0 +(i*0x04));
		CalibrationValue[i+44] = Flash_Read_fourByte(CH4_0 +(i*0x04));
		CalibrationValue[i+55] = Flash_Read_fourByte(CH5_0 +(i*0x04));
		CalibrationValue[i+66] = Flash_Read_fourByte(CH6_0 +(i*0x04));
		CalibrationValue[i+77] = Flash_Read_fourByte(CH7_0 +(i*0x04));
		CalibrationValue[0] = 0;
		CalibrationValue[11] = 0;
		CalibrationValue[22] = 0;
		CalibrationValue[33] = 0;
		CalibrationValue[44] = 0;
		CalibrationValue[55] = 0;
		CalibrationValue[66] = 0;
		CalibrationValue[77] = 0;

	}
	
	parameter_init(CalibrationValue);
	while(1)
	{
		MB_OS_RxTask();
		rt_thread_mdelay(5);/* 延时5个tick */	
	}
}


/**@brief       线程3 地址，波特率修改判断
* @return       无
*/
static void task3_thread_entry(void* parameter)
{	
	uint8_t i=0;
	uint32_t v_buf1[8];
	uint32_t baud=0,baud1=0;
	uint8_t node_addr = 0,node_addr1 = 0;
	ReadsTheStatus(&baud,&node_addr);
	baud1 = baud;
	node_addr1 = node_addr;
	MB_CfgCh(node_addr,MODBUS_SLAVE,0,MODBUS_MODE_RTU,1,baud,8,MODBUS_PARITY_NONE,1,MODBUS_WR_EN);
	
	while(1)
	{
		ReadsTheStatus(&baud,&node_addr);
		if((baud != baud1) || (node_addr != node_addr1))
		{
			baud1 = baud;
			node_addr1 = node_addr;
			Target_Init();	//系统配置
			MB_Init(1000);
			MB_CfgCh(node_addr,MODBUS_SLAVE,0,MODBUS_MODE_RTU,1,baud,8,MODBUS_PARITY_NONE,1,MODBUS_WR_EN);
		}
		
		v_buf1[0] = Channel_0(ldVolutag[0],&v_buf[0]);
		v_buf1[1] = Channel_1(ldVolutag[1],&v_buf[1]);
		v_buf1[2] = Channel_2(ldVolutag[2],&v_buf[2]);
		v_buf1[3] = Channel_3(ldVolutag[3],&v_buf[3]);
		v_buf1[4] = Channel_4(ldVolutag[4],&v_buf[4]);
		v_buf1[5] = Channel_5(ldVolutag[5],&v_buf[5]);
		v_buf1[6] = Channel_6(ldVolutag[6],&v_buf[6]);
		v_buf1[7] = Channel_7(ldVolutag[7],&v_buf[7]);
		for(i=0;i < 8;)		
		{
			MB_DATA_04[100+(i*10)] = ((uint32_t)v_buf1[i]>>16);
			MB_DATA_04[101+(i*10)] = (uint16_t)v_buf1[i];
			MB_DATA_04[102+(i*10)] = 0xFFFF;
			MB_DATA_04[103+(i*10)] = 0xFFFF;
			i++;
		}
		rt_thread_mdelay(60);/* 延时1000个tick */	
	}
}



