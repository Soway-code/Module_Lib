/**@file        at24c08.c
* @brief        at24c08ģ���Ӧ��
* @details      at24c08��д����
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

#include "iic.h"
#include "at24c08.h"
#include "stdio.h"	

/**
* @brief at24c08дһ���ֽ�
* @param[in]   addr: Ҫд���ݵĵ�ַ
* @param[in]   data: Ҫд�������
*/
void at24c08_write_byte(uint8_t addr,uint8_t data)
{
	//��ʼ�ź�
	iic_start();
	
	//���ʹ��豸��ַ��д�ź� 0x50<<1 | 0 = 0xa0
	iic_send_byte(0xa0);
	//�ȴ�ACK
	if(iic_wait_ack()){
		return;
	}
	
	//����д����ֽڵ�ַ
	iic_send_byte(addr);
	//�ȴ�ACK
	if(iic_wait_ack()){
		return;
	}
	
	//����д�������
	iic_send_byte(data);
	//�ȴ�ACK
	if(iic_wait_ack()){
		return;
	}
	
	//ֹͣ�ź�
	iic_stop();
}

/**
* @brief at24c08��һ���ֽ�
* @param[in]   addr: Ҫ��ȡ���ݵĵ�ַ
* @return		���ض�ȡ��������
*/
uint8_t at24c08_read_byte(uint8_t addr)
{
	uint8_t data;
	
	//��ʼ�ź�
	iic_start();
	
	//���ʹ��豸��ַ��д�ź� 0x50<<1 | 0 = 0xa0
	iic_send_byte(0xa0);
	//�ȴ�ACK
	if(iic_wait_ack()){
		return 0;
	}
	
	//���Ͷ����ֽڵ�ַ
	iic_send_byte(addr);
	//�ȴ�ACK
	if(iic_wait_ack()){
		return 0;
	}
	
	//��ʼ�ź�
	iic_start();
	
	//���ʹ��豸��ַ�Ͷ��ź� 0x50<<1 | 1 = 0xa1
	iic_send_byte(0xa1);
	//�ȴ�ACK
	if(iic_wait_ack()){
		return 0;
	}
	
	//��ȡ����
	data = iic_read_byte();
	//��ЧӦ�� ������ȡ
	iic_ack(1);
	
	//ֹͣ�ź�
	iic_stop();
	
	return data;
}

////дһҳ
//void at24c08_write_page(uint8_t addr,uint8_t *pdata,uint8_t len)
//{
//	
//}

////��һҳ
//void at24c08_read_page(uint8_t addr,uint8_t *pdata,uint8_t len)
//{
//	
//}
