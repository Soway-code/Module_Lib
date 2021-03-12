/**
 * @file         rng.h
 * @brief        随机数rng的头文件
 * @details      rng.c文件函数的声明
 * @author       韦剑祥
 * @date         2020-07-03
 * @version      V1.0.0
 * @copyright    2020-2030,深圳市信为科技发展有限公司
 **********************************************************************************
 * @par 修改日志:
 * <table>
 * <tr><th>Date        <th>Version  <th>Author    <th>Description
 * <tr><td>2020/07/03  <td>1.0.0    <td>韦剑祥    <td>创建初始版本
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
* @brief       得到随机数
* @param    	None
* @return       获取到的随机数
*/
uint32_t RNG_Get_RandomNum(void);

/**
* @brief       生成[min,max]范围的随机数
* @param        min: 最小值    max: 最大值
* @return       随机数
*/
int RNG_Get_RandomRange(int min,int max);

/**
* @brief     算术平均滤波法
* @retval    数据的平均值
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
