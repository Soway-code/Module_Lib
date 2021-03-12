#include "bsp.h"
#include "math.h"
#include "os_cfg_app.h"
#include "flash_if.h"
#include <string.h>

#include "weight.h"

#define MAX_CONTINUE_TIME   30  //��������Ϊ��λ
#define MAX_FLAG_PAR  20
static uint16_t g_wFlashFlagBuffer[MAX_FLAG_PAR];

u8 RcvDataBuf[64];
u8 g_cySendBuf[DATAPACKET_LEN];   //�洢�ӻ���Ӧ��Ϣ֡
u8 SendLen;
u8 g_cyStopStateCnt = 0;
u8 g_cyRunStateCnt = 0;
s32 RecvLen = 0;

u8 g_cyCtrlFlag; //���Ʊ�׼  1��ֻ���ٶȣ� 2�����ٶȣ�����״̬ 3�����ٶȣ�����״̬���е���Χ��
u8 g_cyCarState; //��״̬ 0x00��ʾͣ����0x01��ʾ��ʻ
u8 g_cyPbState;  //��״̬ 0x00��ʾ�񲼴򿪣�0x01��ʾ�񲼹رգ�
u8 g_cyElState;  //����Χ��״̬ 0x00��ʾ����ͣ������0x01��ʾ����װ������0x02��ʾ����ж������0x03��ʾ��������


u16 g_wRADValue; //ʵʱ�ɼ�Adֵ
u16 g_wFADValue; //�����˲�Adֵ
extern u16 g_wDlyMaxTime;      //�����ʱʱ�� 0.1S ��λ

const u8 Dev_Id[DEFAULT_DEV_ID_LEN] = "SW2.0 2016-08-25V02";

extern u16 Speed;


//**************************************************************************************************
// Name         : MBRTU_SendPacket()
// CreateDate   : 2013-07-10
// Author       : chenjin
// Function     : �������ݰ�
// ParameIn     : ����(cyCmd), �ַ���(*cyStr), ����(cyLen)
// ParameOut    : ��
// Return       : ��
// commments    :
// Modification :
//**************************************************************************************************

void MBRTU_SendPacket(u8 cyCmd, u8 *cyStr, u8 cyLen)                            //Ӧ������֡��ʽ
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
// Function     : ������ݰ�
// ParameIn     : ��
// ParameOut    : ��
// Return       : ��
// commments    :
// Modification :
//**************************************************************************************************

extern BitAction g_bRevOverTime;

//0x00��ʾ���ճɹ���0x01��ʾ����ʧ�ܣ�0x02��ʾ���ݸ�ʽ����0x03��ʾУ�������

u8 MBRTU_CheckRecvData(void)
{
	u8 i;
	u8 cyTemp;
	
	if (Bit_SET == g_bRevOverTime)
	{
		g_bRevOverTime = Bit_RESET;
		RcvDataBuf[0] = RcvDataBuf[2]; 
		RcvDataBuf[1] = 0x01; //��ʾ����ʧ��
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	if (10 > RecvLen)
	{
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x01; //��ʾ����ʧ��
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
		
	if ( (0x58 != RcvDataBuf[1]) || (0x59 != RcvDataBuf[0]) )
	{
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x02; //��ʾ���ݸ�ʽ����
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	if (0x0D != RcvDataBuf[RecvLen - 2] )
	{
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x02; //��ʾ���ݸ�ʽ����
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	if (0x0A != RcvDataBuf[RecvLen - 1] )
	{
		RcvDataBuf[0] = RcvDataBuf[3]; 
		RcvDataBuf[1] = 0x02; //��ʾ���ݸ�ʽ����
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
		RcvDataBuf[1] = 0x03; //��ʾУ�������
		MBRTU_SendPacket(0xBB, RcvDataBuf, 2);
		return (1);
	}
	
	return (0);
}

//**************************************************************************************************
// Name         : MBRTU_Function()
// CreateDate   : 2013-07-10
// Author       : chenjin
// Function     : ��������
// ParameIn     : ��
// ParameOut    : ��
// Return       : ��
// commments    :
// Modification :
//**************************************************************************************************

extern TEST_PARA g_tpTestPar;
extern u8 u8SendNum;
extern void WriteDefaultParam(void);
//ִ�н����0x00��ʾָ��ִ�гɹ���0x01��ʾָ��ִ��ʧ�ܣ�0x02��ʾ���ݸ�ʽ����0x03��ʾУ�������


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
        myOutData=Runing( g_wRADValue ); // ���ݴ���
        RcvDataBuf[0] = 0x00;
        // �������
        wTemp = myOutData.Weight;//(u16)RunVar.LoadADValue;
        RcvDataBuf[1] = (u8)(wTemp >> 8);
        RcvDataBuf[2] = (u8)(wTemp & 0x00FF);
        
        // ԭʼ����
        wTemp = g_wRADValue;
        RcvDataBuf[3] = (u8)(wTemp >> 8);
        RcvDataBuf[4] = (u8)(wTemp & 0x00FF);
        
        //�˲�ֵ
        wTemp = myOutData.FilterOutData;
        RcvDataBuf[5] = (u8)(wTemp >> 8);
        RcvDataBuf[6] = (u8)(wTemp & 0x00FF);
        RcvDataBuf[7] = 0xa5;
        MBRTU_SendPacket(0xB0, RcvDataBuf, 8);
        break;   
                                        
    case 0x35: // ģ��ģʽ           
      
         // ���մ���λ����������ֵ   

        Speed = RcvDataBuf[7];
        
        ((u8*)&wTemp)[0] =RcvDataBuf[8]; 
        ((u8*)&wTemp)[1] =RcvDataBuf[9]; 

        myOutData=Runing( wTemp ); // ���ݴ���
        
        RcvDataBuf[0] = 0x00;
        // �������
        wTemp = myOutData.Weight;
        RcvDataBuf[1] = (u8)(wTemp >> 8);
        RcvDataBuf[2] = (u8)(wTemp & 0x00FF);
        
        // ���ԭʼ����
        
        RcvDataBuf[3] = RcvDataBuf[8];
        RcvDataBuf[4] = RcvDataBuf[9];
        
        // ����˲�ֵ
        wTemp =  myOutData.FilterOutData;
        RcvDataBuf[5] = (u8)(wTemp >> 8);
        RcvDataBuf[6] = (u8)(wTemp & 0x00FF);
        RcvDataBuf[7] = (u8)EmpiricalData.Counter;
        MBRTU_SendPacket(0xB0, RcvDataBuf, 8);     
        break;

						
    case 0x03:             // ���ذ汾��        
        memcpy(RcvDataBuf, (u8 *)&Dev_Id[0], DEFAULT_DEV_ID_LEN);
        MBRTU_SendPacket(0xB3, RcvDataBuf, DEFAULT_DEV_ID_LEN);  
        break;
						

						      	
    case 0xFF:            
        if (0x01 ==RcvDataBuf[7])
        {
            // ������º���ǰ�ľ����������
            ResetFlashPara(); // ��λflash ����
            CPU_CRITICAL_ENTER();
            FLASH_If_Init();
            FLASH_ErasePage(APP_UPDATE_FLAG_ADDRESS);
            g_wFlashFlagBuffer[0] = 0x1234;
            g_wFlashFlagBuffer[1] = 0x8765;
            dwAddr = APP_UPDATE_FLAG_ADDRESS;
            FLASH_If_Write(&dwAddr, g_wFlashFlagBuffer, MAX_FLAG_PAR);
            CPU_CRITICAL_EXIT();
            RcvDataBuf[0] = 0x01;
            RcvDataBuf[1] = 0x00; //��ʾָ��ִ�гɹ�
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
// Function     : �Զ��ϴ�
// ParameIn     : ��
// ParameOut    : ��
// Return       : ��
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
// ����         : AutoUpLoadTimeTransform(u8 CodeValue)
// ��������     : 2016-09-09
// ����         : ÷����
// ����         : ���Զ��ϴ�ʱ�����ת����ʱ��
// �������     : CodeValue                    ����ֵ
// �������     : ��
// ���ؽ��     : ת���������
// ע���˵��   : 01-������02-10S��ȱʡֵ����03-20S��04-30S
// �޸�����     : 
//**************************************************************************************************    
u8 AutoUpLoadTimeTransform(u8 CodeValue)
{
    u8 code = 0;
    code = CodeValue;
    switch(code)
    {
    case 1: 
        code = code - 1;                                                        //����Ϊ01������ֵΪ0ʱ�ر������ϴ�
        break;
                                                                  
    case 2:                                                             
        code = code * 5;                                                        //����Ϊ02������ֵΪ10ʱ10s�Զ��ϴ�
        break;
        
    case 3: 
        code = (code + 1) * 5;                                                  //����Ϊ03������ֵΪ20ʱ20s�Զ��ϴ�                                              
        break;
        
    case 4:                                                                    //����Ϊ04������ֵΪ30ʱ30s�Զ��ϴ�
        code = (code + 2) * 5; 
        break;  
        
    default:                                                                    //��������ֵ�޷�Ӧ
        code = 0;
    }
    return code;
}



//**************************************************************************************************
// Name         : APP_MB_Task()
// CreateDate   : 2013-07-10
// Author       : chenjin
// Function     : MODBUS����
// ParameIn     : ��
// ParameOut    : ��
// Return       : ��
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
       	OSTaskSemPend((OS_TICK)UART_WAIT_TIME,                                  //�ȴ�1s
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
        else if (os_err == OS_ERR_TIMEOUT)                                     //�������ݳ�ʱ��δ���յ����ݣ�
        {
            UartRunCnt++;                                                       //1s��һ��
            if (0 != AutoUpLoadTimeTransform(ProductPara.cyAutoUpLoadTime))
            {
                if(UartRunCnt >= AutoUpLoadTimeTransform(ProductPara.cyAutoUpLoadTime))
                {   
                    UartRunCnt = 0;
                    if(MB_RTU_MODE ==  ProductPara.cyMbtWorkType)         //0-RTUģʽ
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


