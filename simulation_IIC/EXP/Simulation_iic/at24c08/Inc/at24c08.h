/**@file        at24c08.h
* @details     	at24c08.c��ͷ�ļ�,������at24c08Ӧ�õ�API����
* @author       �����
* @date         2020-08-02
* @version      V1.0.0
* @copyright    2020-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/08/02  <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/

#ifndef __AT24C08_H
#define __AT24C08_H

#include "iic.h"

/**
* @brief at24c08дһ���ֽ�
* @param[in]   addr: Ҫд���ݵĵ�ַ
* @param[in]   data: Ҫд�������
*/
void at24c08_write_byte(uint8_t addr,uint8_t data);

/**
* @brief at24c08��һ���ֽ�
* @param[in]   addr: Ҫ��ȡ���ݵĵ�ַ
* @return		���ض�ȡ��������
*/
uint8_t at24c08_read_byte(uint8_t addr);

//void at24c08_write_page(uint8_t addr,uint8_t *pdata,uint8_t len);
//void at24c08_read_page(uint8_t addr,uint8_t *pdata,uint8_t len);

#endif
