#include "BSP.h"
#include "os_cfg_app.h"

const UartCommTypeDef Uart3Config = 
{
    0,      //SlaveAddr
    1,      //BaudRate
    8,      //WordLength
    1,      //StopBits
    0,      //Parity
    0,      //OutputMode
    1,      //Echo
    1,      //RTU_1OrASCII_0
};

//const u8 TestSendNoEcho[] = {0x02, 'N', 'o', ' ', 'e', 'c', 'h', 'o', '!',0x0d, 0x0a};
//const u8 TestSendGetEcho[] = {0x02, 'G', 'e', 't', ' ', 'e', 'c', 'h', 'o', '!',0x0d, 0x0a};
//const u8 TestSendRequest[] = {0x02, 'R', 'e', 'q', 'u', 'e', 's', 't', '!', 0x0d, 0x0a};

u8 SendRequest[16];

#define UART3_RECV   64
u8 UART3_Recv[UART3_RECV];

u8 CalcuCrc(u8* pRecvBuf, s32 slCRCLength)
{
    u16 usTemp = 0;
    u32 i;

    for(i = 0; i < slCRCLength; i++)
    {
        usTemp += pRecvBuf[i];
    }

    usTemp &= 0xff;

    return usTemp;
}

void AddCrcToFrame(u8* pBuf, s32 slNum)
{
    u8 u8crc;
    
    u8crc = CalcuCrc(pBuf, slNum);

    if((u8crc / 16) > 9)
        pBuf[slNum] = (u8crc / 16) - 10 + 'A';
    else
        pBuf[slNum] = (u8crc / 16) + '0';

    if((u8crc % 16) > 9)
        pBuf[slNum + 1] = (u8crc % 16) - 10 + 'A';
    else
        pBuf[slNum + 1] = (u8crc % 16) + '0';
    
    pBuf[slNum + 2] = 0x0D;
    pBuf[slNum + 3] = 0x0A;
}

BitAction CheckCrc(u8* pRecvBuf, s32 slRecvNum)
{
    u8 ucCRC = CalcuCrc(pRecvBuf, slRecvNum - 4);
    u8 ucCRCHi = CONVERTHEX(pRecvBuf[slRecvNum - 4]);
    u8 ucCRCLo = CONVERTHEX(pRecvBuf[slRecvNum - 3]);

    return (ucCRC == (ucCRCHi * 16 + ucCRCLo)) ? Bit_SET : Bit_RESET;
}


#define MASTER_DELAY OS_CFG_TICK_RATE_HZ

void App_Uart_Task(void *p_arg)
{
    OS_ERR Err;
    s32 slRecvNum;
    s32 slData;
    
    Uart3_Init(&Uart3Config);
    
    while(1)
    {
        OSTaskSemPend(MASTER_DELAY, (OS_OPT)OS_OPT_PEND_BLOCKING, (CPU_TS *)0, (OS_ERR *)&Err);

        //OSTimeDly(1000, OS_OPT_TIME_TIMEOUT, (OS_ERR *)&Err);
        
#if IWDG_EN > 0
        IWDG_ReloadCounter();
#endif

        slRecvNum = UART3_RecvData(UART3_Recv, UART3_RECV);
        if(16 == slRecvNum)
        {
            // "*CFV0100FA32B6"
            if  (('*' == UART3_Recv[0]) && (0x0d == UART3_Recv[14]) && (0x0a == UART3_Recv[15])
              && ('C' == UART3_Recv[1]) && ('F' == UART3_Recv[2]) && ('V' == UART3_Recv[3])
              && (((ProductPara.Uart3Addr / 10) + '0') == UART3_Recv[4]) 
              && (((ProductPara.Uart3Addr % 10) + '0') == UART3_Recv[5]) )
            {
                if(Bit_SET == CheckCrc(UART3_Recv, slRecvNum))
                {
                    UART3_Recv[6] = CONVERTHEX(UART3_Recv[6]);
                    UART3_Recv[7] = CONVERTHEX(UART3_Recv[7]);
                    UART3_Recv[8] = CONVERTHEX(UART3_Recv[8]);
                    UART3_Recv[9] = CONVERTHEX(UART3_Recv[9]);
                    UART3_Recv[10] = CONVERTHEX(UART3_Recv[10]);
                    UART3_Recv[11] = CONVERTHEX(UART3_Recv[11]);

                    slData = UART3_Recv[6];
                    slData *= 16;
                    slData += UART3_Recv[7];
                    slData *= 16;
                    slData += UART3_Recv[8];
                    slData *= 16;
                    slData += UART3_Recv[9];
                    slData *= 16;
                    slData += UART3_Recv[10];
                    slData *= 16;
                    slData += UART3_Recv[11];
                    
                    RunVar.OilHeight = slData;
                }
            }
        }
            
        SendReadChangRunCmd();
        /**
        if (OS_ERR_NONE == Err) //接收到命令返回
        {
            slRecvNum = UART3_RecvData(UART3_Recv, UART3_RECV);
            if(16 == slRecvNum)
            {
                // "*CFV0100FA32B6"
                if  (('*' == UART3_Recv[0]) && (0x0d == UART3_Recv[14]) && (0x0a == UART3_Recv[15])
                  && ('C' == UART3_Recv[1]) && ('F' == UART3_Recv[2]) && ('V' == UART3_Recv[3])
                  && (((ProductPara.Uart3Addr / 10) + '0') == UART3_Recv[4]) 
                  && (((ProductPara.Uart3Addr % 10) + '0') == UART3_Recv[5]) )
                {
                    if(Bit_SET == CheckCrc(UART3_Recv, slRecvNum))
                    {
                        UART3_Recv[6] = CONVERTHEX(UART3_Recv[6]);
                        UART3_Recv[7] = CONVERTHEX(UART3_Recv[7]);
                        UART3_Recv[8] = CONVERTHEX(UART3_Recv[8]);
                        UART3_Recv[9] = CONVERTHEX(UART3_Recv[9]);
                        UART3_Recv[10] = CONVERTHEX(UART3_Recv[10]);
                        UART3_Recv[11] = CONVERTHEX(UART3_Recv[11]);

                        slData = UART3_Recv[6];
                        slData *= 16;
                        slData += UART3_Recv[7];
                        slData *= 16;
                        slData += UART3_Recv[8];
                        slData *= 16;
                        slData += UART3_Recv[9];
                        slData *= 16;
                        slData += UART3_Recv[10];
                        slData *= 16;
                        slData += UART3_Recv[11];
                        
                        RunVar.OilHeight = slData;
                    }
                }
            }
            
            //OSTimeDly(UARTMASTER_WAIT_TIME - 100, OS_OPT_TIME_TIMEOUT, &Err);

            //SendReadChangRunCmd();
        }
        else if (OS_ERR_TIMEOUT == Err) //超时没有返回
        {
            //UART3_ClearRecvBufAndOpenRecv();

            SendReadChangRunCmd();
        }
        */
    }
}


