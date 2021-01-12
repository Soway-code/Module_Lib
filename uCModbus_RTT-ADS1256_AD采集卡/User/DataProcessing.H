/**@file        DataProcessing.h
* @details      DataProcessing.c的头文件,声明了电压多段标定应用的API函数
				适用于ADS1256采集的数据处理;
* @author       马灿林
* @date         2020-12-22
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/12/22  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/
#ifndef __DATSPROCESSING_H
#define __DATSPROCESSING_H
#include "stdio.h"
#include "stdint.h"

uint8_t parameter_init(uint32_t *CalibrationValue);
uint32_t Channel_0(uint32_t V_IN,uint32_t *V_OUT);
uint32_t Channel_1(uint32_t V_IN,uint32_t *V_OUT);
uint32_t Channel_2(uint32_t V_IN,uint32_t *V_OUT);
uint32_t Channel_3(uint32_t V_IN,uint32_t *V_OUT);
uint32_t Channel_4(uint32_t V_IN,uint32_t *V_OUT);
uint32_t Channel_5(uint32_t V_IN,uint32_t *V_OUT);
uint32_t Channel_6(uint32_t V_IN,uint32_t *V_OUT);
uint32_t Channel_7(uint32_t V_IN,uint32_t *V_OUT);
#endif 
