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

#include "BSP.h"

#define FLASH_PAGE_SIZE                 0x00000800U    /*!< FLASH页面大小，2K字节 */

//#define IN_FLASH_BASE_ADDRESS           0x0801E800                          	//操作Flash基地址
///***************************** 设备参数在内部Flash中的映射地址 ****************************/
//#define RUN_ADDR_BASE                     IN_FLASH_BASE_ADDRESS                                  //系统参数初始化标志/内部Flash起始地址
//#define DEVICE_ADDR                       (RUN_ADDR_BASE + 0x01)                //设备地址
//#define BAUDRATE                          (DEVICE_ADDR   + 0x01)                //波特率
//#define PARITY                            (BAUDRATE      + 0x01)                //奇偶校验
//#define FILTER                            (PARITY        + 0x01)                //滤波系数
//#define AUTO_UPLOAD                       (FILTER        + 0x01)                //自动上传周期
//#define COMPENSATE                        (AUTO_UPLOAD   + 0x01)                //补偿使能
//#define FREEZE                            (COMPENSATE    + 0x01)                //是否冻结设备
//#define OUTPUTMODE                        (FREEZE        + 0x01)  				//输出方式

//#define CORRECT_K                         (OUTPUTMODE    + 0x02)               	//修正系数K
//#define CORRECT_B                         (CORRECT_K     + 0x02)               	//修正系数B
//#define UPPERTEMP						  (CORRECT_B     + 0x02)				//温度上阀值
//#define LOWERTEMP						  (UPPERTEMP     + 0x02)				//温度下阀值

//#define ORGANIZATION                      (IN_FLASH_BASE_ADDRESS+0x0400)         ///< 组织机构代码
//#define PRODUCTION                        (ORGANIZATION  + 0x30)                ///< 产品代码
//#define HARDWAREVER                       (PRODUCTION    + 0x30)                ///< 硬件版本
//#define SOFTWAREVER                       (HARDWAREVER   + 0x30)                ///< 软件版本
//#define DEVICENUM                         (SOFTWAREVER   + 0x20)                ///< 设备ID
//#define CUSTOMER                          (DEVICENUM     + 0x30)                ///< 客户代码

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
