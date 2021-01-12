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
#include <string.h>
#include "protocol.h"

/*
******************************************************************
*                               ����
******************************************************************
*/
/* �����߳̿��ƿ� */
static rt_thread_t usart_thread = RT_NULL;
/* �����ź������ƿ� */
rt_sem_t test_sem = RT_NULL;


/* ��غ궨�� */
extern char Usart_Rx_Buf[USART_RBUFF_SIZE];

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void usart_thread_entry(void* parameter);


/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{	
    

  /* ����һ���ź��� */
	test_sem = rt_sem_create("test_sem",/* ��Ϣ�������� */
                     0,     /* �ź�����ʼֵ��Ĭ����һ���ź��� */
                     RT_IPC_FLAG_FIFO); /* �ź���ģʽ FIFO(0x00)*/
  if (test_sem == RT_NULL)	//�ź�������ʧ��
	return 0;
    
  usart_thread =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "usart",              /* �߳����� */
                      usart_thread_entry,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      2,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (usart_thread != RT_NULL)
        rt_thread_startup(usart_thread);
    else
        return -1;
}

/**@brief      �̶߳���
* @return       ����ִ�н��
* @note          \n
*/
static void usart_thread_entry(void* parameter)
{
	rt_err_t uwRet = RT_EOK;
	static uint8_t a=0;

    /* ������һ������ѭ�������ܷ��� */

  while (1)
  {
		uwRet = rt_sem_take(test_sem,	/* ��ȡ�����жϵ��ź��� */
                        0); 	  /* �ȴ�ʱ�䣺0 */
    if(RT_EOK == uwRet)
    {
		rt_kprintf("�յ�����:%s\n",Usart_Rx_Buf);		//���Թ۲���յ�������
		if(strncmp(Usart_Rx_Buf,"HBTQ",4) == 0)			//�Ƚϰ�ͷ�Ƿ���ͬ
			PacketHandlingModule(Usart_Rx_Buf);		//���ݴ���
		memset(Usart_Rx_Buf,0,USART_RBUFF_SIZE);/* ���� */
    }

  }
}


/********************************END OF FILE****************************/
