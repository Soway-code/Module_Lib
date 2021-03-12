/**@file        Calculation.h
* @details      Calculation.lib��ͷ�ļ�,�����˵��ݴ���������Һλ�߶Ⱥ�PWMԤ��ƵֵӦ�õ�API����
				������ֱ������15MM�ĵ��ݴ�����(С��15MM��ʹ��˫��궨PCapDistance.libģ��)������������֧��c���Ե�MCU,��ģ��ֻ�����㴦��;
* @author       �����
* @date         2020-10-15
* @version      V1.0.0
* @copyright    2020-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/10/15  <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/
#ifndef __CALCULATION_H
#define __CALCULATION_H
#include "stdint.h"

/**@brief       ����Һ��߶Ȳ�����ʼ
* @param[in]    *SegmentValue : �����鷽ʽ����11�����Ӧ�߶ȵĵ���ֵ;uint32_t SegmentValue[11]={xxxx,xxxx};
* @param[in]   	*altitude : 11����ĸ߶�; 	uint16_t altitude[11]={0,167,335,435,535,635,735,835,935,1023,1112};	//11���ε�ĸ߶�
* @param[in]   	*frequency : �����鷽ʽ����11�����Ƶ��ֵ
				(Ƶ�ʵ����� ������ݼ�); 	uint16_t frequency[11]={5910,5457,5000,4690,4380,4110,3840,3630,3420,3152,2881};	//11����Ƶ��
* @return       ����ִ�н��
* - 1(�ɹ�)
* - 0(ʧ��)
* @note ���ֻҪ�߶ȣ�Ƶ����������ظ�ʹ�ò���2�ȿɣ����磺parameter_init(SegmentValue,altitude,altitude); \n
*/
uint8_t parameter_init(uint32_t *SegmentValue,uint16_t *altitude,uint16_t *frequency);


/**@brief       ����Һ��߶�
* @param[in]    PCap_in : PCap�������˲��ĵ���ֵ;
* @param[out]   *Period : ����õĶ�ʱ��,��ʱ����Ҫ���ó�0��Ƶ; 
* @param[out]   PCap_out : �����ĵ�ǰ����ֵ; 
* @param[out]   altitude : ��ǰҺλ�߶�x10; 
* @return      ���� 0 ����1 �ɹ�
* @note       200ms�ڵ���һ�δ˺�������һ���µĵ���ֵ; \n
*/
uint32_t PCap_ChangeTheFrequency(const uint32_t PCap_in,uint32_t *Period,uint32_t *PCap_out, uint32_t *altitude);

/******************************����2�������Ǳ���ʹ��*******************************************************/

/******************************FrequencyCalibration����ʹ��Ƶ�����ʱ�������У׼*******************************************************/

/**@brief       У׼������Ƶ��
* @param[in]    Cardinality : Ĭ��48MʱΪ Cardinality = 4800;
				Cardinality ���ݵ�Ƭ�����Ե�Ƶ��������
* @note   Cardinality  �Ĵ�Ƶ�� Cardinality��С   \n
*/
void FrequencyCalibration(uint16_t Cardinality);


/******************************�����������������ʹ��*******************************************************/

/**@brief       �˲��ȼ�����
* @param[in]    Filtering 1-7   Ĭ��ʹ�� 4
* @note 
	Filtering  �˲��ȼ���Ϊ 6����Ĭ������ֵʹ��4���˲����޸��˲��ȼ��������±궨
	1 �˲��ٶȿ�			//������ԭʼ����ֵ���ȱ仯С�ĳ���
	2 �˲��ٶ��п�
	3 �˲��ٶ�����
	4 �˲��ٶ���			//������ԭʼ����ֵ���ȱ仯��ĳ���
							
							//������ʹ�� 6��7 �����˲�
	6 �˲��ٶ���			//�������������Ե���ֵͻȻ��С 
	7 �˲��ٶ���			//�������������Ե���ֵͻȻ���
*/
void SetFilteringLevel(uint8_t Filtering);


/**@brief       �˲�
* @param[in]    PCap_buf ��PCapоƬ��ȡ�ĵ���ֵ
* @return    	�˲����ֵ
* @note        \n
*/
uint32_t PCap_Get_filter(uint32_t PCap_buf);


/**@brief      ��ȡ���Ƶ��
* @return      �������pwm��Ƶ��
*/
uint16_t get_hz(void);

#endif 

