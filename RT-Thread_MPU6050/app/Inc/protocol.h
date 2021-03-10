/**@file        protocol.h
* @brief        (定制)载重测量传感器_定位器_协议
* @details      protocol.c的头文件,声明了数据包数据处理的API函数
* @author      	马灿林
* @date         2021-1-8
* @version      V1.0.0
* @copyright    2021-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/1/8    <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/
#ifndef __PROTOCOI_H
#define __PROTOCOI_H
#include "rtthread.h"
#include "stdint.h"
#include <string.h>
extern uint16_t Null_Ad,Reload_Ad,Overload_Ad;		//车辆空载 AD,车辆重载 AD,车辆超载 AD 报警值;
extern uint8_t D01_sign[2];							//报警标志位-空重载标志位
extern uint8_t Threshold;							//传感器阈值
extern uint8_t DampTime;							//阻尼时间	
extern uint16_t NullCalibrat;						//空载校准
extern uint16_t FullCalibrat;						//满载校准
extern uint16_t ton;				//重量
uint8_t PacketHandlingModule(const char *RX);
void Parameter_Init(void);					//从flash读出参数
#endif
