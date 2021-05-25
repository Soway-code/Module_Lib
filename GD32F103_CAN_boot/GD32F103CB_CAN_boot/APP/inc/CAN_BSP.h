/**@file        CAN_APP.h
* @details      CAN_APP.c的头文件,声明了CAN_APP应用的API函数
* @details 		适用于GD32F103CB单片机,CAN函数
* @author       马灿林
* @date         2021-4-09
* @version      V1.0.0
* @copyright    2021-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/4/09  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/


#ifndef __CAN_BSP_H__
#define __CAN_BSP_H__
#include "gd32f10x.h"

/**@brief       CAN0初始化
*/
void Can0_init(uint8_t baudrate);

/**@brief       CAN发送一组数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为8)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_Msg(uint32_t ID,uint8_t* msg,uint8_t len);

uint8_t CAN_Send_Msg1(uint32_t ID,uint8_t* msg,uint8_t len);
/**@brief       CAN发送多组数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为65535)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_MultiBytes(uint32_t ID,uint8_t *msg,uint16_t len);

/**@brief       CAN发送2字节数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为255)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_twoByte(uint32_t ID,uint16_t *msg,uint8_t len);

/**@brief       CAN发送4字节数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为50)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_fourByte(uint32_t ID,uint32_t *msg,uint8_t len);

#endif
