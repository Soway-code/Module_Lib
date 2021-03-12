/**@file        iic.h
* @details      iic.c的头文件,定义了配置iic管脚和定时器应用的宏定义,声明了iic应用的API函数
* @author       马灿林
* @date         2020-08-02
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/08/02  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#ifndef __IIC_H
#define __IIC_H
/**
*@brief main.h
*main.h头文件里包含的头文件是 #include "stm32f4xx_hal.h"
*如果不是使用STM32CubeMX软件生成的工程没有 main.h 可以注释
* main.h 添加上对应的 #include "stm32(xx)xx_hal.h"
*/
#include "main.h"	


/**
*@brief HclkFrequency默认设置为64M根据使用频率来修改	12M/24M/32M/48M/64M/72M/168M
*/
#define HclkFrequency 	64	

/**
*@brief IIC总线速度默认设置为100k,根据使用情况来修改为 100或者400
*/
#define IIC_SPEED		100

/**
*@brief IIC的时钟线和数据线可以通过修改以下的宏定义
*SCL_RCC_GPIOX_CLK 			//开启IIC SCL管脚GPIOx时钟
*SDA_RCC_GPIOX_CLK			//开启IIC SDA管脚GPIOx时钟
*IIC_SCL_GPIOX				//选择SCL管脚的GPIOx端口
*IIC_SDA_GPIOX				//选择SDA管脚的GPIOx端口
*SCL_PINX					//选择SCL管脚
*SDA_PINX					//选择SDA管脚
*/
#define SCL_RCC_GPIOX_CLK 		__HAL_RCC_GPIOA_CLK_ENABLE();		//开启GPIOx时钟
#define SDA_RCC_GPIOX_CLK 		__HAL_RCC_GPIOA_CLK_ENABLE();		//开启GPIOx时钟

#define IIC_SCL_GPIOX 		GPIOA								//选择DS18B20 GPIOx端口
#define SCL_PINX 			GPIO_PIN_11							//选择DS18B20 连接的管脚

#define IIC_SDA_GPIOX 		GPIOA
#define SDA_PINX 			GPIO_PIN_12	

/**
在主函数引用 #include "iic.h"既可以使用以下接口函数。
void iic_init(void);				//iic初始化
void iic_start(void);				//起始信号
void iic_stop(void);				//停止信号
uint8_t iic_wait_ack(void);			//等待ACK 1-无效 0-有效
void iic_ack(uint8_t ack);			//产生有效/无效应答 1-无效 0-有效
void iic_send_byte(uint8_t txd);	//发送一个字节
uint8_t iic_read_byte(void);		//接收一个字节
*/

/**@brief   初始化iic
*/
void iic_init(void);

/**
* @brief iic起始信号
*/
void iic_start(void);

/**
* @brief iic停止信号
*/
void iic_stop(void);

/**@brief       等待ACK 
* @return       函数执行结果
* - 返回结果    : 1-ACK无效 0-ACK有效
*/
uint8_t iic_wait_ack(void);

/**@brief       产生有效/无效应答
* @param[in]    ack : 1-无效 0-有效
*/
void iic_ack(uint8_t ack);

/**@brief       发送一个字节
* @param[in]    txd : 要写入的字节
*/
void iic_send_byte(uint8_t txd);

/**@brief       接收一个字节
* @return       函数执行结果
* - 返回结果    : 返回一个字节uint8_t类型的数据
*/
uint8_t iic_read_byte(void);

#endif
