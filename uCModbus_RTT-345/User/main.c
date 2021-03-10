#include "main.h"	
#include "stm32f10x_gpio.h"
#include "task.h"

int main(void)
{
  /* Infinite loop */
	task_init();	//�̳߳�ʼ��
}

/*������ʼ��*/
void Target_Init(void)
{
	RCC_ClocksTypeDef Rcc_clocks;						//ϵͳʱ�ӱ���
	RCC_GetClocksFreq(&Rcc_clocks); 				//��ȡʱ������״̬
	SysTick_Config(Rcc_clocks.HCLK_Frequency/TICKS_PER_SEC);	//����ϵͳʱ��,��ʹ���ж�						
}
///*ϵͳʱ���жϷ���*/
///*ʱ����TICKS_PER_SEC��������*/
//void SysTick_Handler(void)
//{
//	static uint32_t index=0;
//	index++;
//}

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
