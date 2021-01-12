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

#include "BSP.h"

#define FLASH_PAGE_SIZE                 0x00000800U    /*!< FLASHҳ���С��2K�ֽ� */

//#define IN_FLASH_BASE_ADDRESS           0x0801E800                          	//����Flash����ַ
///***************************** �豸�������ڲ�Flash�е�ӳ���ַ ****************************/
//#define RUN_ADDR_BASE                     IN_FLASH_BASE_ADDRESS                                  //ϵͳ������ʼ����־/�ڲ�Flash��ʼ��ַ
//#define DEVICE_ADDR                       (RUN_ADDR_BASE + 0x01)                //�豸��ַ
//#define BAUDRATE                          (DEVICE_ADDR   + 0x01)                //������
//#define PARITY                            (BAUDRATE      + 0x01)                //��żУ��
//#define FILTER                            (PARITY        + 0x01)                //�˲�ϵ��
//#define AUTO_UPLOAD                       (FILTER        + 0x01)                //�Զ��ϴ�����
//#define COMPENSATE                        (AUTO_UPLOAD   + 0x01)                //����ʹ��
//#define FREEZE                            (COMPENSATE    + 0x01)                //�Ƿ񶳽��豸
//#define OUTPUTMODE                        (FREEZE        + 0x01)  				//�����ʽ

//#define CORRECT_K                         (OUTPUTMODE    + 0x02)               	//����ϵ��K
//#define CORRECT_B                         (CORRECT_K     + 0x02)               	//����ϵ��B
//#define UPPERTEMP						  (CORRECT_B     + 0x02)				//�¶��Ϸ�ֵ
//#define LOWERTEMP						  (UPPERTEMP     + 0x02)				//�¶��·�ֵ

//#define ORGANIZATION                      (IN_FLASH_BASE_ADDRESS+0x0400)         ///< ��֯��������
//#define PRODUCTION                        (ORGANIZATION  + 0x30)                ///< ��Ʒ����
//#define HARDWAREVER                       (PRODUCTION    + 0x30)                ///< Ӳ���汾
//#define SOFTWAREVER                       (HARDWAREVER   + 0x30)                ///< ����汾
//#define DEVICENUM                         (SOFTWAREVER   + 0x20)                ///< �豸ID
//#define CUSTOMER                          (DEVICENUM     + 0x30)                ///< �ͻ�����

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
