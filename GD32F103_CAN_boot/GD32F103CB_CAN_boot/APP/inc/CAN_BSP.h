/**@file        CAN_APP.h
* @details      CAN_APP.c��ͷ�ļ�,������CAN_APPӦ�õ�API����
* @details 		������GD32F103CB��Ƭ��,CAN����
* @author       �����
* @date         2021-4-09
* @version      V1.0.0
* @copyright    2021-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/4/09  <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/


#ifndef __CAN_BSP_H__
#define __CAN_BSP_H__
#include "gd32f10x.h"

/**@brief       CAN0��ʼ��
*/
void Can0_init(uint8_t baudrate);

/**@brief       CAN����һ������(��չ֡,����֡) 
* @param[in]    ID : ���͵�CANID;
* @param[in]    msg : ����ָ��;
* @param[in]    len : ���ݳ���(���Ϊ8)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_Msg(uint32_t ID,uint8_t* msg,uint8_t len);

uint8_t CAN_Send_Msg1(uint32_t ID,uint8_t* msg,uint8_t len);
/**@brief       CAN���Ͷ�������(��չ֡,����֡) 
* @param[in]    ID : ���͵�CANID;
* @param[in]    msg : ����ָ��;
* @param[in]    len : ���ݳ���(���Ϊ65535)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_MultiBytes(uint32_t ID,uint8_t *msg,uint16_t len);

/**@brief       CAN����2�ֽ�����(��չ֡,����֡) 
* @param[in]    ID : ���͵�CANID;
* @param[in]    msg : ����ָ��;
* @param[in]    len : ���ݳ���(���Ϊ255)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_twoByte(uint32_t ID,uint16_t *msg,uint8_t len);

/**@brief       CAN����4�ֽ�����(��չ֡,����֡) 
* @param[in]    ID : ���͵�CANID;
* @param[in]    msg : ����ָ��;
* @param[in]    len : ���ݳ���(���Ϊ50)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_fourByte(uint32_t ID,uint32_t *msg,uint8_t len);

#endif
