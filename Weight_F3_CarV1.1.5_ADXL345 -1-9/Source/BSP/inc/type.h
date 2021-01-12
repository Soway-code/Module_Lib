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
    u8 cyAutoUpLoadTime;        //自动发送周期
    u8 cyFilterLevel;           //滤波等级
    u8 cyMbtWorkType;           //工作模式（RTU&ASCII）
    u8 bFlashWEn;               //读写EEPROM使能
    u8 OffsetEn;                //补偿使能
}ProductParaTypeDef;


typedef struct
{
    float LoadADValue;
    u16 uiAI[8];
    
    u16 VehicleWeight;          //整车重量/载荷重量
    u16 LoadMeasureScheme;      //载重测量方案
    u16 OverloadLimit;          //超载阀值
    u16 OverloadLimitDevation;  //超载阀值偏差
    u16 LoadLimit;              //重载阀值
    u16 LoadLimitDevation;      //重载阀值偏差
    u16 EmptyLimit;             //空载阀值
    u16 EmptyLimitDevation;     //空载阀值偏差
    u16 LightLimit;             //轻载阀值
    u16 LightLimitDevation;     //轻载阀值偏差    
    
    u16 AdcMaxCnt;
    u16 TotalPacks;
    u8 Weight_Unit;
}RunVarTypeDef;

typedef struct
{
    float UserFlowK;
    float UserFlowB;
    
    u32 CarLoadWeight;          //车辆载荷重量        
    u32 LoadRelativeValue;      //载重相对值
        
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

