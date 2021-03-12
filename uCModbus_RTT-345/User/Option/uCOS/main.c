/*****************************************************************************
* @file    Project/Template/main.c 
* @author  ktfe
* @version STDV3.5
* @date    11/01/2012
* @brief   Main program body
*****************************************************************************/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
	
#include "main.h"
	
//主函数	
int main(void)
{
  /* Infinite loop */
	OSInit();		/*uCOS初始化*/ 
	OSTaskCreate(TaskStart, (void *)0, 
								&TaskStart_stk[80-1], 1);
	OSStart();	/*启动uCOS*/	
	return 0; 	/*处理器不会运行到此步*/
}
/*启动任务*/
void TaskStart(void *pdata)
{	
	/* Initialize the uCOS_Tick. */
	systick_init();
	
	///////////////////////////////
	/*创建其他任务*/
	OSTaskCreate(task1, (void *)0, 
								&task1_stk[STK_SIZE-1], 5);
	OSTaskCreate(task2, (void *)0, 
								&task2_stk[STK_SIZE-1], 6);
	///////////////////////////////
	
	OSTaskDel(OS_PRIO_SELF);	//删除自身
}

/*其他任务*/
void task1(void *pdata)
{
	for(;;)
	{
		OSTimeDly(200);

		OS_ENTER_CRITICAL();

		OS_EXIT_CRITICAL();
	}
}
void task2(void *pdata)
{
	for(;;)
	{
		OSTimeDly(200);

		OS_ENTER_CRITICAL();

		OS_EXIT_CRITICAL();
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
