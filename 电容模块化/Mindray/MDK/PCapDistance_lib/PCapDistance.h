/**@file        PCapDistance.h
* @details      PCapDistance.lib的头文件,声明了电容传感器计算液位高度应用的API函数
				适用于电容传感器,可用于所有支持c语言的MCU,此模块只做运算处理;
* @author       马灿林
* @date         2020-10-15
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/10/15  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#ifndef __PCAPDISTANCE_H
#define __PCAPDISTANCE_H
#include "stdint.h"

#define         OPTIMIZED             1600               //优化系数针对迈瑞开槽122mm电容传感器           

/***********************************  以下两个是参数初始化函数只能选择一个使用********************************************/
/**@brief       标定的参数初始，使用19点+三点标定
* @param[in]    *SegmentValue : 数数组方式传入19个点的电容值;uint32_t SegmentValue[19]={xxxx,xxxx};
* @param[in]    *SegmentValue2 : 数数组方式传入二次标定的3个点电容值;uint32_t SegmentValue2[3]={xxxx,xxxx,xxxx};
* @param[in]   	altitude : 传感器的9段高度差别; 	uint16_t altitude[9]={5,15,30,50,70,90,105,115,122};            //以122mm迈瑞电容传感器为例
* @return       函数执行结果
* - 1(成功)
* - 0(失败)
* @note  \n
*/

uint8_t PCap_EffectiveLength_init(uint32_t *SegmentValue,uint32_t *SegmentValue2,uint16_t *altitude);

/**@brief       标定的参数初始，不使用19点+三点标定
* @param[in]    *SegmentValue : 数数组方式传入19个点的电容值;uint32_t SegmentValue[19]={xxxx,xxxx};
* @param[in]   	altitude : 传感器的9段高度差别; 	uint16_t altitude[9]={5,15,30,50,70,90,105,115,122};
* @return       函数执行结果
* - 1(成功)
* - 0(失败)
* @note  \n
*/
uint8_t PCap_EffectiveLength_init_1(uint32_t *SegmentValue,uint16_t *altitude);

/***************************************************************************************************************/


/**@brief       计算长度
* @param[in]    PCap_in : PCap不经过滤波的电容值;
* @param[out]   *result : 计算好的高度*10; 
* @param[out]   PCap_out : 处理后的当前电容值; 
* @return       函数执行结果
* - 1(成功)
* - 0(失败)
* @note       建议200ms内调用一次此函数此传入一次新的电容值; \n
*/
uint32_t PCap_EffectiveLength(const uint32_t PCap_in,uint32_t *result,uint32_t *PCap_out);


/**@brief       方向切换灵敏度设置
* @param[in]    MulripleUp : 灵敏度 1-200；默认50;
* @note       122->0 液体向下方向
*/
void SetSensitivity_UP(uint8_t MulripleUp);


/**@brief       方向切换灵敏度设置
* @param[in]    MulripleDE : 灵敏度 1-200；默认10;
* @note         0->122 液体向上方向
*/
void SetSensitivity_DE(uint8_t MulripleDe);


/**@brief       滤波等级
* @param[in]    Filtering 1-3   默认使用 2
* @note         等级越高液位移动响应速度越慢
*/
void SetFilteringLevel(uint8_t Filtering);


/**@brief        软件优化处理首次到满量程拉上7-8mm才响应问题（开启此功能将会影响测量精度满量程前面的20mm测量误差会增大,液位高度会出现偏大现象，线性度测试不过的时候可以关闭此功能）
* @param[in]     switc  ： 开启与关闭此功能  （关闭此功能 switc == 0x55 ） 其他值开启此功能,默认开启此功能    
* @param[out]   PCap1_out :  处理后的当前电容值  
* @note         目前本函数只针对迈瑞开槽122mm电容传感器做优化 
*/
void CapacitanceValue(uint8_t switc0,uint32_t *PCap1_out);

#endif
