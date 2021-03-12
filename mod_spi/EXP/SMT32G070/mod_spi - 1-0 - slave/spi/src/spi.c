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


/**
* @brief		微妙延时函数
* @para			None
* @retval		None
*/
void delay_us(uint32_t tim)
{
	tim = tim*10;
	while(tim--);	
}

/**
* @brief		时钟前沿采样，时钟后沿输出
* @para			data,输出的数据
* @retval		None

*/
void spi_ReadWrite_data(uint8_t data)
{
	// CPOL = 1 CPHA = 0 SCLK低电平有效 第偶边沿采样
	uint8_t i=0;

	GPIO_PinState CS_pin;
	//SCLK=0;

	CS_pin = NSS;
	if(CS_pin == GPIO_PIN_RESET)
	{
		for(i=0; i<8; i++)
		{
			if(data & (1<<(7-i)))
			{
				// MOSI=1;
				MISO_H;
			}
			else
			{
				//MOSI=0;
				MISO_L;
			}
			
			//下降沿采样
			//SCLK=1;
			while(SCK);
			while(!SCK);		


		}	
		
	}

	while(SCK);
	
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
	
	if(!NSS)
	{
		for(i=0; i<8; i++)
		{
			if(MOSI)
			{
				rx_data|=1<<(7-i);
			}
			
			while(SCK);
			while(!(SCK));
				
		}
	}
	return rx_data;

}




