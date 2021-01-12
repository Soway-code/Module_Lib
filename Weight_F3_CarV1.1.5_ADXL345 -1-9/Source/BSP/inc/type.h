#ifndef __TYPE_H
#define __TYPE_H

#include "stm32f0xx.h"	
#include "Sys_cfg.h"
#include "os.h"

#define ECHO_ENABLE 	        0
#define ECHO_DISABLE 	        1

#define FLASH_WR_ENABLE         1
#define FLASH_WR_DISABLE        0

#define true                    1
#define false                   0

//typedef enum {false = 0, true = 1} bool;

typedef struct 
{
    u8 SlaveAddr;
    u8 BaudRate;
    u8 WordLength;
    u8 StopBits;
    u8 Parity;
    u8 OutputMode;
    u8 Echo;
}UartCommTypeDef;


typedef struct
{    
    u8 cyAutoUpLoadTime;        //�Զ���������
    u8 cyFilterLevel;           //�˲��ȼ�
    u8 cyMbtWorkType;           //����ģʽ��RTU&ASCII��
    u8 bFlashWEn;               //��дEEPROMʹ��
    u8 OffsetEn;                //����ʹ��
}ProductParaTypeDef;


typedef struct
{
    float LoadADValue;
    u16 uiAI[8];
    
    u16 VehicleWeight;          //��������/�غ�����
    u16 LoadMeasureScheme;      //���ز�������
    u16 OverloadLimit;          //���ط�ֵ
    u16 OverloadLimitDevation;  //���ط�ֵƫ��
    u16 LoadLimit;              //���ط�ֵ
    u16 LoadLimitDevation;      //���ط�ֵƫ��
    u16 EmptyLimit;             //���ط�ֵ
    u16 EmptyLimitDevation;     //���ط�ֵƫ��
    u16 LightLimit;             //���ط�ֵ
    u16 LightLimitDevation;     //���ط�ֵƫ��    
    
    u16 AdcMaxCnt;
    u16 TotalPacks;
    u8 Weight_Unit;
}RunVarTypeDef;

typedef struct
{
    float UserFlowK;
    float UserFlowB;
    
    u32 CarLoadWeight;          //�����غ�����        
    u32 LoadRelativeValue;      //�������ֵ
        
}UserParamTypeDef;

typedef struct
{
    u16 wMinH;
    u16 wMinAdc;
    u16 wMidH;
    u16 wMidAdc;
    u16 wMaxH;
    u16 wMaxAdc;
}TEST_PARA;

#endif

