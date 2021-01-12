#include "bsp.h"

/************************************************************************************************************************************************************************
** ��Ȩ��   2015-2025, ��������Ϊ�Ƽ���չ���޹�˾
** �ļ���:  Modbus_Ascii.h
** ����:    �½�
** �汾:    V1.0.0
** ����:    2015-07-10
** ����:    �����㷨
** ����:         
*************************************************************************************************************************************************************************
** �޸���:          No
** �汾:  		
** �޸�����:    No 
** ����:            No
*************************************************************************************************************************************************************************/

extern u8 UART1_RXBuff[MAX_USART1_DATA_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.

extern BitAction UartRecvNewData ;
extern BitAction UartRecvFrameOK ;
extern OS_TCB  AppTaskMBTCB;

u8 cyAsciiBuff[MAX_USART1_DATA_LEN];

static u8 g_cyRevState = ASCII_IDLE_STATE;
static u8 g_cyRevBufffLen = 0;

//**************************************************************************************************
// ����         : MODBUS_ASCII_HexToAscii()
// ��������     : 2015-07-24
// ����         : �½�
// ����         : ʮ��������תASCII��
// �������     : ʮ��������(u8 cyHexData)
// �������     : ASCII��(u8 *pCyAsciiBuf)
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
void MODBUS_ASCII_HexToAscii(u8 cyHexData, u8 *pCyAsciiBuf)
{
    u8 cyTemp;
    
    cyTemp = cyHexData / 16;                                
    if (10 > cyTemp) //0-9
    {
        *(pCyAsciiBuf + 0) = cyTemp + '0';                                  //��4bitsת��������0~9��0x30~0x39��
    }
    else
    {
        *(pCyAsciiBuf + 0) = (cyTemp - 10) + 'A';                           //��4bitsת������ĸA~F(0x41~0x46)
    }

    cyTemp = cyHexData % 16;
    if (10 > cyTemp) //0-9
    {
        *(pCyAsciiBuf + 1) = cyTemp + '0';                                  //��4bitsת��������0~9��0x30~0x39��
    }
    else
    {
        *(pCyAsciiBuf + 1) = (cyTemp - 10) + 'A';                           //��4bitsת������ĸA~F(0x41~0x46)
    }
}


//**************************************************************************************************
// ����         : MODBUS_ASCII_AsciiToHex()
// ��������     : 2015-07-24
// ����         : �½�
// ����         : ASCII��תʮ��������
// �������     : ASCII��(u8 *pCyAsciiBuf)
// �������     : ��
// ���ؽ��     : ʮ��������(u8 cyHexData)
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
u8 MODBUS_ASCII_AsciiToHex(u8 *pCyAsciiBuf)
{
    u8 cyHexData;
    
    cyHexData = 0;
    if ('A' > *(pCyAsciiBuf + 0) ) //0-9
    {
        cyHexData += *(pCyAsciiBuf + 0) - '0';
    }
    else if ('a' > *(pCyAsciiBuf + 0) ) //��д
    {
        cyHexData += *(pCyAsciiBuf + 0) - 'A' + 10;
    }
    else
    {
        cyHexData += *(pCyAsciiBuf + 0) - 'a' + 10;
    }

    cyHexData *= 16;
    
    if ('A' > *(pCyAsciiBuf + 1) ) //0-9
    {
        cyHexData += *(pCyAsciiBuf + 1) - '0';
    }
    else if ('a' > *(pCyAsciiBuf + 1) ) //��д
    {
        cyHexData += *(pCyAsciiBuf + 1) - 'A' + 10;
    }
    else
    {
        cyHexData += *(pCyAsciiBuf + 1) - 'a' + 10;
    }
    
    return (cyHexData);
}

//**************************************************************************************************
// ����         : MODBUS_ASCII_GetLrc()
// ��������     : 2015-07-24
// ����         : �½�
// ����         : ��ȡLRCֵ
// �������     : ASCII�봮(u8 *pCyAsciiBuf), ���ݳ���(u8 cyLen)
// �������     : ��
// ���ؽ��     : LRCֵ(u8 cyLrcVal)
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
u8 MODBUS_ASCII_GetLrc(u8 *pCyAsciiBuf, u8 cyLen)
{
    u8 i;
    u8 cyLrcVal;
    
    if (1 == (cyLen % 2) )
    {
        return 0;
    }
    
    cyLen /= 2;
    cyLrcVal = 0;
    for (i = 0; i < cyLen; i++)
    {
        cyLrcVal += MODBUS_ASCII_AsciiToHex(pCyAsciiBuf + i * 2);
    }
    //����
    cyLrcVal = ~cyLrcVal;
    cyLrcVal += 1;
    
    return (cyLrcVal);
}


//**************************************************************************************************
// ����         : MODBUS_ASCII_AsciiPacketToRtuPacket()
// ��������     : 2015-07-24
// ����         : �½�
// ����         : ASCII���ݰ�ת��RTU���ݰ�
// �������     : ASCII�봮(u8 *pCyAsciiBuf),  ASCII�봮������(u8 cyAsciiLen)
// �������     : RTU�봮(u8 *pCyRtuBuf),
// ���ؽ��     : 0:����������RTU�봮������(u8 cyRtuLen)
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
u8 MODBUS_ASCII_AsciiPacketToRtuPacket(u8 *pCyAsciiBuf, u8 cyAsciiLen, u8 *pCyRtuBuf)
{
    u8 i;
    u8 cyRtuLen;
    
    if (1 == (cyAsciiLen % 2) )                                                 //ASCII���Ȳ���Ϊ����
    {
        return 0;
    }  
    cyRtuLen = cyAsciiLen / 2;                                                  //RTU��ĳ���
    for (i = 0; i < cyRtuLen; i++)
    {
        *(pCyRtuBuf + i) = MODBUS_ASCII_AsciiToHex(pCyAsciiBuf + i * 2);
    }
    return (cyRtuLen);
}

//**************************************************************************************************
// ����         : MODBUS_ASCII_RtuPacketToAsciiPacket()
// ��������     : 2015-07-24
// ����         : �½�
// ����         : RTU���ݰ�ת��ASCII���ݰ�
// �������     : RTU�봮(u8 *pCyRtuBuf),  RTU�봮������(u8 cyRtuLen)
// �������     : ASCII�봮(u8 *pCyAsciiBuf),
// ���ؽ��     : ASCII�봮������(u8 cyAsciiLen)
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************

u8 MODBUS_ASCII_RtuPacketToAsciiPacket(u8 *pCyRtuBuf, u8 cyRtuLen, u8 *pCyAsciiBuf)
{
    u8 i;
    u8 cyAsciiLen;
    
    cyAsciiLen = cyRtuLen * 2;
    for (i = 0; i < cyRtuLen; i++)
    {
        MODBUS_ASCII_HexToAscii( *(pCyRtuBuf + i), pCyAsciiBuf + i * 2);
    }
    return (cyAsciiLen);
}


//**************************************************************************************************
// ����         : MODBUS_ASCII_HandlRevData()
// ��������     : 2015-07-27
// ����         : �½�
// ����         : ASCII�����������
// �������     : ��������(u8 cyRevData)
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************
void MODBUS_ASCII_HandlRevData(u8 cyRevData)//ASCII��    ���մ����ж����ݴ���
{
    OS_ERR  os_err;
	
    switch(g_cyRevState)
    {
      case ASCII_HEAD_STATE:  //0   ͷ״̬
          if (ASCII_HEAD_DATA == cyRevData)  //�����':',��0x3A
          {
              g_cyRevBufffLen = 0;//��Ϊ �����������ֽ�
          }
          else if (0x0D == cyRevData)  //����0x0D  
          {
              g_cyRevState = ASCII_END_STATE;  //��Ϊ����״̬
          }
          
          UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
          g_cyRevBufffLen++;
          
          if (MAX_USART1_DATA_LEN <= g_cyRevBufffLen)// MAX_USART1_DATA_LEN =  255
          {
              g_cyRevState = ASCII_IDLE_STATE;//����״̬
          }
          break;
      
      case ASCII_END_STATE:   //1
          if (ASCII_HEAD_DATA == cyRevData)//���ǡ�������֡ͷ
          {
              g_cyRevBufffLen = 0;
              g_cyRevState = ASCII_HEAD_STATE;//���¿�ʼ���棬����Ϊͷ״̬
              UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
              g_cyRevBufffLen++;
          }
          else if (0x0A == cyRevData)
          {
                g_cyRevState = ASCII_IDLE_STATE;
                
                UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
                g_cyRevBufffLen++;

              //�ɹ�����һ�����ݵı�־
              UartRecvFrameOK = Bit_SET;
                                
              
              
              //���ô˺�����Ϊ����ȴ��źŵ����񷢳��źš�
              OSTaskSemPost((OS_TCB *)&AppTaskMBTCB, (OS_OPT) OS_OPT_POST_NONE,
                    (OS_ERR *)&os_err);
          }
          else
          {
              g_cyRevState = ASCII_IDLE_STATE;//����״̬
          }
          break; 
        
       case ASCII_IDLE_STATE:  
          if (ASCII_HEAD_DATA == cyRevData)//���ǡ�������֡ͷ
          {
              g_cyRevBufffLen = 0;
              g_cyRevState = ASCII_HEAD_STATE;//���¿�ʼ���棬����Ϊͷ״̬
              UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
              g_cyRevBufffLen++;
          }
            break;
        
       default:               
            g_cyRevState = ASCII_IDLE_STATE; //Ĭ��Ϊ����
            break;
    }//end switch
}

//**************************************************************************************************
// ����         : MODBUS_ASCII_CheckAscii()
// ��������     : 2015-07-27
// ����         : �½�
// ����         : �����Ƿ���Ascii��
// �������     : ASCII�봮(u8 *pCyAsciiBuf), ���ݳ���(u8 cyLen)
// �������     : ��
// ���ؽ��     : ���(0 ��ȫ�ǣ� 1 ȫ��)
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
u8 MODBUS_ASCII_CheckAscii(u8 *pCyAsciiBuf, u8 cyLen)
{
    u8 i;
    for (i = 0; i < cyLen; i++)
    {
        if ('0' > *(pCyAsciiBuf + i) )                                          //ֵ����С��31����0����
        {
            break;
        }
        if ( ('9' < *(pCyAsciiBuf + i) ) && ( *(pCyAsciiBuf + i) < 'A' ) )      //ֵ�ڡ�0��~��9������A��~��F��
        {
            break;
        }
        if ( ('F' < *(pCyAsciiBuf + i) ) && ( *(pCyAsciiBuf + i) < 'a' ) )
        {
            break;
        }
        if ('f' < *(pCyAsciiBuf + i) )
        {
            break;
        }
    }
    if (i == cyLen)
    {
        return (1);                                                             //����1ֵΪ������
    }
    return (0);                                                                 //����0��ֵ������
}

//**************************************************************************************************
// ����         : MODBUS_ASCII_RecvData()
// ��������     : 2015-07-27
// ����         : �½�
// ����         : ����һ������
// �������     : ���ݴ�(cyRecvBuff)
// �������     : ���ݳ���(u8 *cyLen)
// ���ؽ��     : ִ�н��(0 û�н������ݣ� 1 �������ݳ��ֲ���ASCII�룬 2 Ч������� 3 �ɹ�)
// ע���˵��   :
// �޸�����     : ���ʹ����LRCʱ����ôӻ���ַΪ0x3A����ΪLRC����ʱֱ���˳���δִ��ASC->RTU
//                ������LRC����ʱ���һ��    2016.09.09      
//**************************************************************************************************

u8 MODBUS_ASCII_RecvData(u8* cyRecvBuff, u8 *pCyLen)
  {
    u8 cyLrc;
	
    if (((u8*)NULL) == cyRecvBuff)
    {
        return 0;
    }

    if ((Bit_RESET == UartRecvFrameOK) || (0 == g_cyRevBufffLen))
    {
        return 0;
    }
    
    UartRecvFrameOK = Bit_RESET;
    
    if (0 == MODBUS_ASCII_CheckAscii(&UART1_RXBuff[1], g_cyRevBufffLen - 3) )
    {
    	return 1;
    }

    cyLrc = MODBUS_ASCII_GetLrc(&UART1_RXBuff[1], g_cyRevBufffLen - 5);         //ȥ��3A��LRC��0d 0a����У����
    if (cyLrc != MODBUS_ASCII_AsciiToHex(&UART1_RXBuff[g_cyRevBufffLen - 4]) )  //�Ƚ�У����
    {                                                                           //���������䣬�ڷ���LRC����ʱ��֤���յ������ݱ�ת����RTU��ʽ                                                                      
    	return 2;                                                               //��������Ч�����
    }

    *pCyLen = MODBUS_ASCII_AsciiPacketToRtuPacket(&UART1_RXBuff[1], g_cyRevBufffLen - 5, cyRecvBuff);

    return (3);
}


//**************************************************************************************************
// ����         : MODBUS_ASCII_SendData()
// ��������     : 2015-07-13
// ����         : �½�
// ����         : ����һ������
// �������     : ���ݴ�(u8 *cySendBuff), ���ݳ���(cyLen) (���ݳ��� С�� 123)
// �������     : ��
// ���ؽ��     : ִ�н��(0 ʧ�ܣ� ���ݳ��� �ɹ�)
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************
u8 MODBUS_ASCII_SendData(u8 *cySendBuff, u8 cyLen)
{
    u8 cyLrc;
    u8 cyAsciiLen;
	
    if ( (0 == cyLen) || ( ((u8*)NULL) == cySendBuff) )
    {
        return 0;
    }
    
    if ( (cyLen * 2 + 5) > MAX_USART1_DATA_LEN)
    {
    	return 0;
    }
    
    cyAsciiBuff[0] = ASCII_HEAD_DATA;
    cyAsciiLen = 1;
    
    cyAsciiLen += MODBUS_ASCII_RtuPacketToAsciiPacket(cySendBuff, cyLen, &cyAsciiBuff[1]);
    cyLrc = MODBUS_ASCII_GetLrc(&cyAsciiBuff[1], cyAsciiLen - 1);
    MODBUS_ASCII_HexToAscii(cyLrc, &cyAsciiBuff[cyAsciiLen]);
    cyAsciiLen += 2;
    cyAsciiBuff[cyAsciiLen] = 0x0D;
    cyAsciiLen++;
    cyAsciiBuff[cyAsciiLen] = 0x0A;
    cyAsciiLen++;
    return (UART1_SendData(cyAsciiBuff, cyAsciiLen) );
}
