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
#include <string.h>
#include "protocol.h"

/*
******************************************************************
*                               变量
******************************************************************
*/
/* 定义线程控制块 */
static rt_thread_t usart_thread = RT_NULL;
/* 定义信号量控制块 */
rt_sem_t test_sem = RT_NULL;


/* 相关宏定义 */
extern char Usart_Rx_Buf[USART_RBUFF_SIZE];

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void usart_thread_entry(void* parameter);


/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
    

  /* 创建一个信号量 */
	test_sem = rt_sem_create("test_sem",/* 消息队列名字 */
                     0,     /* 信号量初始值，默认有一个信号量 */
                     RT_IPC_FLAG_FIFO); /* 信号量模式 FIFO(0x00)*/
  if (test_sem == RT_NULL)	//信号量创建失败
	return 0;
    
  usart_thread =                          /* 线程控制块指针 */
    rt_thread_create( "usart",              /* 线程名字 */
                      usart_thread_entry,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      2,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (usart_thread != RT_NULL)
        rt_thread_startup(usart_thread);
    else
        return -1;
}

/**@brief      线程定义
* @return       函数执行结果
* @note          \n
*/
static void usart_thread_entry(void* parameter)
{
	rt_err_t uwRet = RT_EOK;
	static uint8_t a=0;

    /* 任务都是一个无限循环，不能返回 */

  while (1)
  {
		uwRet = rt_sem_take(test_sem,	/* 获取串口中断的信号量 */
                        0); 	  /* 等待时间：0 */
    if(RT_EOK == uwRet)
    {
		rt_kprintf("收到数据:%s\n",Usart_Rx_Buf);		//测试观察接收到的数据
		if(strncmp(Usart_Rx_Buf,"HBTQ",4) == 0)			//比较包头是否相同
			PacketHandlingModule(Usart_Rx_Buf);		//数据处理
		memset(Usart_Rx_Buf,0,USART_RBUFF_SIZE);/* 清零 */
    }

  }
}


/********************************END OF FILE****************************/
