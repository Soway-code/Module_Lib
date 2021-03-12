/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2012; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                          STM32F0518-EVAL
*                                         Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : MD
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <includes.h>
#include "ADXL345.h"

const u8 SoftWareVersion[]="SV16.10.24.01";
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if REMAP > 0

#define APPLICATION_ADDRESS     (uint32_t)0x08003000

#if   (defined ( __CC_ARM ))
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
__no_init __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )
__IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#elif defined ( __TASKING__ )
__IO uint32_t VectorTable[48] __at(0x20000000);
#endif

#endif

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB  AppTaskStartTCB;
OS_TCB          AppTaskMBTCB;
OS_TCB          AppDITCB;
OS_TCB          AppAnalogTCB;
OS_TCB          AppUartMasterTCB;
OS_TCB          ADXL345;

//#pragma pack(8)

static  CPU_STK         AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static	CPU_STK         AppTaskMBStk[APP_CFG_TASK_MB_STK_SIZE];
//static	CPU_STK         AppDIStk[APP_CFG_TASK_DI_STK_SIZE];
static	CPU_STK         AppANStk[APP_CFG_TASK_AN_STK_SIZE];
static	CPU_STK         ADXL[APP_CFG_TASK_AN_STK_SIZE];
//static	CPU_STK         AppUARTMASTERStk[APP_CFG_TASK_UART3_STK_SIZE];

//#pragma pack()


#define PLUSE_CNT_Q_NUM 8

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void App_StartTask(void *p_arg);
static void App_CreateTask(void);
static void App_CreateObject(void);
extern unsigned char	ADXL345_BUF[12];

#if REMAP > 0
void CopyVectorToRamAndRemap(void)
{
    for(long i = 0; i < 48; i++)
    {
        VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
    }
	
    /* Enable the SYSCFG peripheral clock*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); 
    /* Remap SRAM at 0x00000000 */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
}
#endif

float temp[3];
float x,y,z;
int  dis_data[3]; 
char devid=0;
static void ADXL345_Task(void *p_arg)
{
    OS_ERR Err;
  Init_ADXL345();
  devid=Single_Read_ADXL345(0X00);
    while(1)
    {
        
        Multiple_read1_ADXL345();       	//连续读出数据，存储在BUF中

     OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&Err); //延时500Ms

    }
  

}

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/


int main(void)
{
    OS_ERR      err;
#if (CPU_CFG_NAME_ERR == DEF_ENABLED)
    CPU_ERR     cpu_err;
#endif
    
    BSP_IntDisAll();                                            /* Disable all interrupts.                              */

    #if REMAP > 0
    CopyVectorToRamAndRemap();
    #endif

    SystemInit();
    RCC_MCOConfig(RCC_MCOSource_NoClock, RCC_MCOPrescaler_1);
        
    CPU_Init();

    OSInit(&err);                                               /* Initialize "uC/OS-II, The Real-Time Kernel"          */

    OSTaskCreate((OS_TCB      *)&AppTaskStartTCB,               /* Create the start task                                */
                 (CPU_CHAR    *)"App Start Task",
                 (OS_TASK_PTR  )App_StartTask,
                 (void        *)0,
                 (OS_PRIO      )APP_CFG_TASK_START_PRIO,
                 (CPU_STK     *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE )APP_CFG_TASK_START_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY   )0,
                 (OS_TICK      )0,
                 (void        *)0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-II)   */

    return (1);
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  App_StartTask(void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR  Err;

    (void)p_arg;                                                /* Note #1                                             */

    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    cnts         = cpu_clk_freq                                 /* Determine nbr SysTick increments                     */
                 / (CPU_INT32U)OSCfg_TickRate_Hz;

    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&os_err);                            /* Compute CPU capacity with no task running            */
#endif
    App_CreateObject();
    App_CreateTask();                                            /* Create application tasks                              */

	OSTaskDel((OS_TCB *)0, (OS_ERR *)&Err);
}

void App_CreateObject(void)
{
    
}


/*
*********************************************************************************************************
*                                      App_CreateTask()
*
* Description : Create the application tasks.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  App_CreateTask(void)
{
    OS_ERR  err;

    OSTaskCreate((OS_TCB      *)&AppTaskMBTCB,              /* Task Modbus for custom */
                 (CPU_CHAR    *)"App Task Modbus",
                 (OS_TASK_PTR  ) APP_MB_Task,
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_MB_PRIO,
                 (CPU_STK     *) &AppTaskMBStk[0],
                 (CPU_STK_SIZE ) APP_CFG_TASK_MB_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE ) APP_CFG_TASK_MB_STK_SIZE,
                 (OS_MSG_QTY   ) 0,
                 (OS_TICK      ) 0,
                 (void        *) 0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&err);
/*
	OSTaskCreate((OS_TCB      *)&AppDITCB,                 // Task Read DI
                 (CPU_CHAR    *)"App DI Task",
                 (OS_TASK_PTR  ) App_DI_Task,
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_DI_PRIO,
                 (CPU_STK     *) &AppDIStk[0],
                 (CPU_STK_SIZE ) APP_CFG_TASK_DI_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE ) APP_CFG_TASK_DI_STK_SIZE,
                 (OS_MSG_QTY   ) 0,
                 (OS_TICK      ) 0,
                 (void        *) 0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&err);
*/
	OSTaskCreate((OS_TCB      *)&AppAnalogTCB,              /*Task Read Analog Data*/
                 (CPU_CHAR    *)"App Analog Task",
                 (OS_TASK_PTR  ) App_Analog_Task,
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_AN_PRIO,
                 (CPU_STK     *) &AppANStk[0],
                 (CPU_STK_SIZE ) APP_CFG_TASK_AN_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE ) APP_CFG_TASK_AN_STK_SIZE,
                 (OS_MSG_QTY   ) 0,
                 (OS_TICK      ) 0,
                 (void        *) 0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&err);
        
        OSTaskCreate((OS_TCB      *)&ADXL345,            
                 (CPU_CHAR    *)"ADXL345_Task",
                 (OS_TASK_PTR  ) ADXL345_Task,
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_AN_PRIO,
                 (CPU_STK     *) &ADXL[0],
                 (CPU_STK_SIZE ) APP_CFG_TASK_AN_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE ) APP_CFG_TASK_AN_STK_SIZE,
                 (OS_MSG_QTY   ) 0,
                 (OS_TICK      ) 0,
                 (void        *) 0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&err);
/*
    OSTaskCreate((OS_TCB      *)&AppUartMasterTCB,          //Task Read Uart Data
                 (CPU_CHAR    *)"App UartMaster",
                 (OS_TASK_PTR  ) App_Uart_Task,
                 (void        *) 0,
                 (OS_PRIO      ) APP_CFG_TASK_UART_MASTER_PRIO,
                 (CPU_STK     *) &AppUARTMASTERStk[0],
                 (CPU_STK_SIZE ) APP_CFG_TASK_UART3_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE ) APP_CFG_TASK_UART3_STK_SIZE,
                 (OS_MSG_QTY   ) 0,
                 (OS_TICK      ) 0,
                 (void        *) 0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&err);
                 */
}

