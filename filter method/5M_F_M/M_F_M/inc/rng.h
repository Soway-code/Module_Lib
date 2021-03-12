/**
 * @file         rng.h
 * @brief        �����rng��ͷ�ļ�
 * @details      rng.c�ļ�����������
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __rng_H
#define __rng_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RNG_HandleTypeDef hrng;

extern int Value;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_RNG_Init(void);


/**
* @brief       �õ������
* @param    	None
* @return       ��ȡ���������
*/
uint32_t RNG_Get_RandomNum(void);

/**
* @brief       ����[min,max]��Χ�������
* @param        min: ��Сֵ    max: ���ֵ
* @return       �����
*/
int RNG_Get_RandomRange(int min,int max);

/**
* @brief   ��λֵƽ���˲������ֳƷ��������ƽ���˲�����
* @retval  ��ȡż��λ���ݣ�ȥ�������Сֵ���ƽ��ֵ
*/
int Filter(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ rng_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
