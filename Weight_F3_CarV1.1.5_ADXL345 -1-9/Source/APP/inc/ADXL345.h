/**@file        ADXL345.c
* @brief        读写ADXL345的头文件,声明了读写ADXL345驱动程序的API函数,
* @author      马灿林
* @date         2020-12-26
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/12/26  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#ifndef __ADXL345_H
#define __ADXL345_H	 
#include "BSP.h"
#include "stdint.h"

/*端口定义*/ 
#define Set_SCL		GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define Reset_SCL		GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define Set_SDA		GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define Reset_SDA		GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define DataBit_SDA		GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8)
#define WriteBit_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, x?Bit_SET:Bit_RESET)


#define	SlaveAddress   0xA6	  //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
                              //ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A

/**@brief       ADXL345初始化
*/
void Init_ADXL345(void);

/**@brief       读取 X Y Z
*/
void Multiple_read_ADXL345(void);

/**@brief       读取 X Y Z
*/
void Multiple_read1_ADXL345(void);

#endif
