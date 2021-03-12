/**@file        Flash_app.h
* @details      Flash_app.c��ͷ�ļ�,������Flash��ַ�궨��,������FlashӦ�õ�API����
* @details 		������STM32F10xϵ�е�Ƭ��,д���ȡFlash����
* @author       �����
* @date         2020-11-20
* @version      V1.0.0
* @copyright    2020-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/11/20  <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/
#ifndef Flash_APP_H
#define Flash_APP_H
#include "board.h"
#include "rtthread.h"

#define FLASH_PAGE_SIZE                 0x00000800U    /*!< FLASHҳ���С��2K�ֽ� */

#define IN_FLASH_BASE_ADDRESS           0x0801E800                          	//����Flash����ַ
	
#define FIRST							IN_FLASH_BASE_ADDRESS	//�״�д���־

//�������Ƕ�ƽ����������ֵ������оƬ�̶���
#define X_ACCEL_ADDR					(FIRST+0x01)	
#define Y_ACCEL_ADDR					(X_ACCEL_ADDR+0x04)			//��Ԥ��0x02
#define Z_ACCEL_ADDR					(Y_ACCEL_ADDR+0x04)	
//��ǰ45�ȱ궨ֵ
#define X_45ACCEL						(Z_ACCEL_ADDR+0x04)		
#define Y_45ACCEL						(X_45ACCEL+0x04)	
#define Z_45ACCEL						(Y_45ACCEL+0x04)

#define NULL_CAIIBRATION				(Z_45ACCEL+0x04)			//���ն˻���Է����������Ŀ���У׼
#define FULL_CAIIBRATION				(NULL_CAIIBRATION+0x04)		//���ն˻���Է���������������У׼
#define ID_ADDR							(FULL_CAIIBRATION+0x04)		//ID��ַ
#define VPT_ADDR						(ID_ADDR+0x03)				//��ֵ��ַ
#define DAMPTE							(VPT_ADDR+0x01)				//����ʱ��
#define NULL_AD_ADDR					(DAMPTE+0x01)				//�������� ADֵ
#define RELOAD_AD_ADDR					(NULL_AD_ADDR+0x04)			//�������� ADֵ
#define OVERLOAD_AD_ADDR				(RELOAD_AD_ADDR+0x04)		//�������� ����ADֵ;

#define SIGN_ADDR						(OVERLOAD_AD_ADDR+0x04)		//��־�ӵڼ���д��
#define NOW_ADDR						(SIGN_ADDR+0x01)			//Ŀǰ�Ѿ��궨�е�����
#define AD1_ADDR						(NOW_ADDR+0x01)				//10������ֵAD�궨;	21��
#define LOAD1_ADDR						(AD1_ADDR+0x2A)				//10������ֵ�����궨;	21��
#define xxx								(LOAD1_ADDR+0x2A)

/**@brief       ���ڲ�Flashָ��λ�ö�ȡһ�ֽ�����
* @param[in]    RWAddr : ��ȡ��ַ
* @return       ����ִ�н�� - 1���ֽ�����
* @note         
*/
uint8_t Flash_Read_OneByte(uint32_t RWAddr);

/**@brief       ���ڲ�Flashָ��λ��д��һ�ֽ�����
* @param[in]    RWAddr : д���ַ
* @param[in]    WrData : д������
* @return       ����ִ�н��  1 �ɹ�  0 ʧ��
* @note         
*/
uint8_t Flash_Write_OneByte(uint32_t RWAddr, uint8_t WrData);


/**@brief       ���ڲ�Flashָ��λ�ö�ȡ���ֽ�����
* @param[in]    RWAddr : ��ȡ��ַ
* @return       ����ִ�н�� - 2���ֽ�����
* @note         
*/
uint16_t Flash_Read_twoByte(uint32_t RWAddr);

/**@brief       ���ڲ�Flashָ��λ��д����ֽ�����
* @param[in]    RWAddr : д���ַ
* @param[in]    WrData : д������
* @return       ����ִ�н��  1 �ɹ�  0 ʧ��
* @note         
*/
uint8_t  Flash_Write_twoByte(uint32_t RWAddr, uint16_t WrData);


/**@brief       ���ڲ�Flashָ��λ�ö�ȡ���ֽ�����
* @param[in]    RWAddr : ��ȡ��ַ
* @return       ����ִ�н�� - 4���ֽ�����
* @note         
*/
uint32_t Flash_Read_fourByte(uint32_t RWAddr);

/**@brief       ���ڲ�Flashָ��λ��д�����ֽ�����
* @param[in]    uint32_t RWAddr : д���ַ
* @param[in]    uint32_t WrData : д������
* @return       ����ִ�н��  1 �ɹ�  0 ʧ��
* @note         
*/
uint8_t Flash_Write_fourByte(uint32_t RWAddr, uint32_t WrData);


/**@brief       ���ڲ�Flashָ��λ�ö�����ֽ�
* @param[in]    RWAddr : ����ʼ��ַ
* @param[in]    pRdbuf : ���ݻ���ָ��
* @param[in]    Rdlen : �����ݳ���
* @return       ����ִ�н��
* @note        
*/
void Flash_Read_MultiBytes(uint32_t RWAddr, uint8_t *pRdbuf, uint16_t Rdlen);


/**@brief       ���ڲ�Flashָ��λ��д����ֽ�
* @param[in]    RWAddr : д��ʼ��ַ
* @param[in]    pWrbuf : ���ݻ���ָ��
* @param[in]    Wrlen : д���ݳ���
* @return       ����ִ�н�� 1 �ɹ�  0 ʧ��
* @note  		Wrlen < 2048,����������д2048���ֽڣ��ɿ�ҳд
*/
uint8_t Flash_Write_MultiBytes(uint32_t RWAddr, uint8_t const *pWrbuf, uint16_t Wrlen);

#endif
