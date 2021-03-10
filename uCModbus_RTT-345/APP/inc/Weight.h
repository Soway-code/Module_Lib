#include "bsp.h"
#include "type.h"
#include "SoftwareIIC.h"
#ifndef _WEIGHT_H_
#define _WEIGHT_H_

#define UInt16 uint16_t



struct S_Weight        // 重量数据结构
{
    UInt16 FullLoad;      //满载
    UInt16 EmptyLoad;     //空载
    UInt16 Weight;	//重量
};


struct OutData          // 处理数据后，返回给车机的数据
{
    UInt16 FilterOutData; // 原始数据滤波后的数据
    UInt16 Weight;        // 处理后的数据 1-600之间
};


#define EMPIRICAL_BUF_LEN  5    //历史/经验数据个数

struct Empirical_Data          // 经验值数据结构
{
    UInt16 Counter;		//经验计数器
    UInt16 AverageFullLoad;       //满载值平均值
    UInt16 AverageEmptyLoad;      //空载平均值
    UInt16 AverageWeight;         //重量平均值
    struct S_Weight Data[EMPIRICAL_BUF_LEN];// = new Struct_Weight[5]; //历史数据
};



struct OutData Runing(UInt16 NewData); //主要的数据处理函数

void Init_xx(void);                     // 初始化参数

void ResetFlashPara(void);              // 复位flash 参数


#endif