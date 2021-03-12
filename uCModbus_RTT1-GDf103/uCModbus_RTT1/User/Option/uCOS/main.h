/*****************************************************************************
* @file    Project/Template/app_cfg.h 
* @author  ktfe
* @version V1.0.0
* @date    16/03/2013
* @brief   APP_Config
*****************************************************************************/

#ifndef _MAIN_H
#define _MAIN_H

#include "ucos_ii.h"
#include "stm32f10x_sys.h"

//���ٶ�ջ
OS_CPU_SR cpu_sr=0;
#define 	STK_SIZE	80
static OS_STK TaskStart_stk[STK_SIZE];
static OS_STK task1_stk[STK_SIZE];
static OS_STK task2_stk[STK_SIZE];

/*��������*/
void TaskStart(void *pdata);	//��������
void task1(void *pdata);			//����1
void task2(void *pdata);			//����1

/*��������*/


#endif
