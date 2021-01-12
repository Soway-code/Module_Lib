/**@file        Calculation.h
* @details      Calculation.lib的头文件,声明了电容传感器计算液位高度和PWM预分频值应用的API函数
				适用于直径大于15MM的电容传感器(小于15MM可使用双向标定PCapDistance.lib模块)，可用于所有支持c语言的MCU,此模块只做运算处理;
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
#ifndef __CALCULATION_H
#define __CALCULATION_H
#include "stdint.h"

/**@brief       计算液体高度参数初始
* @param[in]    *SegmentValue : 数数组方式传入11个点对应高度的电容值;uint32_t SegmentValue[11]={xxxx,xxxx};
* @param[in]   	*altitude : 11个点的高度; 	uint16_t altitude[11]={0,167,335,435,535,635,735,835,935,1023,1112};	//11个段点的高度
* @param[in]   	*frequency : 数数组方式传入11个点的频率值
				(频率单方向 递增或递减); 	uint16_t frequency[11]={5910,5457,5000,4690,4380,4110,3840,3630,3420,3152,2881};	//11个点频率
* @return       函数执行结果
* - 1(成功)
* - 0(失败)
* @note 如果只要高度，频率输入参数重复使用参数2既可，例如：parameter_init(SegmentValue,altitude,altitude); \n
*/
uint8_t parameter_init(uint32_t *SegmentValue,uint16_t *altitude,uint16_t *frequency);


/**@brief       计算液体高度
* @param[in]    PCap_in : PCap不经过滤波的电容值;
* @param[out]   *Period : 计算好的定时器,定时器需要设置成0分频; 
* @param[out]   PCap_out : 处理后的当前电容值; 
* @param[out]   altitude : 当前液位高度x10; 
* @return      返回 0 错误，1 成功
* @note       200ms内调用一次此函数传入一次新的电容值; \n
*/
uint32_t PCap_ChangeTheFrequency(const uint32_t PCap_in,uint32_t *Period,uint32_t *PCap_out, uint32_t *altitude);

/******************************以上2个函数是必须使用*******************************************************/

/******************************FrequencyCalibration函数使用频率输出时用于零点校准*******************************************************/

/**@brief       校准零点输出频率
* @param[in]    Cardinality : 默认48M时为 Cardinality = 4800;
				Cardinality 根据单片机所以的频率来决定
* @note   Cardinality  改大频率 Cardinality就小   \n
*/
void FrequencyCalibration(uint16_t Cardinality);


/******************************以下三个函数看情况使用*******************************************************/

/**@brief       滤波等级设置
* @param[in]    Filtering 1-7   默认使用 4
* @note 
	Filtering  滤波等级分为 6级，默认其他值使用4级滤波，修改滤波等级后建议重新标定
	1 滤波速度快			//适用于原始电容值幅度变化小的场合
	2 滤波速度中快
	3 滤波速度中慢
	4 滤波速度慢			//适用于原始电容值幅度变化大的场合
							
							//不建议使用 6和7 进行滤波
	6 滤波速度中			//抑制那种周期性电容值突然变小 
	7 滤波速度中			//抑制那种周期性电容值突然变大
*/
void SetFilteringLevel(uint8_t Filtering);


/**@brief       滤波
* @param[in]    PCap_buf ：PCap芯片读取的电容值
* @return    	滤波后的值
* @note        \n
*/
uint32_t PCap_Get_filter(uint32_t PCap_buf);


/**@brief      获取输出频率
* @return      现在输出pwm的频率
*/
uint16_t get_hz(void);

#endif 

