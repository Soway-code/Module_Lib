
#include "bsp.h"
#include "math.h"


extern u8 RcvDataBuf[64];
extern u8 SendBuf[DATAPACKET_LEN];   //�洢�ӻ���Ӧ��Ϣ֡
extern u8 SendLen;

void RespondLNG(void)
{
/*
���ݰ����·�����
ѯ��֡��AA A1 52 41 00 00 00 B2 A5
��ʼ�ֽ�AA �ڶ��ֽ�Ϊ��������� ��A1Ϊ1�ţ�A2Ϊ2�ţ�A3Ϊ3�ţ�A4Ϊ4�ţ���
�ظ�֡��A1 13 57 00 00 00 00 11 B2 15 68 01 C2 7E AB

A1 // ��һ�ֽ�A1Ϊ��������ţ�ͬѯ�ʣ�
00 00  // �ڶ������ֽ�Ϊһ����  13 57 ��ʾ��ǰ����ֵ 495.1PF
00 00 00 00 
11 B2  // �ڰ˵ھ��ֽ�Ϊһ����  11 B2��ʾ�յ���ֵΪ  453PF
15 68  // ��ʮ��ʮһ�ֽ�Ϊһ����15 68��ʾ������ֵΪ 548PF
01 C2  // ����

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
���ݰ����·�����
ѯ��֡��AA A1 52 41 00 00 00 B2 A5
��ʼ�ֽ�AA �ڶ��ֽ�Ϊ��������� ��A1Ϊ1�ţ�A2Ϊ2�ţ�A3Ϊ3�ţ�A4Ϊ4�ţ���
�ظ�֡��A1 13 57 00 00 00 00 11 B2 15 68 01 C2 7E AB

A1 // ��һ�ֽ�A1Ϊ��������ţ�ͬѯ�ʣ�
00 00  // �ڶ������ֽ�Ϊһ����  13 57 ��ʾ��ǰ����ֵ 495.1PF
00 00 00 00 
11 B2  // �ڰ˵ھ��ֽ�Ϊһ����  11 B2��ʾ�յ���ֵΪ  453PF
15 68  // ��ʮ��ʮһ�ֽ�Ϊһ����15 68��ʾ������ֵΪ 548PF
01 C2  // ����

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

