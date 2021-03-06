/**
* @file         spi.h
* @brief        模拟SPI的头文件
* @details      模拟SPI主机引脚的宏定义、spi.c文件函数的声明
* @author       韦剑祥
* @date         2020-08-03
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/05/06  <td>1.0.0    <td>韦剑祥    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#ifndef __SPI_H
#define __SPI_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

//SPI主机时钟引脚宏定义
#define 	SCK_L		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define 	SCK_H		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)
//SPI主机输入引脚宏定义
#define 	MISO		HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5)
//SPI主机输出引脚宏定义
#define 	MOSI_L		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define 	MOSI_H		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
//SPI主机片选引脚宏定义
#define 	NSS_L		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET)
#define 	NSS_H		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET)

/**
* @brief		微妙延时函数
* @para			None
* @retval		None
*/
void delay_us(uint32_t nus);
/**
* @brief		时钟前沿采样，时钟后沿输出
* @para			data,输出的数据
* @retval		None

*/
void spi_ReadWrite_data(uint8_t data);
/**
* @brief		时钟前沿采样，时钟后沿输出
* @para			None
* @retval		rx_data,接收到的数据

*/
uint8_t spi_Read_Data(void);

#endif

