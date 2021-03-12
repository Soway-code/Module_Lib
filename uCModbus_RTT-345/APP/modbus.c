#include "bsp.h"
#include "math.h"
#include "os_cfg_app.h"
#include "flash_if.h"
#include <string.h>

#include "weight.h"

#define MAX_CONTINUE_TIME   30  //连续，秒为单位
#define MAX_FLAG_PAR  20
static uint16_t g_wFlashFlagBuffer[MAX_FLAG_PAR];

u8 RcvDataBuf[64];
u8 g_cySendBuf[DATAPACKET_LEN];   //存储从机响应消息帧
u8 SendLen;
u8 g_cyStopStateCnt = 0;
u8 g_cyRunStateCnt = 0;
s32 RecvLen = 0;

u8 g_cyCtrlFlag; //控制标准  1：只有速度， 2：有速度，有篷布状态 3：有速度，有篷布状态，有电子围栏
u8 g_cyCarState; //车状态 0x00表示停车，0x01表示行驶
u8 g_cyPbState;  //篷布状态 0x00表示篷布打开，0x01表示篷布关闭；
u8 g_cyElState;  //电子围栏状态 0x00表示车辆停车场，0x01表示渣土装载区，0x02表示渣土卸载区，0x03表示其他区域


u16 g_wRADValue; //实时采集Ad值
u16 g_wFADValue; //经过滤波Ad值
extern u16 g_wDlyMaxTime;      //最大延时时间 0.1S 单位

const u8 Dev_Id[DEFAULT_DEV_ID_LEN] = "SW2.0 2016-08-25V02";

extern u16 Speed;


//**************************************************************************************************
// Name         : MBRTU_SendPacket()
// CreateDate   : 2013-07-10
// Author       : chenjin
// Function     : 发送数据包
// ParameIn     : 命令(cyCmd), 字符串(*cyStr), 长度(cyLen)
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

void MBRTU_SendPacket(u8 cyCmd, u8 *cyStr, u8 cyLen)                            //应答数据帧格式
{
	u8 i;
	u8 cyTemp;
	
	g_cySendBuf[0] = 0x59;
	g_cySendBuf[1] = 0x58;
	g_cySendBuf[2] = 0x00;
	g_cySendBuf[3] = cyCmd;
	g_cySendBuf[4] = 0x00;
	g_cySendBuf[5] = 0x00;
	g_cySendBuf[6] = cyLen;
	for (i = 0; i < cyLen; i++)
	{
            g_cySendBuf[7 + i] = *(cyStr + i);
	}
	
	cyLen += 7;
	cyTemp = 0;
	for (i = 0; i < cyLen; i++)
	{
            cyTemp ^= g_cySendBuf[i];
	}
	g_cySendBuf[cyLen] = cyTemp;
	cyLen++;
	g_cySendBuf[cyLen] = 0x0D;
	cyLen++;
	g_cySendBuf[cyLen] = 0x0A;
	cyLen++;
	UART1_SendData(g_cySendBuf, cyLen);
}

//**************************************************************************************************
// Name         : MBRTU_CheckRecvData()
// CreateDate   : 2013-07-10
// Author       : chenjin
// Function     : 检查数据包
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

extern BitAction g_bRevOverTime;

//0x00表示接收成功，0x01表示接收失败，0x02表示数据格式错误，0x03表示校验码错误。

u8 MBRTU_CheckRecvData(void)
{
	u8 i;
	u8 cyTemp;
	
	if (Bit_SET == g_bRevOverTime)
	{
		g_bRevOverTime = Bit_RESET;
		RcvDataBuf[0] = RcvDataBuf[2]; 
		RcvDataBuf[1] = 0x01; //表示接收失败
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	if (10 > RecvLen)
	{
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x01; //表示接收失败
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
		
	if ( (0x58 != RcvDataBuf[1]) || (0x59 != RcvDataBuf[0]) )
	{
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x02; //表示数据格式错误
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	if (0x0D != RcvDataBuf[RecvLen - 2] )
	{
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x02; //表示数据格式错误
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	if (0x0A != RcvDataBuf[RecvLen - 1] )
	{
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x02; //表示数据格式错误
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	cyTemp = 0;
	for (i = 0; i < (RecvLen - 3); i++)
	{
		cyTemp ^= RcvDataBuf[i];
	}
	
	if (cyTemp != RcvDataBuf[RecvLen - 3] )
	{		
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x03; //表示校验码错误
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	return (0);
}

//**************************************************************************************************
// Name         : MBRTU_Function()
// CreateDate   : 2013-07-10
// Author       : chenjin
// Function     : 解析命令
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

extern TEST_PARA g_tpTestPar;
extern u8 u8SendNum;
extern void WriteDefaultParam(void);
//执行结果：0x00表示指令执行成功，0x01表示指令执行失败，0x02表示数据格式错误，0x03表示校验码错误。


struct OutData myOutData;
extern struct Empirical_Data EmpiricalData;

void MBRTU_Function(void)
{

    u16 wTemp;
    uint32_t dwAddr;
    CPU_SR_ALLOC();
    RecvLen = UART1_RecvData(RcvDataBuf, sizeof(RcvDataBuf) / sizeof(RcvDataBuf[0]));
    if (RecvLen <= 0)
    {
        return;
    }
    if (0 != MBRTU_CheckRecvData())
    {
    	return;
    }
    ProductPara.bFlashWEn = FLASH_WR_ENABLE;
    
    switch (RcvDataBuf[3])
    {
    case 0x00:
        Speed = RcvDataBuf[7];
        myOutData=Runing( g_wRADValue ); // 数据处理
        RcvDataBuf[0] = 0x00;
        // 输出重量
        wTemp = myOutData.Weight;//(u16)RunVar.LoadADValue;
        RcvDataBuf[1] = (u8)(wTemp >> 8);
        RcvDataBuf[2] = (u8)(wTemp & 0x00FF);
        
        // 原始数据
        wTemp = g_wRADValue;
        RcvDataBuf[3] = (u8)(wTemp >> 8);
        RcvDataBuf[4] = (u8)(wTemp & 0x00FF);
        
        //滤波值
        wTemp = myOutData.FilterOutData;
        RcvDataBuf[5] = (u8)(wTemp >> 8);
        RcvDataBuf[6] = (u8)(wTemp & 0x00FF);
        RcvDataBuf[7] = 0xa5;
        MBRTU_SendPacket(0xB0, RcvDataBuf, 8);
        break;   
                                        
    case 0x35: // 模拟模式           
      
         // 接收从上位机传过来的值   

        Speed = RcvDataBuf[7];
        
        ((u8*)&wTemp)[0] =RcvDataBuf[8]; 
        ((u8*)&wTemp)[1] =RcvDataBuf[9]; 

        myOutData=Runing( wTemp ); // 数据处理
        
        RcvDataBuf[0] = 0x00;
        // 输出重量
        wTemp = myOutData.Weight;
        RcvDataBuf[1] = (u8)(wTemp >> 8);
        RcvDataBuf[2] = (u8)(wTemp & 0x00FF);
        
        // 输出原始数据
        
        RcvDataBuf[3] = RcvDataBuf[8];
        RcvDataBuf[4] = RcvDataBuf[9];
        
        // 输出滤波值
        wTemp =  myOutData.FilterOutData;
        RcvDataBuf[5] = (u8)(wTemp >> 8);
        RcvDataBuf[6] = (u8)(wTemp & 0x00FF);
        RcvDataBuf[7] = (u8)EmpiricalData.Counter;
        MBRTU_SendPacket(0xB0, RcvDataBuf, 8);     
        break;

						
    case 0x03:             // 返回版本号        
        memcpy(RcvDataBuf, (u8 *)&Dev_Id[0], DEFAULT_DEV_ID_LEN);
        MBRTU_SendPacket(0xB3, RcvDataBuf, DEFAULT_DEV_ID_LEN);  
        break;
						

						      	
    case 0xFF:            
        if (0x01 ==RcvDataBuf[7])
        {
            // 程序更新后，以前的经验数据清空
            ResetFlashPara(); // 复位flash 参数
            CPU_CRITICAL_ENTER();
            FLASH_If_Init();
            FLASH_ErasePage(APP_UPDATE_FLAG_ADDRESS);
            g_wFlashFlagBuffer[0] = 0x1234;
            g_wFlashFlagBuffer[1] = 0x8765;
            dwAddr = APP_UPDATE_FLAG_ADDRESS;
            FLASH_If_Write(&dwAddr, g_wFlashFlagBuffer, MAX_FLAG_PAR);
            CPU_CRITICAL_EXIT();
            RcvDataBuf[0] = 0x01;
            RcvDataBuf[1] = 0x00; //表示指令执行成功
            MBRTU_SendPacket(0xBF, RcvDataBuf, 2); 
            while(0 != u8SendNum);
            NVIC_SystemReset();							
        }
        break;
      
        default:
            break;
			          	
   }  
   ProductPara.bFlashWEn = FLASH_WR_DISABLE;
}

//**************************************************************************************************
// Name         : AutoUpLoadFrame()
// CreateDate   : 2013-07-10
// Author       : chenjin
// Function     : 自动上传
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

void AutoUpLoadFrame(void)
{
	u16 wTemp;
	
	RcvDataBuf[0] = 0x00;
        wTemp = (u16)RunVar.LoadADValue;
	RcvDataBuf[1] = (u8)(wTemp >> 8);
	RcvDataBuf[2] = (u8)(wTemp & 0x00FF);
	MBRTU_SendPacket(0xB0, RcvDataBuf, 3);
}



//**************************************************************************************************
// 名称         : AutoUpLoadTimeTransform(u8 CodeValue)
// 创建日期     : 2016-09-09
// 作者         : 梅梦醒
// 功能         : 将自动上传时间编码转换成时间
// 输入参数     : CodeValue                    编码值
// 输出参数     : 无
// 返回结果     : 转换后的秒数
// 注意和说明   : 01-被动；02-10S（缺省值）；03-20S；04-30S
// 修改内容     : 
//**************************************************************************************************    
u8 AutoUpLoadTimeTransform(u8 CodeValue)
{
    u8 code = 0;
    code = CodeValue;
    switch(code)
    {
    case 1: 
        code = code - 1;                                                        //编码为01，返回值为0时关闭主动上传
        break;
                                                                  
    case 2:                                                             
        code = code * 5;                                                        //编码为02，返回值为10时10s自动上传
        break;
        
    case 3: 
        code = (code + 1) * 5;                                                  //编码为03，返回值为20时20s自动上传                                              
        break;
        
    case 4:                                                                    //编码为04，返回值为30时30s自动上传
        code = (code + 2) * 5; 
        break;  
        
    default:                                                                    //输入其他值无反应
        code = 0;
    }
    return code;
}



//**************************************************************************************************
// Name         : APP_MB_Task()
// CreateDate   : 2013-07-10
// Author       : chenjin
// Function     : MODBUS任务
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

#define UART_SLEEP_TIME 6  

void  APP_MB_Task(void *p_arg)
{
    OS_ERR      os_err;
    u32 UartRunCnt = 0;
   // u32 UartRunCntPre = 0;

    UART1_Init(&UartComm);
    FLASH_If_Read(APP_UPDATE_FLAG_ADDRESS, g_wFlashFlagBuffer, MAX_FLAG_PAR);       
    
//    RunVar.uartInitTime = OSTimeGet(&os_err);
  
    while (1)
    {                      
       	OSTaskSemPend((OS_TICK)UART_WAIT_TIME,                                  //等待1s
                      (OS_OPT)OS_OPT_PEND_BLOCKING,
                      (CPU_TS *)0,
                      (OS_ERR *)&os_err);

#if IWDG_EN > 0
        IWDG_ReloadCounter();
#endif
        /*if((OSTimeGet(&os_err) - RunVar.uartInitTime) < (UART_SLEEP_TIME * OS_CFG_TICK_RATE_HZ))
        {
            continue;
        } */ 
   
        if (os_err == OS_ERR_NONE)
        {
            /*if(MB_RTU_MODE == ProductPara.cyMbtWorkType)
            {
                MBRTU_Function();  
            }
            else*/
            //{
                MBASC_Function();
            //}
        }
        else if (os_err == OS_ERR_TIMEOUT)                                     //串口数据超时（未接收到数据）
        {
            UartRunCnt++;                                                       //1s加一次
            if (0 != AutoUpLoadTimeTransform(ProductPara.cyAutoUpLoadTime))
            {
                if(UartRunCnt >= AutoUpLoadTimeTransform(ProductPara.cyAutoUpLoadTime))
                {   
                    UartRunCnt = 0;
                    if(MB_RTU_MODE ==  ProductPara.cyMbtWorkType)         //0-RTU模式
                    {
                        //AutoUpLoadFrame();
                    }
                    else
                    {
                        MBASC_AutoUpLoadFrame();
                    }
                }
            }
            else
            {
                UartRunCnt = 0;
            }
        }
    }
}


