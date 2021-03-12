/**@file        Flash_app.h
* @details      Flash_app.c的头文件,定义了Flash地址宏定义,声明了Flash应用的API函数
* @details 		适用于STM32F10x系列单片机,写入读取Flash函数
* @author       马灿林
* @date         2020-11-20
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/11/20  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/
#ifndef Flash_APP_H
#define Flash_APP_H
#include "board.h"
#include "rtthread.h"

#define FLASH_PAGE_SIZE                 0x00000800U    /*!< FLASH页面大小，2K字节 */

#define IN_FLASH_BASE_ADDRESS           0x0801E800                          	//操作Flash基地址
	
#define FIRST							IN_FLASH_BASE_ADDRESS	//首次写入标志

//传感器角度平面数据修正值（消除芯片固定误差）
#define X_ACCEL_ADDR					(FIRST+0x01)	
#define Y_ACCEL_ADDR					(X_ACCEL_ADDR+0x04)			//多预留0x02
#define Z_ACCEL_ADDR					(Y_ACCEL_ADDR+0x04)	
//向前45度标定值
#define X_45ACCEL						(Z_ACCEL_ADDR+0x04)		
#define Y_45ACCEL						(X_45ACCEL+0x04)	
#define Z_45ACCEL						(Y_45ACCEL+0x04)

#define NULL_CAIIBRATION				(Z_45ACCEL+0x04)			//由终端或电脑发往传感器的空载校准
#define FULL_CAIIBRATION				(NULL_CAIIBRATION+0x04)		//由终端或电脑发往传感器的满载校准
#define ID_ADDR							(FULL_CAIIBRATION+0x04)		//ID地址
#define VPT_ADDR						(ID_ADDR+0x03)				//阈值地址
#define DAMPTE							(VPT_ADDR+0x01)				//阻尼时间
#define NULL_AD_ADDR					(DAMPTE+0x01)				//车辆空载 AD值
#define RELOAD_AD_ADDR					(NULL_AD_ADDR+0x04)			//车辆重载 AD值
#define OVERLOAD_AD_ADDR				(RELOAD_AD_ADDR+0x04)		//车辆超载 报警AD值;

#define SIGN_ADDR						(OVERLOAD_AD_ADDR+0x04)		//标志从第几组写入
#define NOW_ADDR						(SIGN_ADDR+0x01)			//目前已经标定有的组数
#define AD1_ADDR						(NOW_ADDR+0x01)				//10组载重值AD标定;	21组
#define LOAD1_ADDR						(AD1_ADDR+0x2A)				//10组载重值重量标定;	21组
#define xxx								(LOAD1_ADDR+0x2A)

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
