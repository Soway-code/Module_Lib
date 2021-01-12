/**@file        at24c08.h
* @details     	at24c08.c的头文件,声明了at24c08应用的API函数
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

#ifndef __AT24C08_H
#define __AT24C08_H

#include "iic.h"

/**
* @brief at24c08写一个字节
* @param[in]   addr: 要写数据的地址
* @param[in]   data: 要写入的数据
*/
void at24c08_write_byte(uint8_t addr,uint8_t data);

/**
* @brief at24c08读一个字节
* @param[in]   addr: 要读取数据的地址
* @return		返回读取到的数据
*/
uint8_t at24c08_read_byte(uint8_t addr);

//void at24c08_write_page(uint8_t addr,uint8_t *pdata,uint8_t len);
//void at24c08_read_page(uint8_t addr,uint8_t *pdata,uint8_t len);

#endif
