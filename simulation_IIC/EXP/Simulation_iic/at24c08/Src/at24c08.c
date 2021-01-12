/**@file        at24c08.c
* @brief        at24c08模块的应用
* @details      at24c08读写函数
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

#include "iic.h"
#include "at24c08.h"
#include "stdio.h"	

/**
* @brief at24c08写一个字节
* @param[in]   addr: 要写数据的地址
* @param[in]   data: 要写入的数据
*/
void at24c08_write_byte(uint8_t addr,uint8_t data)
{
	//起始信号
	iic_start();
	
	//发送从设备地址和写信号 0x50<<1 | 0 = 0xa0
	iic_send_byte(0xa0);
	//等待ACK
	if(iic_wait_ack()){
		return;
	}
	
	//发送写入的字节地址
	iic_send_byte(addr);
	//等待ACK
	if(iic_wait_ack()){
		return;
	}
	
	//发送写入的数据
	iic_send_byte(data);
	//等待ACK
	if(iic_wait_ack()){
		return;
	}
	
	//停止信号
	iic_stop();
}

/**
* @brief at24c08读一个字节
* @param[in]   addr: 要读取数据的地址
* @return		返回读取到的数据
*/
uint8_t at24c08_read_byte(uint8_t addr)
{
	uint8_t data;
	
	//起始信号
	iic_start();
	
	//发送从设备地址和写信号 0x50<<1 | 0 = 0xa0
	iic_send_byte(0xa0);
	//等待ACK
	if(iic_wait_ack()){
		return 0;
	}
	
	//发送读的字节地址
	iic_send_byte(addr);
	//等待ACK
	if(iic_wait_ack()){
		return 0;
	}
	
	//起始信号
	iic_start();
	
	//发送从设备地址和读信号 0x50<<1 | 1 = 0xa1
	iic_send_byte(0xa1);
	//等待ACK
	if(iic_wait_ack()){
		return 0;
	}
	
	//读取数据
	data = iic_read_byte();
	//无效应答 结束读取
	iic_ack(1);
	
	//停止信号
	iic_stop();
	
	return data;
}

////写一页
//void at24c08_write_page(uint8_t addr,uint8_t *pdata,uint8_t len)
//{
//	
//}

////读一页
//void at24c08_read_page(uint8_t addr,uint8_t *pdata,uint8_t len)
//{
//	
//}
