/**@file        protocol.h
* @brief        (����)���ز���������_��λ��_Э��
* @details      protocol.c��ͷ�ļ�,���������ݰ����ݴ����API����
* @author      	�����
* @date         2021-1-8
* @version      V1.0.0
* @copyright    2021-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/1/8    <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/
#ifndef __PROTOCOI_H
#define __PROTOCOI_H
#include "rtthread.h"
#include "stdint.h"
#include <string.h>
uint8_t PacketHandlingModule(const char *RX);
#endif
