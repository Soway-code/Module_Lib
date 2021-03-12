/**@file        flash.c
* @details      flash.c的头文件,声明了STM32F103系列flash应用的API函数
* @author       马灿林
* @date         2020-08-14
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/08/14  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#ifndef __FLASH_H
#define __FLASH_H 			   
#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#define FLASH_PAGE_SIZE                 0x00000800U    /*!< FLASH页面大小，2K字节 */

#define ADD_I0 		0x801F000 
#define ADD_I1		(ADD_I0 + 0x02)
#define ADD_I2		(ADD_I1 + 0x02)
#define ADD_I3		(ADD_I2 + 0x02)
#define ADD_I4		(ADD_I3 + 0x02)
#define ADD_I5		(ADD_I4 + 0x02)
#define ADD_I6		(ADD_I5 + 0x02)
#define ADD_I7		(ADD_I6 + 0x02)
#define SIGN		(ADD_I7 + 0x02)

#define CH0_0 		0x801F800 
#define CH0_1 		(CH0_0 + 0X04)
#define CH0_2 		(CH0_1 + 0X04)
#define CH0_3 		(CH0_2 + 0X04)
#define CH0_4 		(CH0_3 + 0X04)
#define CH0_5 		(CH0_4 + 0X04)
#define CH0_6 		(CH0_5 + 0X04)
#define CH0_7 		(CH0_6 + 0X04)
#define CH0_8 		(CH0_7 + 0X04)
#define CH0_9 		(CH0_8 + 0X04)
#define CH0_10 		(CH0_9 + 0X04)
#define CH0_11 		(CH0_10 + 0X04)

#define CH1_0 		(CH0_11 + 0X04)
#define CH1_1 		(CH1_0 + 0X04)
#define CH1_2 		(CH1_1 + 0X04)
#define CH1_3 		(CH1_2 + 0X04)
#define CH1_4 		(CH1_3 + 0X04)
#define CH1_5 		(CH1_4 + 0X04)
#define CH1_6 		(CH1_5 + 0X04)
#define CH1_7 		(CH1_6 + 0X04)
#define CH1_8 		(CH1_7 + 0X04)
#define CH1_9 		(CH1_8 + 0X04)
#define CH1_10 		(CH1_9 + 0X04)
#define CH1_11 		(CH1_10 + 0X04)

#define CH2_0 		(CH1_11 + 0X04)
#define CH2_1 		(CH2_0 + 0X04)
#define CH2_2 		(CH2_1 + 0X04)
#define CH2_3 		(CH2_2 + 0X04)
#define CH2_4 		(CH2_3 + 0X04)
#define CH2_5 		(CH2_4 + 0X04)
#define CH2_6 		(CH2_5 + 0X04)
#define CH2_7 		(CH2_6 + 0X04)
#define CH2_8 		(CH2_7 + 0X04)
#define CH2_9 		(CH2_8 + 0X04)
#define CH2_10 		(CH2_9 + 0X04)
#define CH2_11 		(CH2_10 + 0X04)

#define CH3_0 		(CH2_11 + 0X04)
#define CH3_1 		(CH3_0 + 0X04)
#define CH3_2 		(CH3_1 + 0X04)
#define CH3_3 		(CH3_2 + 0X04)
#define CH3_4 		(CH3_3 + 0X04)
#define CH3_5 		(CH3_4 + 0X04)
#define CH3_6 		(CH3_5 + 0X04)
#define CH3_7 		(CH3_6 + 0X04)
#define CH3_8 		(CH3_7 + 0X04)
#define CH3_9 		(CH3_8 + 0X04)
#define CH3_10 		(CH3_9 + 0X04)
#define CH3_11 		(CH3_10 + 0X04)

#define CH4_0 		(CH3_11 + 0X04)
#define CH4_1 		(CH4_0 + 0X04)
#define CH4_2 		(CH4_1 + 0X04)
#define CH4_3 		(CH4_2 + 0X04)
#define CH4_4 		(CH4_3 + 0X04)
#define CH4_5 		(CH4_4 + 0X04)
#define CH4_6 		(CH4_5 + 0X04)
#define CH4_7 		(CH4_6 + 0X04)
#define CH4_8 		(CH4_7 + 0X04)
#define CH4_9 		(CH4_8 + 0X04)
#define CH4_10 		(CH4_9 + 0X04)
#define CH4_11 		(CH4_10 + 0X04)

#define CH5_0 		(CH4_11 + 0X04)
#define CH5_1 		(CH5_0 + 0X04)
#define CH5_2 		(CH5_1 + 0X04)
#define CH5_3 		(CH5_2 + 0X04)
#define CH5_4 		(CH5_3 + 0X04)
#define CH5_5 		(CH5_4 + 0X04)
#define CH5_6 		(CH5_5 + 0X04)
#define CH5_7 		(CH5_6 + 0X04)
#define CH5_8 		(CH5_7 + 0X04)
#define CH5_9 		(CH5_8 + 0X04)
#define CH5_10 		(CH5_9 + 0X04)
#define CH5_11 		(CH5_10 + 0X04)

#define CH6_0 		(CH5_11 + 0X04)
#define CH6_1 		(CH6_0 + 0X04)
#define CH6_2 		(CH6_1 + 0X04)
#define CH6_3 		(CH6_2 + 0X04)
#define CH6_4 		(CH6_3 + 0X04)
#define CH6_5 		(CH6_4 + 0X04)
#define CH6_6 		(CH6_5 + 0X04)
#define CH6_7 		(CH6_6 + 0X04)
#define CH6_8 		(CH6_7 + 0X04)
#define CH6_9 		(CH6_8 + 0X04)
#define CH6_10 		(CH6_9 + 0X04)
#define CH6_11 		(CH6_10 + 0X04)

#define CH7_0 		(CH6_11 + 0X04)
#define CH7_1 		(CH7_0 + 0X04)
#define CH7_2 		(CH7_1 + 0X04)
#define CH7_3 		(CH7_2 + 0X04)
#define CH7_4 		(CH7_3 + 0X04)
#define CH7_5 		(CH7_4 + 0X04)
#define CH7_6 		(CH7_5 + 0X04)
#define CH7_7 		(CH7_6 + 0X04)
#define CH7_8 		(CH7_7 + 0X04)
#define CH7_9 		(CH7_8 + 0X04)
#define CH7_10 		(CH7_9 + 0X04)
#define CH7_11 		(CH7_10 + 0X04)

void FLASH_W(u32 add,u16 dat);
u16 FLASH_R(u32 add);

/**@brief       向内部Flash指定位置读取一字节数据
* @param[in]    RWAddr : 读取地址
* @return       函数执行结果 - 1个字节数据
* @note         
*/
uint8_t Flash_Read_OneByte(uint32_t RWAddr);

/**@brief       向内部Flash指定位置写入一字节数据
* @param[in]    RWAddr : 写入地址
* @param[in]    WrData : 写入数据
* @return       函数执行结果  1 成功  0 失败
* @note         
*/
uint8_t Flash_Write_OneByte(uint32_t RWAddr, uint8_t WrData);


/**@brief       向内部Flash指定位置读取二字节数据
* @param[in]    RWAddr : 读取地址
* @return       函数执行结果 - 2个字节数据
* @note         
*/
uint16_t Flash_Read_twoByte(uint32_t RWAddr);

/**@brief       向内部Flash指定位置写入二字节数据
* @param[in]    RWAddr : 写入地址
* @param[in]    WrData : 写入数据
* @return       函数执行结果  1 成功  0 失败
* @note         
*/
uint8_t  Flash_Write_twoByte(uint32_t RWAddr, uint16_t WrData);


/**@brief       向内部Flash指定位置读取四字节数据
* @param[in]    RWAddr : 读取地址
* @return       函数执行结果 - 4个字节数据
* @note         
*/
uint32_t Flash_Read_fourByte(uint32_t RWAddr);

/**@brief       向内部Flash指定位置写入四字节数据
* @param[in]    uint32_t RWAddr : 写入地址
* @param[in]    uint32_t WrData : 写入数据
* @return       函数执行结果  1 成功  0 失败
* @note         
*/
uint8_t Flash_Write_fourByte(uint32_t RWAddr, uint32_t WrData);


/**@brief       从内部Flash指定位置读多个字节
* @param[in]    RWAddr : 读起始地址
* @param[in]    pRdbuf : 数据缓存指针
* @param[in]    Rdlen : 读数据长度
* @return       函数执行结果
* @note        
*/
void Flash_Read_MultiBytes(uint32_t RWAddr, uint8_t *pRdbuf, uint16_t Rdlen);


/**@brief       向内部Flash指定位置写多个字节
* @param[in]    RWAddr : 写起始地址
* @param[in]    pWrbuf : 数据缓存指针
* @param[in]    Wrlen : 写数据长度
* @return       函数执行结果 1 成功  0 失败
* @note  		Wrlen < 2048,单次最大可以写2048个字节，可跨页写
*/
uint8_t Flash_Write_MultiBytes(uint32_t RWAddr, uint8_t const *pWrbuf, uint16_t Wrlen);


#endif
