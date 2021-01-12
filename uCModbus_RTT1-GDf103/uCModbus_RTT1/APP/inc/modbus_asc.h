#ifndef __MODBUS_ASC_H
#define __MODBUS_ASC_H

/************************************************************************************************************************************************************************
** 版权：   2016-2026, 深圳市信为科技发展有限公司
** 文件名:  modbus_asc.h
** 作者:    庄明群
** 版本:    V1.0.0
** 日期:    2016-09-05
** 描述:    modbus ascii 功能码
** 功能:         
*************************************************************************************************************************************************************************
** 修改者:          No
** 版本:  		
** 修改内容:    No 
** 日期:            No
*************************************************************************************************************************************************************************/

/* ----------------------- Address Limit definitions -----------------------------*/

#define MBASC_HOLDING_REG_REGION1_BGEIN		        0x7130
#define MBASC_HOLDING_REG_REGION1_END			0x714F
#define MBASC_HOLDING_REG_REGION2_BGEIN		        0X7160
#define MBASC_HOLDING_REG_REGION3_BGEIN		        0X7170
#define MBASC_HOLDING_REG_REGION3_END		        0X7173
#define MBASC_HOLDING_REG_REGION4_BGEIN			0X7180

#define MBASC_INPUT_REG_REGION1_BGEIN        	        0x7100
#define MBASC_INPUT_REG_REGION1_END          	        0x710A
#define MBASC_INPUT_REG_REGION2_BGEIN        	        0x710C
#define MBASC_INPUT_REG_REGION2_END          	        0x710C
#define MBASC_INPUT_REG_REGION3_BGEIN        	        0x710E
#define MBASC_INPUT_REG_REGION3_END          	        0x710E

#define MBASC_SINGLE_COIL_ADDR_BGEIN                    0x7150
#define MBASC_SINGLE_COIL_ADDR_END                      0x7154

//#define MBASC_SPECICAL_COIL_ADDR_BGEIN           0x45B0
//#define MBASC_SPECICAL_COIL_ADDR_END             0x45B0

#define MBASC_MUL_REG_REGION1_BGEIN          MBASC_HOLDING_REG_REGION1_BGEIN
#define MBASC_MUL_REG_REGION1_END            MBASC_HOLDING_REG_REGION1_END
#define MBASC_MUL_REG_REGION2_BGEIN          MBASC_HOLDING_REG_REGION2_BGEIN
#define MBASC_MUL_REG_REGION2_END            MBASC_HOLDING_REG_REGION2_END
#define MBASC_MUL_REG_REGION3_BGEIN          MBASC_HOLDING_REG_REGION3_BGEIN
#define MBASC_MUL_REG_REGION3_END            MBASC_HOLDING_REG_REGION3_END



extern void MBASC_Function(void);
extern void MBASC_AutoUpLoadFrame(void);

#endif