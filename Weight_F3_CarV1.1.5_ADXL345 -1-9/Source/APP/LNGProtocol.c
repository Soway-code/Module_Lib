
#include "bsp.h"
#include "math.h"


extern u8 RcvDataBuf[64];
extern u8 SendBuf[DATAPACKET_LEN];   //存储从机响应消息帧
extern u8 SendLen;

void RespondLNG(void)
{
/*
数据包个事范例：
询问帧：AA A1 52 41 00 00 00 B2 A5
起始字节AA 第二字节为变送器编号 （A1为1号，A2为2号，A3为3号，A4为4号）。
回复帧：A1 13 57 00 00 00 00 11 B2 15 68 01 C2 7E AB

A1 // 第一字节A1为变送器编号（同询问）
00 00  // 第二第三字节为一个字  13 57 表示当前电容值 495.1PF
00 00 00 00 
11 B2  // 第八第九字节为一个字  11 B2表示空电容值为  453PF
15 68  // 第十第十一字节为一个字15 68表示满电容值为 548PF
01 C2  // 容量

7E   // xor :A1 00 00 00 00 00 00 11 B2 15 68 01
AB  // end 

*/
    u8 u8Crc;
    u32 i;
    
    SendLen = 0;
    SendBuf[SendLen++] = 0xA0 + UartComm.SlaveAddr;
    SendBuf[SendLen++] = ((RunVar.LiquidHeight >> 8) & 0xff);
    SendBuf[SendLen++] = (RunVar.LiquidHeight & 0xff);

    SendBuf[SendLen++] = 0x0;
    SendBuf[SendLen++] = 0x0;
    SendBuf[SendLen++] = 0x0;
    SendBuf[SendLen++] = 0x0;

    SendBuf[SendLen++] = 0x0;   //0x0;
    SendBuf[SendLen++] = 0x0;

    SendBuf[SendLen++] = 0x10;  //0x1000
    SendBuf[SendLen++] = 0x0;

    SendBuf[SendLen++] = 0x1;
    SendBuf[SendLen++] = 0xC2;
    
    u8Crc = 0;
    for(i = 0; i < (SendLen - 1); i++)
    {
        u8Crc ^= SendBuf[i];
    }
    
    SendBuf[SendLen++] = u8Crc;
    SendBuf[SendLen++] = 0xAB;

    UART1_SendData(SendBuf, SendLen);
}

BitAction LNGProtocol(s32 slRevLength, u8 *u8Msg)
{
/*
数据包个事范例：
询问帧：AA A1 52 41 00 00 00 B2 A5
起始字节AA 第二字节为变送器编号 （A1为1号，A2为2号，A3为3号，A4为4号）。
回复帧：A1 13 57 00 00 00 00 11 B2 15 68 01 C2 7E AB

A1 // 第一字节A1为变送器编号（同询问）
00 00  // 第二第三字节为一个字  13 57 表示当前电容值 495.1PF
00 00 00 00 
11 B2  // 第八第九字节为一个字  11 B2表示空电容值为  453PF
15 68  // 第十第十一字节为一个字15 68表示满电容值为 548PF
01 C2  // 容量

7E   // xor :A1 00 00 00 00 00 00 11 B2 15 68 01
AB  // end 

*/
    if  ((9 == slRevLength) && (0xAA == u8Msg[0]) 
        && ((0xA0 + UartComm.SlaveAddr)==u8Msg[1]))
    {
        RespondLNG();

        return Bit_SET;
    }

    return Bit_RESET;
}

