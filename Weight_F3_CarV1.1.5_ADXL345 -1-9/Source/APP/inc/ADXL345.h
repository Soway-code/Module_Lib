/**@file        ADXL345.c
* @brief        ��дADXL345��ͷ�ļ�,�����˶�дADXL345���������API����,
* @author      �����
* @date         2020-12-26
* @version      V1.0.0
* @copyright    2020-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/12/26  <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/

#ifndef __ADXL345_H
#define __ADXL345_H	 
#include "BSP.h"
#include "stdint.h"

/*�˿ڶ���*/ 
#define Set_SCL		GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define Reset_SCL		GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define Set_SDA		GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define Reset_SDA		GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define DataBit_SDA		GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8)
#define WriteBit_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, x?Bit_SET:Bit_RESET)


#define	SlaveAddress   0xA6	  //����������IIC�����еĴӵ�ַ,����ALT  ADDRESS��ַ���Ų�ͬ�޸�
                              //ALT  ADDRESS���Žӵ�ʱ��ַΪ0xA6���ӵ�Դʱ��ַΪ0x3A

/**@brief       ADXL345��ʼ��
*/
void Init_ADXL345(void);

/**@brief       ��ȡ X Y Z
*/
void Multiple_read_ADXL345(void);

/**@brief       ��ȡ X Y Z
*/
void Multiple_read1_ADXL345(void);

#endif
