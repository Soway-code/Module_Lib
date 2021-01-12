/**@file        iic.h
* @details      iic.c��ͷ�ļ�,����������iic�ܽźͶ�ʱ��Ӧ�õĺ궨��,������iicӦ�õ�API����
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

#ifndef __IIC_H
#define __IIC_H
/**
*@brief main.h
*main.hͷ�ļ��������ͷ�ļ��� #include "stm32f4xx_hal.h"
*�������ʹ��STM32CubeMX������ɵĹ���û�� main.h ����ע��
* main.h ����϶�Ӧ�� #include "stm32(xx)xx_hal.h"
*/
#include "main.h"	


/**
*@brief HclkFrequencyĬ������Ϊ64M����ʹ��Ƶ�����޸�	12M/24M/32M/48M/64M/72M/168M
*/
#define HclkFrequency 	64	

/**
*@brief IIC�����ٶ�Ĭ������Ϊ100k,����ʹ��������޸�Ϊ 100����400
*/
#define IIC_SPEED		100

/**
*@brief IIC��ʱ���ߺ������߿���ͨ���޸����µĺ궨��
*SCL_RCC_GPIOX_CLK 			//����IIC SCL�ܽ�GPIOxʱ��
*SDA_RCC_GPIOX_CLK			//����IIC SDA�ܽ�GPIOxʱ��
*IIC_SCL_GPIOX				//ѡ��SCL�ܽŵ�GPIOx�˿�
*IIC_SDA_GPIOX				//ѡ��SDA�ܽŵ�GPIOx�˿�
*SCL_PINX					//ѡ��SCL�ܽ�
*SDA_PINX					//ѡ��SDA�ܽ�
*/
#define SCL_RCC_GPIOX_CLK 		__HAL_RCC_GPIOA_CLK_ENABLE();		//����GPIOxʱ��
#define SDA_RCC_GPIOX_CLK 		__HAL_RCC_GPIOA_CLK_ENABLE();		//����GPIOxʱ��

#define IIC_SCL_GPIOX 		GPIOA								//ѡ��DS18B20 GPIOx�˿�
#define SCL_PINX 			GPIO_PIN_11							//ѡ��DS18B20 ���ӵĹܽ�

#define IIC_SDA_GPIOX 		GPIOA
#define SDA_PINX 			GPIO_PIN_12	

/**
������������ #include "iic.h"�ȿ���ʹ�����½ӿں�����
void iic_init(void);				//iic��ʼ��
void iic_start(void);				//��ʼ�ź�
void iic_stop(void);				//ֹͣ�ź�
uint8_t iic_wait_ack(void);			//�ȴ�ACK 1-��Ч 0-��Ч
void iic_ack(uint8_t ack);			//������Ч/��ЧӦ�� 1-��Ч 0-��Ч
void iic_send_byte(uint8_t txd);	//����һ���ֽ�
uint8_t iic_read_byte(void);		//����һ���ֽ�
*/

/**@brief   ��ʼ��iic
*/
void iic_init(void);

/**
* @brief iic��ʼ�ź�
*/
void iic_start(void);

/**
* @brief iicֹͣ�ź�
*/
void iic_stop(void);

/**@brief       �ȴ�ACK 
* @return       ����ִ�н��
* - ���ؽ��    : 1-ACK��Ч 0-ACK��Ч
*/
uint8_t iic_wait_ack(void);

/**@brief       ������Ч/��ЧӦ��
* @param[in]    ack : 1-��Ч 0-��Ч
*/
void iic_ack(uint8_t ack);

/**@brief       ����һ���ֽ�
* @param[in]    txd : Ҫд����ֽ�
*/
void iic_send_byte(uint8_t txd);

/**@brief       ����һ���ֽ�
* @return       ����ִ�н��
* - ���ؽ��    : ����һ���ֽ�uint8_t���͵�����
*/
uint8_t iic_read_byte(void);

#endif
