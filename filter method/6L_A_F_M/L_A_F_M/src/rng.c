/**
 * @file         rng.c
 * @brief        ��������޷�ƽ���˲���
 * @author       Τ����
 * @date         2020-07-03
 * @version      V1.0.0
 * @copyright    2020-2030,��������Ϊ�Ƽ���չ���޹�˾
 **********************************************************************************
 * @par �޸���־:
 * <table>
 * <tr><th>Date        <th>Version  <th>Author    <th>Description
 * <tr><td>2020/07/03  <td>1.0.0    <td>Τ����    <td>������ʼ�汾
 * </table>
 *
 **********************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rng.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RNG_HandleTypeDef hrng;


#define   FILTER_A   1				//���Ƶķ���

int filter_buf[FILTER_N] = {0};		//���������

int i ;


/* RNG init function */
void MX_RNG_Init(void)
{

	hrng.Instance = RNG;
	hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
	if (HAL_RNG_Init(&hrng) != HAL_OK)
	{
	Error_Handler();
	}
	
	filter_buf[FILTER_N - 2] = 300;
	
}

void HAL_RNG_MspInit(RNG_HandleTypeDef* rngHandle)
{

  if(rngHandle->Instance==RNG)
  {
  /* USER CODE BEGIN RNG_MspInit 0 */

  /* USER CODE END RNG_MspInit 0 */
    /* RNG clock enable */
    __HAL_RCC_RNG_CLK_ENABLE();
  /* USER CODE BEGIN RNG_MspInit 1 */

  /* USER CODE END RNG_MspInit 1 */
  }
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef* rngHandle)
{

  if(rngHandle->Instance==RNG)
  {
  /* USER CODE BEGIN RNG_MspDeInit 0 */

  /* USER CODE END RNG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();
  /* USER CODE BEGIN RNG_MspDeInit 1 */

  /* USER CODE END RNG_MspDeInit 1 */
  }
} 

/**
* @brief       �õ������
* @param    	None
* @return       ��ȡ���������
*/
uint32_t RNG_Get_RandomNum(void)
{
	uint32_t randomnum;
	
    HAL_RNG_GenerateRandomNumber(&hrng,&randomnum);
	return randomnum;
}

/**
* @brief       ����[min,max]��Χ�������
* @param        min: ��Сֵ    max: ���ֵ
* @return       �����
*/
int RNG_Get_RandomRange(int min,int max)
{ 
	uint32_t randomnum;
	
	HAL_RNG_GenerateRandomNumber(&hrng,&randomnum);
	return randomnum%(max-min+1)+min;
}


/**
* @brief   �޷�ƽ���˲���
* @brief   ���趨�ķ����ڣ���ƽ��ֵ
* @retval  �޷�ƽ��ֵ
*/
int Filter(void) 
{
	int filter_sum = 0;
	
	filter_buf[FILTER_N - 1] = RNG_Get_RandomRange(295, 305);

	if((filter_buf[FILTER_N - 1] - filter_buf[FILTER_N - 2] > FILTER_A) || (filter_buf[FILTER_N - 2] - filter_buf[FILTER_N - 1] > FILTER_A))
	{
		filter_buf[FILTER_N - 1] = filter_buf[FILTER_N - 2];
		
	}
	
	for(i = 0; i < FILTER_N - 1; i++) 
	{
		filter_buf[i] = filter_buf[i + 1];
		filter_sum += filter_buf[i];
	}
	
    
	return (int)(filter_sum / (FILTER_N - 1));

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
