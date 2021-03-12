
/*
*********************************************************************************************************
*
*                         uC/MODBUS TARGET SPECIFIC DATA ACCESS FUNCTIONS 
*
* Filename      : mb_data.c
* Version       : V2.12
* Programmer(s) : JJL
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include <mb.h>
#include "main.h"
#include "AT24cxx.h"
/*
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
#define 	REG_HOLDING_START		0x0000		//保持寄存器起始地址
#define		REG_HOLDING_NREGS		610				//保持寄存器数量

volatile uint16_t MB_DATA[125] = {0};
volatile uint16_t* Ptr = &MB_DATA[0];

volatile uint16_t MB_DATA_04[REG_HOLDING_NREGS] = {0};
volatile uint16_t* Ptr_04 = &MB_DATA_04[0];
u32 AD_Buff[5][10];                                                             //AD值缓存
u32 Weight_Buff[5][10];                                                         //载重值缓存

u8 g_cyCtrlFlag; //控制标准  1：只有速度， 2：有速度，有篷布状态 3：有速度，有篷布状态，有电子围栏
u8 g_cyCarState; //车状态 0x00表示停车，0x01表示行驶
u8 g_cyPbState;  //篷布状态 0x00表示篷布打开，0x01表示篷布关闭；
u8 g_cyElState;  //电子围栏状态 0x00表示车辆停车场，0x01表示渣土装载区，0x02表示渣土卸载区，0x03表示其他区域
u16 g_wRADValue; //实时采集Ad值
u16 g_wFADValue; //经过滤波Ad值
/*
*********************************************************************************************************
*                                              PROTOTYPES
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC01_EN == DEF_ENABLED)
CPU_BOOLEAN
MB_CoilRd (CPU_INT16U   coil, CPU_INT16U  *perr)
{
	*perr = MODBUS_ERR_RANGE;
   return (DEF_FALSE);
}
#endif

#if (MODBUS_CFG_FC05_EN == DEF_ENABLED) || \
    (MODBUS_CFG_FC15_EN == DEF_ENABLED)
void
MB_CoilWr (CPU_INT16U coil, CPU_BOOLEAN coil_val, CPU_INT16U *perr)
{
	*perr = MODBUS_ERR_RANGE;
}
#endif

#if (MODBUS_CFG_FC02_EN == DEF_ENABLED)
CPU_BOOLEAN
MB_DIRd (CPU_INT16U  di, CPU_INT16U  *perr)
{
	*perr = MODBUS_ERR_RANGE;
	return DEF_FALSE;
}
#endif

#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
CPU_INT16U
MB_InRegRd (CPU_INT16U reg, CPU_INT16U *perr)
{
   CPU_INT16U  val;
	reg &= 0x00FF; 
	if ( reg <= REG_HOLDING_START+REG_HOLDING_NREGS-1 )	//有效地址
	{
		*perr = MODBUS_ERR_NONE;
		val = *(Ptr_04+reg);
	}
	else 
	{
		*perr = MODBUS_ERR_RANGE;
		val = 0;
	}
	return (val);
}
#endif

#if (MODBUS_CFG_FP_EN   == DEF_ENABLED)
#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
CPU_FP32
MB_InRegRdFP (CPU_INT16U reg, CPU_INT16U *perr)
{
	*perr = MODBUS_ERR_RANGE;
	return ((CPU_FP32)0);
}
#endif
#endif

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
CPU_INT16U
MB_HoldingRegRd (CPU_INT16U reg, CPU_INT16U  *perr)
{
	CPU_INT16U  val;
	reg = reg & 0x00FF;
	if ( reg <= REG_HOLDING_START+REG_HOLDING_NREGS-1 )	//有效地址
	{
		*perr = MODBUS_ERR_NONE;
		val = *(Ptr+reg);
	}
	else 
	{
		*perr = MODBUS_ERR_RANGE;
		val = 0;
	}
	return (val);
}
#endif

#if (MODBUS_CFG_FP_EN   == DEF_ENABLED)
#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
CPU_FP32
MB_HoldingRegRdFP (CPU_INT16U reg, CPU_INT16U *perr)
{
	*perr = MODBUS_ERR_RANGE;
	return ((CPU_FP32)0);
}
#endif
#endif

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED) || \
    (MODBUS_CFG_FC16_EN == DEF_ENABLED)
void
MB_HoldingRegWr (CPU_INT16U  reg, CPU_INT16U  reg_val_16, CPU_INT16U  *perr)
{
	uint8_t buf,buf1[2];
	reg = reg & 0x00FF;
	if ( reg < REG_HOLDING_START+REG_HOLDING_NREGS )		//有效地址
	{
		*perr = MODBUS_ERR_NONE;
		*(Ptr+reg) = reg_val_16;
		buf = (uint8_t)reg_val_16;
		buf1[0] = (reg_val_16>>8);
		buf1[1] = buf;
		switch (reg)
        {
        	case 0x30:{
				 I2C1_WriteByte(EEPROM_ADDRESS,SLAVE_ADDR,buf); //设备地址
	
			break;
			}
        		
        	case 0x31:{        //波特率
				 I2C1_WriteByte(EEPROM_ADDRESS,BAUDRATE,buf);

			break;				
			}
			
			case 0x32:{ //奇偶校验
					I2C1_WriteByte(EEPROM_ADDRESS,PARITY1,buf);
			break;				
			}
			
			case 0x33:{    //保留
					
			break;				
			}
			
			case 0x34:{
				I2C1_WriteByte(EEPROM_ADDRESS,OFFSET_ENABLE,buf);        //补偿使能

				break;				
			}
			
			case 0x35:{
				I2C1_WriteByte(EEPROM_ADDRESS,FILTER_LEVEL,buf);   //滤波系数
				break;				
			}
			
			case 0x36:{

				break;				
			}
        		
			case 0x37:{						   //输出修正系数K

				break;				
			}	
			case 0x38:{				       //输出修正系数B

				break;				
			}		       
			case 0x3D:{
				I2C1_WriteByte(EEPROM_ADDRESS,WEIGHT_UNIT,buf);//重量单位
				break;				
			}
			case 0x3F:{
				I2C1_WNBytesMul3T(EEPROM_ADDRESS, VEHICLE_WEIGHT, 2, buf1);  //整车重量/载荷重量
				break;				
			}
			case 0x40:{
				I2C1_WNBytesMul3T(EEPROM_ADDRESS, OVERLOAD_LIMIT, 2, buf1);    //超载阀值
				break;				
			}
			
			case 0x41:{
					I2C1_WriteByte(EEPROM_ADDRESS,OVERLOAD_LIMIT_DEVIATION,buf);	 //超载阀值偏差
				break;				
			}
			case 0x42:{
					I2C1_WriteByte(EEPROM_ADDRESS,LOAD_MEASURE_SCHEME,buf);
				break;				
			}
			case 0x43:{
					I2C1_WriteByte(EEPROM_ADDRESS,LOAD_LIMIT,buf);
				break;				
			}
			case 0x44:{
					I2C1_WriteByte(EEPROM_ADDRESS,LOAD_LIMIT_DEVIATION,buf);
				break;				
			}
			case 0x45:{
					I2C1_WriteByte(EEPROM_ADDRESS,EMPTYLOAD_LIMIT,buf);
				break;				
			}
			case 0x46:{
					I2C1_WriteByte(EEPROM_ADDRESS,EMPTYLOAD_LIMIT_DEVIATION,buf);
				break;				
			}
			case 0x47:{			
					I2C1_WriteByte(EEPROM_ADDRESS,LIGHTLOAD_LIMIT,buf);//轻载阈值
				break;				
			}

        	default:*perr = MODBUS_ERR_RANGE;
        		break;
        }
	
//		MB_ASCII_Tx(pch);                     /* Send back reply.                                                */
//
	//	HAL_NVIC_SystemReset();
		
		
	}
	else 
	{
		*perr = MODBUS_ERR_RANGE;
	}
}
#endif

#if (MODBUS_CFG_FP_EN    == DEF_ENABLED)
#if (MODBUS_CFG_FC06_EN == DEF_ENABLED) || \
    (MODBUS_CFG_FC16_EN == DEF_ENABLED)
void
MB_HoldingRegWrFP (CPU_INT16U reg, CPU_FP32 reg_val_fp, CPU_INT16U *perr)
{
	*perr = MODBUS_ERR_RANGE;
}
#endif
#endif

#if (MODBUS_CFG_FC20_EN == DEF_ENABLED)
CPU_INT16U
MB_FileRd (CPU_INT16U  file_nbr, 
           CPU_INT16U  record_nbr, 
           CPU_INT16U  ix, 
           CPU_INT08U  record_len, 
           CPU_INT16U  *perr)
{
    (void)file_nbr;
    (void)record_nbr;
    (void)ix;
    (void)record_len;
    *perr  = MODBUS_ERR_NONE;
    return (0);
}
#endif

#if (MODBUS_CFG_FC21_EN == DEF_ENABLED)
void
MB_FileWr (CPU_INT16U   file_nbr, 
					 CPU_INT16U   record_nbr, 
					 CPU_INT16U   ix, 
					 CPU_INT08U   record_len, 
					 CPU_INT16U   val, 
					 CPU_INT16U  *perr)
{
    (void)file_nbr;
    (void)record_nbr;
    (void)ix;
    (void)record_len;
    (void)val;
    *perr = MODBUS_ERR_NONE;
}
#endif
