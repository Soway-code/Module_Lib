#ifndef __MODBUS_ASC_H
#define __MODBUS_ASC_H

/************************************************************************************************************************************************************************
** ��Ȩ��   2016-2026, ��������Ϊ�Ƽ���չ���޹�˾
** �ļ���:  modbus_asc.h
** ����:    ׯ��Ⱥ
** �汾:    V1.0.0
** ����:    2016-09-05
** ����:    modbus ascii ������
** ����:         
*************************************************************************************************************************************************************************
** �޸���:          No
** �汾:  		
** �޸�����:    No 
** ����:            No
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