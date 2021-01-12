#include "bsp.h"

/************************************************************************************************************************************************************************
** 版权：   2015-2025, 深圳市信为科技发展有限公司
** 文件名:  Modbus_Ascii.h
** 作者:    陈锦
** 版本:    V1.0.0
** 日期:    2015-07-10
** 描述:    各种算法
** 功能:         
*************************************************************************************************************************************************************************
** 修改者:          No
** 版本:  		
** 修改内容:    No 
** 日期:            No
*************************************************************************************************************************************************************************/

extern u8 UART1_RXBuff[MAX_USART1_DATA_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

extern BitAction UartRecvNewData ;
extern BitAction UartRecvFrameOK ;
extern OS_TCB  AppTaskMBTCB;

u8 cyAsciiBuff[MAX_USART1_DATA_LEN];

static u8 g_cyRevState = ASCII_IDLE_STATE;
static u8 g_cyRevBufffLen = 0;

//**************************************************************************************************
// 名称         : MODBUS_ASCII_HexToAscii()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : 十六进制数转ASCII码
// 输入参数     : 十六进制数(u8 cyHexData)
// 输出参数     : ASCII码(u8 *pCyAsciiBuf)
// 返回结果     : 无
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
void MODBUS_ASCII_HexToAscii(u8 cyHexData, u8 *pCyAsciiBuf)
{
    u8 cyTemp;
    
    cyTemp = cyHexData / 16;                                
    if (10 > cyTemp) //0-9
    {
        *(pCyAsciiBuf + 0) = cyTemp + '0';                                  //高4bits转换成数字0~9（0x30~0x39）
    }
    else
    {
        *(pCyAsciiBuf + 0) = (cyTemp - 10) + 'A';                           //低4bits转换成字母A~F(0x41~0x46)
    }

    cyTemp = cyHexData % 16;
    if (10 > cyTemp) //0-9
    {
        *(pCyAsciiBuf + 1) = cyTemp + '0';                                  //高4bits转换成数字0~9（0x30~0x39）
    }
    else
    {
        *(pCyAsciiBuf + 1) = (cyTemp - 10) + 'A';                           //低4bits转换成字母A~F(0x41~0x46)
    }
}


//**************************************************************************************************
// 名称         : MODBUS_ASCII_AsciiToHex()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : ASCII码转十六进制数
// 输入参数     : ASCII码(u8 *pCyAsciiBuf)
// 输出参数     : 无
// 返回结果     : 十六进制数(u8 cyHexData)
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
u8 MODBUS_ASCII_AsciiToHex(u8 *pCyAsciiBuf)
{
    u8 cyHexData;
    
    cyHexData = 0;
    if ('A' > *(pCyAsciiBuf + 0) ) //0-9
    {
        cyHexData += *(pCyAsciiBuf + 0) - '0';
    }
    else if ('a' > *(pCyAsciiBuf + 0) ) //大写
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
    else if ('a' > *(pCyAsciiBuf + 1) ) //大写
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
// 名称         : MODBUS_ASCII_GetLrc()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : 获取LRC值
// 输入参数     : ASCII码串(u8 *pCyAsciiBuf), 数据长度(u8 cyLen)
// 输出参数     : 无
// 返回结果     : LRC值(u8 cyLrcVal)
// 注意和说明   : 
// 修改内容     :
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
    //求补码
    cyLrcVal = ~cyLrcVal;
    cyLrcVal += 1;
    
    return (cyLrcVal);
}


//**************************************************************************************************
// 名称         : MODBUS_ASCII_AsciiPacketToRtuPacket()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : ASCII数据包转成RTU数据包
// 输入参数     : ASCII码串(u8 *pCyAsciiBuf),  ASCII码串包长度(u8 cyAsciiLen)
// 输出参数     : RTU码串(u8 *pCyRtuBuf),
// 返回结果     : 0:错误；其他：RTU码串包长度(u8 cyRtuLen)
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
u8 MODBUS_ASCII_AsciiPacketToRtuPacket(u8 *pCyAsciiBuf, u8 cyAsciiLen, u8 *pCyRtuBuf)
{
    u8 i;
    u8 cyRtuLen;
    
    if (1 == (cyAsciiLen % 2) )                                                 //ASCII长度不能为奇数
    {
        return 0;
    }  
    cyRtuLen = cyAsciiLen / 2;                                                  //RTU码的长度
    for (i = 0; i < cyRtuLen; i++)
    {
        *(pCyRtuBuf + i) = MODBUS_ASCII_AsciiToHex(pCyAsciiBuf + i * 2);
    }
    return (cyRtuLen);
}

//**************************************************************************************************
// 名称         : MODBUS_ASCII_RtuPacketToAsciiPacket()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : RTU数据包转成ASCII数据包
// 输入参数     : RTU码串(u8 *pCyRtuBuf),  RTU码串包长度(u8 cyRtuLen)
// 输出参数     : ASCII码串(u8 *pCyAsciiBuf),
// 返回结果     : ASCII码串包长度(u8 cyAsciiLen)
// 注意和说明   : 
// 修改内容     :
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
// 名称         : MODBUS_ASCII_HandlRevData()
// 创建日期     : 2015-07-27
// 作者         : 陈锦
// 功能         : ASCII处理接收数据
// 输入参数     : 接收数据(u8 cyRevData)
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************
void MODBUS_ASCII_HandlRevData(u8 cyRevData)//ASCII码    接收串口中断数据处理
{
    OS_ERR  os_err;
	
    switch(g_cyRevState)
    {
      case ASCII_HEAD_STATE:  //0   头状态
          if (ASCII_HEAD_DATA == cyRevData)  //如果是':',即0x3A
          {
              g_cyRevBufffLen = 0;//记为 接收数据首字节
          }
          else if (0x0D == cyRevData)  //若是0x0D  
          {
              g_cyRevState = ASCII_END_STATE;  //变为结束状态
          }
          
          UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
          g_cyRevBufffLen++;
          
          if (MAX_USART1_DATA_LEN <= g_cyRevBufffLen)// MAX_USART1_DATA_LEN =  255
          {
              g_cyRevState = ASCII_IDLE_STATE;//空闲状态
          }
          break;
      
      case ASCII_END_STATE:   //1
          if (ASCII_HEAD_DATA == cyRevData)//若是‘：’，帧头
          {
              g_cyRevBufffLen = 0;
              g_cyRevState = ASCII_HEAD_STATE;//重新开始保存，并变为头状态
              UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
              g_cyRevBufffLen++;
          }
          else if (0x0A == cyRevData)
          {
                g_cyRevState = ASCII_IDLE_STATE;
                
                UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
                g_cyRevBufffLen++;

              //成功接收一包数据的标志
              UartRecvFrameOK = Bit_SET;
                                
              
              
              //调用此函数是为了向等待信号的任务发出信号。
              OSTaskSemPost((OS_TCB *)&AppTaskMBTCB, (OS_OPT) OS_OPT_POST_NONE,
                    (OS_ERR *)&os_err);
          }
          else
          {
              g_cyRevState = ASCII_IDLE_STATE;//空闲状态
          }
          break; 
        
       case ASCII_IDLE_STATE:  
          if (ASCII_HEAD_DATA == cyRevData)//若是‘：’，帧头
          {
              g_cyRevBufffLen = 0;
              g_cyRevState = ASCII_HEAD_STATE;//重新开始保存，并变为头状态
              UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
              g_cyRevBufffLen++;
          }
            break;
        
       default:               
            g_cyRevState = ASCII_IDLE_STATE; //默认为空闲
            break;
    }//end switch
}

//**************************************************************************************************
// 名称         : MODBUS_ASCII_CheckAscii()
// 创建日期     : 2015-07-27
// 作者         : 陈锦
// 功能         : 检验是否都是Ascii码
// 输入参数     : ASCII码串(u8 *pCyAsciiBuf), 数据长度(u8 cyLen)
// 输出参数     : 无
// 返回结果     : 检测(0 不全是， 1 全是)
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
u8 MODBUS_ASCII_CheckAscii(u8 *pCyAsciiBuf, u8 cyLen)
{
    u8 i;
    for (i = 0; i < cyLen; i++)
    {
        if ('0' > *(pCyAsciiBuf + i) )                                          //值不能小于31（‘0’）
        {
            break;
        }
        if ( ('9' < *(pCyAsciiBuf + i) ) && ( *(pCyAsciiBuf + i) < 'A' ) )      //值在‘0’~‘9’，‘A’~‘F’
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
        return (1);                                                             //返回1值为正常的
    }
    return (0);                                                                 //返回0，值不正常
}

//**************************************************************************************************
// 名称         : MODBUS_ASCII_RecvData()
// 创建日期     : 2015-07-27
// 作者         : 陈锦
// 功能         : 接收一串数据
// 输入参数     : 数据串(cyRecvBuff)
// 输出参数     : 数据长度(u8 *cyLen)
// 返回结果     : 执行结果(0 没有接收数据， 1 接收数据出现不是ASCII码， 2 效验码错误， 3 成功)
// 注意和说明   :
// 修改内容     : 发送错误的LRC时，获得从机地址为0x3A，因为LRC错误时直接退出，未执行ASC->RTU
//                所以在LRC错误时添加一句    2016.09.09      
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

    cyLrc = MODBUS_ASCII_GetLrc(&UART1_RXBuff[1], g_cyRevBufffLen - 5);         //去掉3A、LRC和0d 0a后求校验码
    if (cyLrc != MODBUS_ASCII_AsciiToHex(&UART1_RXBuff[g_cyRevBufffLen - 4]) )  //比较校验码
    {                                                                           //添加以下这句，在发送LRC错误时保证接收到的数据被转换成RTU格式                                                                      
    	return 2;                                                               //发送数据效验错误
    }

    *pCyLen = MODBUS_ASCII_AsciiPacketToRtuPacket(&UART1_RXBuff[1], g_cyRevBufffLen - 5, cyRecvBuff);

    return (3);
}


//**************************************************************************************************
// 名称         : MODBUS_ASCII_SendData()
// 创建日期     : 2015-07-13
// 作者         : 陈锦
// 功能         : 发送一串数据
// 输入参数     : 数据串(u8 *cySendBuff), 数据长度(cyLen) (数据长度 小于 123)
// 输出参数     : 无
// 返回结果     : 执行结果(0 失败， 数据长度 成功)
// 注意和说明   :
// 修改内容     :
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
