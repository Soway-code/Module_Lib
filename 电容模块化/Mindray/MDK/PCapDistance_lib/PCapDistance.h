/**@file        PCapDistance.h
* @details      PCapDistance.lib��ͷ�ļ�,�����˵��ݴ���������Һλ�߶�Ӧ�õ�API����
				�����ڵ��ݴ�����,����������֧��c���Ե�MCU,��ģ��ֻ�����㴦��;
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

#ifndef __PCAPDISTANCE_H
#define __PCAPDISTANCE_H
#include "stdint.h"

#define         OPTIMIZED             1600               //�Ż�ϵ��������𿪲�122mm���ݴ�����           

/***********************************  ���������ǲ�����ʼ������ֻ��ѡ��һ��ʹ��********************************************/
/**@brief       �궨�Ĳ�����ʼ��ʹ��19��+����궨
* @param[in]    *SegmentValue : �����鷽ʽ����19����ĵ���ֵ;uint32_t SegmentValue[19]={xxxx,xxxx};
* @param[in]    *SegmentValue2 : �����鷽ʽ������α궨��3�������ֵ;uint32_t SegmentValue2[3]={xxxx,xxxx,xxxx};
* @param[in]   	altitude : ��������9�θ߶Ȳ��; 	uint16_t altitude[9]={5,15,30,50,70,90,105,115,122};            //��122mm������ݴ�����Ϊ��
* @return       ����ִ�н��
* - 1(�ɹ�)
* - 0(ʧ��)
* @note  \n
*/

uint8_t PCap_EffectiveLength_init(uint32_t *SegmentValue,uint32_t *SegmentValue2,uint16_t *altitude);

/**@brief       �궨�Ĳ�����ʼ����ʹ��19��+����궨
* @param[in]    *SegmentValue : �����鷽ʽ����19����ĵ���ֵ;uint32_t SegmentValue[19]={xxxx,xxxx};
* @param[in]   	altitude : ��������9�θ߶Ȳ��; 	uint16_t altitude[9]={5,15,30,50,70,90,105,115,122};
* @return       ����ִ�н��
* - 1(�ɹ�)
* - 0(ʧ��)
* @note  \n
*/
uint8_t PCap_EffectiveLength_init_1(uint32_t *SegmentValue,uint16_t *altitude);

/***************************************************************************************************************/


/**@brief       ���㳤��
* @param[in]    PCap_in : PCap�������˲��ĵ���ֵ;
* @param[out]   *result : ����õĸ߶�*10; 
* @param[out]   PCap_out : �����ĵ�ǰ����ֵ; 
* @return       ����ִ�н��
* - 1(�ɹ�)
* - 0(ʧ��)
* @note       ����200ms�ڵ���һ�δ˺����˴���һ���µĵ���ֵ; \n
*/
uint32_t PCap_EffectiveLength(const uint32_t PCap_in,uint32_t *result,uint32_t *PCap_out);


/**@brief       �����л�����������
* @param[in]    MulripleUp : ������ 1-200��Ĭ��50;
* @note       122->0 Һ�����·���
*/
void SetSensitivity_UP(uint8_t MulripleUp);


/**@brief       �����л�����������
* @param[in]    MulripleDE : ������ 1-200��Ĭ��10;
* @note         0->122 Һ�����Ϸ���
*/
void SetSensitivity_DE(uint8_t MulripleDe);


/**@brief       �˲��ȼ�
* @param[in]    Filtering 1-3   Ĭ��ʹ�� 2
* @note         �ȼ�Խ��Һλ�ƶ���Ӧ�ٶ�Խ��
*/
void SetFilteringLevel(uint8_t Filtering);


/**@brief        ����Ż������״ε�����������7-8mm����Ӧ���⣨�����˹��ܽ���Ӱ���������������ǰ���20mm������������,Һλ�߶Ȼ����ƫ���������ԶȲ��Բ�����ʱ����Թرմ˹��ܣ�
* @param[in]     switc  �� ������رմ˹���  ���رմ˹��� switc == 0x55 �� ����ֵ�����˹���,Ĭ�Ͽ����˹���    
* @param[out]   PCap1_out :  �����ĵ�ǰ����ֵ  
* @note         Ŀǰ������ֻ������𿪲�122mm���ݴ��������Ż� 
*/
void CapacitanceValue(uint8_t switc0,uint32_t *PCap1_out);

#endif
