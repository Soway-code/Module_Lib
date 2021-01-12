#include "bsp.h"
#include "type.h"
#include "SoftwareIIC.h"
#ifndef _WEIGHT_H_
#define _WEIGHT_H_

#define UInt16 uint16_t



struct S_Weight        // �������ݽṹ
{
    UInt16 FullLoad;      //����
    UInt16 EmptyLoad;     //����
    UInt16 Weight;	//����
};


struct OutData          // �������ݺ󣬷��ظ�����������
{
    UInt16 FilterOutData; // ԭʼ�����˲��������
    UInt16 Weight;        // ���������� 1-600֮��
};


#define EMPIRICAL_BUF_LEN  5    //��ʷ/�������ݸ���

struct Empirical_Data          // ����ֵ���ݽṹ
{
    UInt16 Counter;		//���������
    UInt16 AverageFullLoad;       //����ֵƽ��ֵ
    UInt16 AverageEmptyLoad;      //����ƽ��ֵ
    UInt16 AverageWeight;         //����ƽ��ֵ
    struct S_Weight Data[EMPIRICAL_BUF_LEN];// = new Struct_Weight[5]; //��ʷ����
};



struct OutData Runing(UInt16 NewData); //��Ҫ�����ݴ�����

void Init_xx(void);                     // ��ʼ������

void ResetFlashPara(void);              // ��λflash ����


#endif