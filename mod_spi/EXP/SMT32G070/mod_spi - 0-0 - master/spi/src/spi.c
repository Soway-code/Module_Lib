/**
* @file         spi.c
* @brief        模拟SPI的应用
* @details      模拟SPI主机在偶边沿的下降沿发送数据
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

#include "spi.h"
#include "tim.h"

/**
* @brief		微妙延时函数
* @para			None
* @retval		None
*/
//void delay_us(uint32_t tim)
//{
//	tim = tim*6;
//	while(tim--);	
//}

/**
* @brief		时钟前沿采样，时钟后沿输出
* @para			data,输出的数据
* @retval		None
*/
void spi_ReadWrite_data(uint8_t data)
{
	// CPOL = 0 CPHA = 0 SCLK低电平有效 第偶边沿采样
	uint8_t i=0;

	//SCLK=0;

	NSS_L;
	for(i=0; i<8; i++)
	{
		//数据发送从高位到低位
		if(data & (1<<(7-i)))
		{
			// MOSI=1;
			MOSI_H;
		}
		else
		{
			//MOSI=0;
			MOSI_L;
		}
		
		//上升沿采样
		//SCLK=1;

		SCK_L;
		delay_tim_5us();		
	
		SCK_H;
		delay_tim_5us();
	
		//下降沿改变数据
		//SCLK=0;
	}	
	//根据数据的最高位决定数据波形进出的高低
	if(data & (1<<(7-0)))
	{
		// MOSI=1;
		MOSI_H;
	}
	else
	{
		//MOSI=0;
		MOSI_L;
	}
	SCK_L;
//	delay_us(5);
	NSS_H;
	return;

}

/**
* @brief		时钟前沿采样，时钟后沿输出
* @para			None
* @retval		rx_data,接收到的数据
*/
uint8_t spi_Read_Data(void)
{
	uint8_t i;
	uint8_t rx_data = 0;
	
	NSS_L;
	for(i=0; i<8; i++)
	{
		if(MISO)
		{
			rx_data|=1<<(7-i);
		}
		
	}
	NSS_H;

	return rx_data;

}




