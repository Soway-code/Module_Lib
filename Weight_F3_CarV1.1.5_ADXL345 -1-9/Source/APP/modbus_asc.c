#include "bsp.h"

/************************************************************************************************************************************************************************
** ��Ȩ��   2016-2026, ��������Ϊ�Ƽ���չ���޹�˾
** �ļ���:  modbus_asc.c
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

extern u8 SendLen;
u8 SendBuf[DATAPACKET_LEN];
extern u8 UART1_RXBuff[MAX_USART1_DATA_LEN];
extern u8 Dev_Id[DEFAULT_DEV_ID_LEN];
extern u8 u8SendNum;

extern u16 g_wFADValue;
extern u16 g_wRADValue;

extern u16 Speed;
u8 Acc_Flag;
u8 Acc_Flag_Cnt = 0;

extern unsigned char	ADXL345_BUF[12];
u32 AD_Buff[5][10];                                                             //ADֵ����
u32 Weight_Buff[5][10];                                                         //����ֵ����
//**************************************************************************************************
// ����         : MBASC_GetSlaveAddr()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : ��ȡ�ӻ���ַ
// �������     : ֡������(u8 *u8Msg)
// �������     : ��
// ���ؽ��     : �ӻ���ַ
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
u8 MBASC_GetSlaveAddr(u8 *u8Msg)
{
    return u8Msg[0];
}

//**************************************************************************************************
// ����         : MBASC_GetFunCode()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : ��ȡ������
// �������     : ֡������(u8 *u8Msg)
// �������     : ��
// ���ؽ��     : ������
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
u8 MBASC_GetFunCode(u8 *u8Msg)
{
    return u8Msg[1];
}

//**************************************************************************************************
// ����         : MBASC_SendMsg()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : ��ASCII��ʽ������Ϣ
// �������     : ֡������(u8 *u8Msg),֡��(u8 u8MsgLen)
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
void MBASC_SendMsg(u8 *u8Msg, u8 u8MsgLen)
{
  
      Delay_Ms(45);  //��������ʱ����50ms��ʱ  20190905
  
    if((MB_ADDRESS_BROADCAST != MBASC_GetSlaveAddr(u8Msg)) && (UartComm.Echo == ECHO_ENABLE))
    {
        MODBUS_ASCII_SendData(u8Msg, u8MsgLen);
    }
}

//**************************************************************************************************
// ����         : MBASC_SendMsg_NoLimit()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : �������Ե���ASCII��ʽ������Ϣ
// �������     : ֡������(u8 *u8Msg),֡��(u8 u8MsgLen)
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
void MBASC_SendMsg_NoLimit(u8 *u8Msg, u8 u8MsgLen)
{
      Delay_Ms(45); //��������ʱ����50ms��ʱ  20190905
   
   
    if(UartComm.Echo == ECHO_ENABLE)
    {
        MODBUS_ASCII_SendData(u8Msg, u8MsgLen);
    }
}

//**************************************************************************************************
// ����         : MBASC_SendErr()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : ���ʹ��������Ӧ֡
// �������     : ������(u8 ErrCode)
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
void MBASC_SendErr(u8 ErrCode)
{
    SendLen = 1;
    SendBuf[SendLen++] |= 0x80;
    SendBuf[SendLen++] = ErrCode;
    MBASC_SendMsg(SendBuf, SendLen);
}


//**************************************************************************************************
// ����         : MBASC_Fun03()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : 03�����룬�������Ĵ�������
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     :  ���ӻ���ַ�޸�Ϊ0x45����Ӧ�ļĴ�����ַ�޸�Ϊ0x45xx   2016.09.08
//**************************************************************************************************
void MBASC_Fun03(void)
{
    u8 i;
    u16 Data_Buf;
    u16 ReadAdr = 0;
    u16 Register_Num = (u16)UART1_RXBuff[4] * 256 + UART1_RXBuff[5];
    
    u8 uBuf[2];
    u16 s_TotalPacks, s_CurrentPack;
    
    if(UART1_RXBuff[2] - UartComm.SlaveAddr !=0)                                //�޸ĳ�0x71����Ĵӻ���ַҲ�ʺ�
        ReadAdr = (u16)UART1_RXBuff[2] * 256 + UART1_RXBuff[3];
    else
        ReadAdr = 0x71 * 256 + UART1_RXBuff[3];
    
    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00;
    SendBuf[SendLen++] = MB_FUNC_READ_HOLDING_REGISTER;	                        
    SendBuf[SendLen++] = Register_Num * 2;		                        
                                                                                
    if (!(((ReadAdr >= MBASC_HOLDING_REG_REGION1_BGEIN) && (ReadAdr <= MBASC_HOLDING_REG_REGION1_END)
      && (ReadAdr + Register_Num <= (MBASC_HOLDING_REG_REGION1_END + 1)))
      || ((ReadAdr == MBASC_HOLDING_REG_REGION2_BGEIN) 
      || ((ReadAdr >= MBASC_HOLDING_REG_REGION3_BGEIN) && (ReadAdr <= MBASC_HOLDING_REG_REGION3_END)
      && (ReadAdr + Register_Num <= (MBASC_HOLDING_REG_REGION3_END + 1)))
     || (ReadAdr == 0x7178) || (ReadAdr == MBASC_HOLDING_REG_REGION4_BGEIN))    
      && (0 != Register_Num)))
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

    for (u8 k = 0; k < Register_Num; ReadAdr++, k++)
    {
        switch (ReadAdr)
        {
        case 0x7130:                                                            //�豸��ַ
            Data_Buf = UartComm.SlaveAddr;				                
            break;

        case 0x7131:                                                            //������
            Data_Buf = UartComm.BaudRate;				                
            break;

        case 0x7132:                                                            //��żУ��
            Data_Buf = UartComm.Parity;			                        
            break;

        case 0x7133:                                                            //����
            Data_Buf = 0;						                
            break;

        case 0x7134:                                                            //����ʹ�ܣ��ޣ�
            Data_Buf = ProductPara.OffsetEn;						        
            break;

        case 0x7135:                                                            //�˲��ȼ���ϵ����
            Data_Buf = ProductPara.cyFilterLevel;			                
            break;

        case 0x7136:                                                            //�Զ��ϴ�ʱ��                                              
            Data_Buf = ProductPara.cyAutoUpLoadTime;                            //�����������Ǵ��룬����ʵ������
            break;

        case 0x7137:                                                            //����ϵ��K���ޣ�
            Data_Buf = (u32)(UserParam.UserFlowK * 100);                            
            break;

        case 0x7138:                                                            //����ϵ��B���ޣ�
            Data_Buf = (u32)(UserParam.UserFlowB + 100);                            
            break;

        case 0x7139:                                                            
            Data_Buf = 0;                            
            break;
            
        case 0x713A:                                                            
            Data_Buf = 0;                            
            break;
            
        case 0x713B:                                                            
            Data_Buf = 0;                            
            break;

        case 0x713C:                                                            
            Data_Buf = 0;                            
            break;

        case 0x713D:                                                            
            Data_Buf = RunVar.Weight_Unit;                            
            break;

        case 0x713E:                                                            
            Data_Buf = 0;                            
            break;
            
        case 0x713F:                                                            //��������/�غ�����
            Data_Buf = RunVar.VehicleWeight;                                       
            break;

        case 0x7140:                                                            //���ط�ֵ                
            Data_Buf = RunVar.OverloadLimit;			                
            break;

        case 0x7141:                                                            //���ط�ֵƫ��
            Data_Buf = RunVar.OverloadLimitDevation;
            break;

        case 0x7142:                                                            //���ز�������
            Data_Buf = RunVar.LoadMeasureScheme;
            break;

        case 0x7143:                                                            //���ط�ֵ
            Data_Buf = RunVar.LoadLimit;
            break;

        case 0x7144:                                                            //���ط�ֵƫ��
            Data_Buf = RunVar.LoadLimitDevation;
            break;

        case 0x7145:                                                            //���ط�ֵ
            Data_Buf = RunVar.EmptyLimit;
            break;

        case 0x7146:                                                            //���ط�ֵƫ��
            Data_Buf = RunVar.EmptyLimitDevation;
            break;

        case 0x7147:                                                              
            Data_Buf = RunVar.LightLimit;                                       //������ֵ
            break;

        case 0x7148:                                                              
            Data_Buf = RunVar.LightLimitDevation;                               //������ֵƫ��
            break;

        case 0x7149:                                                              
            Data_Buf = 0;
            break;

        case 0x714A:                                                             
            Data_Buf = 0;
            break;

        case 0x714B:                                                             
            Data_Buf = 0;
            break;

        case 0x714C:                                                             
            Data_Buf = 0;
            break;

        case 0x714D:                                                             
            Data_Buf = 0;
            break;

        case 0x714E:                                                             
            Data_Buf = 0;
            break;

        case 0x714F:                                                             
            Data_Buf = 0;
            break;            

        case 0x7160:                                                            //���ģʽ 0-RTU��1-ASCII
            Data_Buf = ProductPara.cyMbtWorkType;				        
            break;
            
        case 0x7170:                                                            
            Data_Buf = (u32)Speed;				        
            break; 
            
        case 0x7171:                                                            
            Data_Buf = (u32)Acc_Flag;				        
            break;

        case 0x7172:                                                            
            Data_Buf = 0;				        
            break;

        case 0x7173:                                                            
            Data_Buf = 0;				        
            break;      
            
        case 0x7178:     //��ȡ�궨�ܰ���
                     
            I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_PACKS, 2, uBuf); //д����ձ�
            //Data_Buf = (u16)uBuf[0] * 256 + uBuf[1];
            Data_Buf = ArrayToUshort(uBuf);
            break;
            
        case 0x7180:                                                            //��ȡ �궨�� ���50��ԭʼADֵ�ͳ����غ����� 
            s_TotalPacks = (u16)(UART1_RXBuff[7] * 256 + UART1_RXBuff[8]);
            s_CurrentPack = (u16)(UART1_RXBuff[9] * 256 + UART1_RXBuff[10]);
            if((s_TotalPacks > 5) || (s_CurrentPack > 4) || (s_CurrentPack >= s_TotalPacks)
               || (UART1_RXBuff[6] != Register_Num * 2)/* || (s_CurrentPack >= RunVar.TotalPacks)*/)
            {
                MBASC_SendErr(MB_EX_ILLEGAL_DATA_VALUE);
                return;
            }
            I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_ADDR(s_CurrentPack), 80, SendBuf + 11);
            memcpy(SendBuf, UART1_RXBuff, 11);
            MBASC_SendMsg(SendBuf, 91);
           
            return;
            
        default:
            break;
        }

        for (i = 2; i > 0; i--)
        {
            SendBuf[SendLen++] = (u8)(Data_Buf >> ((i - 1) * 8));
        }
    }
    
    MBASC_SendMsg(SendBuf, SendLen);
}


extern struct OutData myOutData;
extern u16 g_wRADValue;
//**************************************************************************************************
// ����         : MBASC_Fun04()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : 04�����룬��˫���Ĵ�������
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     :  �Ĵ�����ַ��Ϊ0x71xx     2016.09.14
//**************************************************************************************************
void MBASC_Fun04(void)	//��˫���Ĵ���
{
    u8 i;
    u32 Data_Buf;
    u16 ReadAdr = 0;
    u16 Register_Num = (u16)UART1_RXBuff[4] * 256 + UART1_RXBuff[5];

    if(UART1_RXBuff[2] - UartComm.SlaveAddr !=0) 
        ReadAdr = (u16)UART1_RXBuff[2] * 256 + UART1_RXBuff[3];
    else
        ReadAdr = 0x71 * 256 + UART1_RXBuff[3];
    
    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00;
    SendBuf[SendLen++] = MB_FUNC_READ_INPUT_REGISTER;
    SendBuf[SendLen++] = Register_Num * 2;		                        //���ݳ���
                                                                                //�����ȡ��Χ���
    if(!(((ReadAdr >= MBASC_INPUT_REG_REGION1_BGEIN) && (ReadAdr <= MBASC_INPUT_REG_REGION1_END)
        && ((ReadAdr + Register_Num) <= (MBASC_INPUT_REG_REGION1_END + 2)))
        || ((ReadAdr >= MBASC_INPUT_REG_REGION2_BGEIN) && (ReadAdr <= MBASC_INPUT_REG_REGION2_END)
        && (ReadAdr + Register_Num <= (MBASC_INPUT_REG_REGION2_END + 2)))
        || ((ReadAdr >= MBASC_INPUT_REG_REGION3_BGEIN) && (ReadAdr <= MBASC_INPUT_REG_REGION3_END) 
        && (ReadAdr + Register_Num <= (MBASC_INPUT_REG_REGION3_END + 2)))    
        && (0 != Register_Num) && (0 == (Register_Num & 0x01)) && (0 == (ReadAdr & 0x01))))   
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

  if((ReadAdr != 0x7114) && (ReadAdr != 0x7120))
    for (u8 k = 0; k < Register_Num; ReadAdr += 2, k += 2)
    {
        switch (ReadAdr)
        {
        case 0x7100:                                                            //����״̬��01-���أ�02-���أ�03-���أ�04-װ�أ�05-ж��(����)��
            Data_Buf = (u32)(((RunVar.Weight_Unit << 8) & 0xFF00) + (g_cyWorkState & 0xFF));            	                                
            break;

        case 0x7102:                                                            //����
            Data_Buf = (u32)g_cyRecordShovelNum;                                                   	                                        
            break;

        case 0x7104:                                                            //�����غ�����                                                                                   
            Data_Buf = UserParam.CarLoadWeight;		                                
            break;

        case 0x7106:                                                            //�������ֵ
            Data_Buf = UserParam.LoadRelativeValue;
            break;

        case 0x7108:                                                            //ԭʼADֵ
            Data_Buf = (u32)g_wRADValue;	                                				                        //NC
            break;

        case 0x710A:                                                            //�������		                                
//            myOutData = Runing(g_wFADValue); 
            Data_Buf = myOutData.Weight; 					                       
            break;

        case 0x710C:
            Data_Buf = 0;                        
            break;

        case 0x710E:
            Data_Buf = 0;                             
            break;
            
       /*  case 0x7114:  
           Data_Buf = 0x11;                             
            break;
            case 0x7115:  
           Data_Buf = 0x22;                             
            break;
            case 0x7116:  
           Data_Buf = 0x33;                             
            break;
            case 0x7117:  
           Data_Buf = 0x44;                             
            break;
            case 0x7118:  
           Data_Buf = 0x55;                             
            break;
            case 0x7119:  
           Data_Buf = 0x66;                             
            break;
            case 0x711A:  
           Data_Buf = 0x77;                             
            break;
            case 0x711B:  
           Data_Buf = 0x88;                             
            break;
          */
        default:
            break;
        }
        for (i = 4; i > 0; i--)
        {
            SendBuf[SendLen++] = (u8)(Data_Buf >> ((i - 1) * 8));
        }
    }
    else
    {
    
        for (u8 k = 0; k < Register_Num; ReadAdr++, k++)
        {
          switch (ReadAdr)
          {
           
            case 0x7114:  
            SendBuf[2]= Register_Num;
            SendBuf[SendLen++]= (uint16_t)g_wRADValue>>8;  
            SendBuf[SendLen++] = (uint16_t)g_wRADValue & 0x00FF;  
            break;
            
            case 0x7115:  
            SendBuf[SendLen++] = ADXL345_BUF[0];  
             SendBuf[SendLen++] = ADXL345_BUF[1];
            break;
            
            case 0x7116:  
            SendBuf[SendLen++] =ADXL345_BUF[2];
            SendBuf[SendLen++] = ADXL345_BUF[3];
            break;
            
            case 0x7117:         
            SendBuf[SendLen++] = ADXL345_BUF[4];
            SendBuf[SendLen++] = ADXL345_BUF[5]; 
            break;
          
            case 0x7120:  
            SendBuf[2]= Register_Num;
            SendBuf[SendLen++]= (uint16_t)g_wRADValue>>8;  
            SendBuf[SendLen++] = (uint16_t)g_wRADValue & 0x00FF;  
            break;
            
            case 0x7121:  
            SendBuf[SendLen++] = ADXL345_BUF[6];  
            SendBuf[SendLen++] = ADXL345_BUF[7];
            break;
            
            case 0x7122:  
            SendBuf[SendLen++] =ADXL345_BUF[8];
            SendBuf[SendLen++] = ADXL345_BUF[9];
            break;
            
            case 0x7123:         
            SendBuf[SendLen++] = ADXL345_BUF[10];
            SendBuf[SendLen++] = ADXL345_BUF[11]; 
            break;
            
          default:
            break;
          }
        }
    
    
    }
    MBASC_SendMsg(SendBuf, SendLen);
}

//**************************************************************************************************
// ����         : MBASC_Fun05()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : 05�����룬��������Ȧ���𿪹�ʹ������
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : ָ�����3A 34 35 30 35 35 30 35 33 46 46 30 30 31 34 0D 0A  �޸�flashд����ʹ��λΪENABLE
// �޸�����     :
//**************************************************************************************************
void MBASC_Fun05(void)
{
    u16 ReadAdr = 0;
    if(UART1_RXBuff[2] - UartComm.SlaveAddr !=0) 
        ReadAdr = (u16)UART1_RXBuff[2] * 256 + UART1_RXBuff[3];
    else
        ReadAdr = 0x71 * 256 + UART1_RXBuff[3];
    
    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00; 
    SendBuf[SendLen++] = MB_FUNC_WRITE_SINGLE_COIL;
                                                                            
    if ((ReadAdr < MBASC_SINGLE_COIL_ADDR_BGEIN) || (ReadAdr > MBASC_SINGLE_COIL_ADDR_END))
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

    if ((ReadAdr != 0x7153) && (ProductPara.bFlashWEn == FLASH_WR_DISABLE))     //��дflashʧ������£�ֻ�ܶ�0x5053���в������������Ĵ������ܲ���     
    {
        MBASC_SendErr(MB_EX_SLAVE_DEVICE_FAILURE);
        return;
    }

    if (!(((UART1_RXBuff[4] == 0xFF) && (UART1_RXBuff[5] == 0x00))
          || ((UART1_RXBuff[4] == 0x00) && (UART1_RXBuff[5] == 0x00))))
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_VALUE);
        return;
    }

    switch (ReadAdr)
    {
    case 0x7150:                                                                //������ֹͣ����ʵ�����е��������������ж�����״̬
      break;

    case 0x7151:							        //�ָ�����ֵ
      if ((UART1_RXBuff[4] == 0xFF) && (UART1_RXBuff[5] == 0x00))
      {
        Cur_Param[0] = User_Default_Param[0] + 1;
        I2C1_WNBytesMul3T(EEPROM_ADDRESS, RUN_ADDR_BASE, 1, Cur_Param);
        Rd_Dev_Param();
      }
      break;

    case 0x7152:                                                                //�ⶳ�����ᡣ�������ǽ���������ǲ���������
      break;
        
    case 0x7153:                                                                //ʹ��/�ر�flashд����

      if ((UART1_RXBuff[4] == 0xFF) && (UART1_RXBuff[5] == 0x00))
      {
        ProductPara.bFlashWEn = FLASH_WR_ENABLE;
      }
      else
      {
        ProductPara.bFlashWEn = FLASH_WR_DISABLE;
      }
      break;
      
    case 0x7154:                                                                //�������
      if ((UART1_RXBuff[4] == 0xFF) && (UART1_RXBuff[5] == 0x00))
      {
        g_cyWorkState = 0x01;
        RunVar.LoadADValue = 0;
        RunVar.uiAI[0] = 0;
//        Save_Param(0);
      }
      break; 
    }
    MBASC_SendMsg(UART1_RXBuff, 6);
}


//**************************************************************************************************
// ����         : MBASC_Fun10()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : 10�����룬д����Ĵ��������������޸Ĳ���
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     : 1.�Ƚ����Զ��ϴ�ʱ����룬�ٰѱ���ת����ʱ�䣬�����ָ���޸ı��벻�������Զ��ϴ�ʱ
//                �䲻����Ҫ��ʱ������      2016.09.10
//                2.���Ӵ��ڳ�ʼ�����޸Ĳ����ʺ���Բ�������Ƭ����
//**************************************************************************************************
void MBASC_Fun10()
{
  
  // 71 10 71 id  Register_Num  BYTE_Num  data...
    u16 ReadAdr = 0;
    u16 Register_Num = 0;
    u32 index = 0;
    u8 i, j, u8Temp[4];
    u8 uBuf[80];
    
    u16 uTemp;
    
    u16 s_TotalPacks, s_CurrentPack;
  
    BitAction CommChangeFlag = Bit_RESET;
    
    ProductPara.bFlashWEn = FLASH_WR_ENABLE;
    
    Register_Num = (u16)(UART1_RXBuff[4] * 256 + UART1_RXBuff[5]);
    if(UART1_RXBuff[2] - UartComm.SlaveAddr != 0) 
    {
        ReadAdr = (u16)(UART1_RXBuff[2] * 256 + UART1_RXBuff[3]);
    }
    else
    {
        ReadAdr = 0x71 * 256 + UART1_RXBuff[3];
    }
    
    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00;
    SendBuf[SendLen++] = MB_FUNC_WRITE_MULTIPLE_REGISTERS;
    SendBuf[SendLen++] = Register_Num * 2;                
                                                       //�����ȡ��Χ���
    if (!(((ReadAdr >= MBASC_MUL_REG_REGION1_BGEIN) && (ReadAdr <= MBASC_MUL_REG_REGION1_END)
            && (ReadAdr + Register_Num <= (MBASC_MUL_REG_REGION1_END + 1)))
            || ((ReadAdr == MBASC_HOLDING_REG_REGION2_BGEIN)
            || ((ReadAdr >= MBASC_MUL_REG_REGION3_BGEIN) && (ReadAdr <= MBASC_MUL_REG_REGION3_END)
            && (ReadAdr + Register_Num <= (MBASC_MUL_REG_REGION3_END + 1)))
            || (ReadAdr == MBASC_HOLDING_REG_REGION4_BGEIN))
            && (0 != Register_Num)) || (Register_Num * 2 != UART1_RXBuff[6])) //�޸�modbus_asc.c  10������ �Ƿ����ݵ�ַ��MB_EX_ILLEGAL_DATA_ADDRESS�����ж����� 20190628 
    { 
      MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
      return;
    }

    for (u8 k = 0; k < Register_Num; ReadAdr++, k++)
    {
        switch (ReadAdr)
        {
        case 0x7130:						                //�豸��ַ
            UartComm.SlaveAddr = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, SLAVE_ADDR, 1, &UartComm.SlaveAddr);
            index += 2;
            break;

        case 0x7131:                                                            //������
            UartComm.BaudRate = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            if(UartComm.BaudRate <1)
                UartComm.BaudRate = 1;
            if(UartComm.BaudRate > 7)
                UartComm.BaudRate = 7;
              
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, BAUDRATE, 1, &UartComm.BaudRate);
            CommChangeFlag = Bit_SET;
            index += 2;                                               
            break;

        case 0x7132:						                //��żУ��
            UartComm.Parity = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            if((UartComm.Parity <1) || (UartComm.BaudRate >3))
                UartComm.Parity = 3;
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, PARITY, 1, &UartComm.Parity);
            CommChangeFlag = Bit_SET;
            index += 2;
            break;

        case 0x7133:                                                            //����
            index += 2;
            break;

        case 0x7134:                                                            //����ʹ��
            ProductPara.OffsetEn = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            if((ProductPara.OffsetEn != 1) && (ProductPara.OffsetEn != 2))
            {
                ProductPara.OffsetEn = 1;
            }
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, OFFSET_ENABLE, 1, &ProductPara.OffsetEn);
            index += 2;
            break;

        case 0x7135:						                //�˲�ϵ��
            ProductPara.cyFilterLevel = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            if((ProductPara.cyFilterLevel == 0) || (ProductPara.cyFilterLevel > 3))
            {
                ProductPara.cyFilterLevel = 2;
            }
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, FILTER_LEVEL, 1, &ProductPara.cyFilterLevel);
            index += 2;
            break;

        case 0x7136:                                                            //�����޸�ʱ�����Ϳ���ʵ��ʱ����л�����������
            ProductPara.cyAutoUpLoadTime = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            if((ProductPara.cyAutoUpLoadTime < 1) || (ProductPara.cyAutoUpLoadTime >4))
            {
                ProductPara.cyAutoUpLoadTime = 1;
            }
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, AUTO_UPLOAD_TIME, 1, &ProductPara.cyAutoUpLoadTime);
            index += 2;
            break;

        case 0x7137:                                                            //�������ϵ��K
            UserParam.UserFlowK = (UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]) / 100.0;  //����ϵ��K������ֵΪ1~200
            if((UserParam.UserFlowK < 0.01f) || (UserParam.UserFlowK > 2.0f))
            {
                UserParam.UserFlowK = 1.0f;
            }
            floatToHexArray(UserParam.UserFlowK, u8Temp, 4);
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, USER_FLOW_K, 4, u8Temp);
            index += 2;
            break;

        case 0x7138:                                                            //�������ϵ��B
            UserParam.UserFlowB = (UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]) - 100.0;
            if((UserParam.UserFlowB < -100.0f) || (UserParam.UserFlowB > 100.0f))
                UserParam.UserFlowB = 100.0f;
            floatToHexArray(UserParam.UserFlowB, u8Temp, 4);
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, USER_FLOW_B, 4, u8Temp);
            index += 2;
            break;
            
        case 0x7139:	//����						                
            index += 2;
            break;

        case 0x713A:	//����						                
            index += 2;
            break;            

        case 0x713B:	//����						                
            index += 2;
            break; 

        case 0x713C:	//����						                
            index += 2;
            break; 
            
        case 0x713D:                                                             //������λ
             uTemp = (u8)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
             if(uTemp != 0xFFFF)
             {
                RunVar.Weight_Unit = uTemp;
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, WEIGHT_UNIT, 1, &RunVar.Weight_Unit);
             }
            index += 2;
            break; 
            
        case 0x713E:	//����					                
            index += 2;
            break; 
            
        case 0x713F:						                //��������/�غ�����
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.VehicleWeight = uTemp;
                UshortToArray(RunVar.VehicleWeight, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, VEHICLE_WEIGHT, 2, u8Temp);
            }
            index += 2;
            break;

        case 0x7140:						                //���ط�ֵ
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.OverloadLimit = uTemp;
                UshortToArray(RunVar.OverloadLimit, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, OVERLOAD_LIMIT, 2, u8Temp);
            }
            index += 2;
            break; 

        case 0x7141:						                //���ط�ֵƫ��
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.OverloadLimitDevation = uTemp;
                UshortToArray(RunVar.OverloadLimitDevation, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, OVERLOAD_LIMIT_DEVIATION, 2, u8Temp);  
            }
            index += 2;
            break;

        case 0x7142:						                //���ز�������
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.LoadMeasureScheme = uTemp;
                if((RunVar.LoadMeasureScheme != 1) && (RunVar.LoadMeasureScheme != 2))
                {
                    RunVar.LoadMeasureScheme = 2;
                }
                UshortToArray(RunVar.LoadMeasureScheme, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_MEASURE_SCHEME, 2, u8Temp); 
            }
            index += 2;
            break;

        case 0x7143:						                //���ط�ֵ
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.LoadLimit = uTemp;
                UshortToArray(RunVar.LoadLimit, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_LIMIT, 2, u8Temp);  
            }
            index += 2;
            break;

        case 0x7144:						                //���ط�ֵƫ��
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.LoadLimitDevation = uTemp;
                UshortToArray(RunVar.LoadLimitDevation, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_LIMIT_DEVIATION, 2, u8Temp); 
            }
            index += 2;
            break;

        case 0x7145:						                //���ط�ֵ
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.EmptyLimit = uTemp;
                UshortToArray(RunVar.EmptyLimit, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, EMPTYLOAD_LIMIT, 2, u8Temp);
            }            
            index += 2;
            break;

        case 0x7146:						                //���ط�ֵƫ��
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.EmptyLimitDevation = uTemp;
                UshortToArray(RunVar.EmptyLimitDevation, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, EMPTYLOAD_LIMIT_DEVIATION, 2, u8Temp);
            }
            index += 2;
            break;

        case 0x7147://������ֵ
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.LightLimit = uTemp;
                UshortToArray(RunVar.LightLimit, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LIGHTLOAD_LIMIT, 2, u8Temp); 
            }
            index += 2;
            break;

        case 0x7148:	//������ֵƫ��				                
            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
            if(uTemp != 0xFFFF)
            {
                RunVar.LightLimitDevation = uTemp;
                UshortToArray(RunVar.LightLimitDevation, u8Temp);
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LIGHTLOAD_LIMIT_DEVIATION, 2, u8Temp);
            }
            index += 2;            
            break;

        case 0x7149:						                
            index += 2;
            break;

        case 0x714A:                                                             
            index += 2;
            break;                                                                                      
            
         case 0x714B:                                                             
            index += 2;
            break;           
            
        case 0x714C:                                                             
            index += 2;
            break;
            
        case 0x714D:                                                             
            index += 2;
            break;

        case 0x714E:                                                             
            index += 2;
            break;
            
        case 0x714F:                                                             
            index += 2;
            break;
            
        case 0x7160:                                                            //���ģʽ 0-RTU��1-ASCII
            ProductPara.cyMbtWorkType = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            if((ProductPara.cyMbtWorkType != 0) && (ProductPara.cyMbtWorkType != 1))
                ProductPara.cyMbtWorkType = 1;
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, MBT_WORK_TYPE, 1, &ProductPara.cyMbtWorkType);
            index += 2;				        
            break;

        case 0x7170:						                //���ٶ�
            Speed = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            myOutData = Runing(g_wRADValue);                                    //����һ���ٶȼ��һ������
            index += 2;
            break;
            
        case 0x7171:
            Acc_Flag = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
            if((Acc_Flag == 1) && (Acc_Flag_Cnt++ >= 4))
            {
                Acc_Flag_Cnt = 0;
                RunVar.AdcMaxCnt = 20;
            }
            else
            {
                RunVar.AdcMaxCnt = ProductPara.cyFilterLevel * 90;
            }
            break;
            
        case 0x7172:
            break;

        case 0x7173:
            break;             

        case 0x7180: /*������5�飬ÿ��10�����ݣ�ÿ������4���ֽ�*/                
            s_TotalPacks = ArrayToUshort(UART1_RXBuff + 7);
            s_CurrentPack = ArrayToUshort(UART1_RXBuff + 9);
            if((s_TotalPacks > 5) || (s_CurrentPack > 4) || (s_CurrentPack >= s_TotalPacks) 
               || (Register_Num < 6) || (Register_Num > 42))
            {
                MBASC_SendErr(MB_EX_ILLEGAL_DATA_VALUE);
                return;
            }          
            
            for(i = 0; i < 10; i++)
            {
                if((HexToUlong(UART1_RXBuff + 8 * i + 11) != 0xFFFFFFFF)
                   && (HexToUlong(UART1_RXBuff + 8 * i + 15) != 0xFFFFFFFF))    //�������ݲ�Ϊ0xFFFFFFFF��д������
                {
                    AD_Buff[s_CurrentPack][i] = HexToUlong(UART1_RXBuff + 8 * i + 11);
                    Weight_Buff[s_CurrentPack][i] = HexToUlong(UART1_RXBuff + 8 * i + 15);
                }
                else
                {
                    for(j = i; j < 10; j++)
                    {
                        AD_Buff[s_CurrentPack][j] = 0xFFFFFFFF;
                        Weight_Buff[s_CurrentPack][j] = 0xFFFFFFFF;
                    }
                    break;
                }
            }
            if(s_CurrentPack == 0)                                              //��0��ʱд���ܰ���
            {
                UshortToArray(s_TotalPacks, u8Temp);
                I2C1_WriteNBytes(EEPROM_ADDRESS, AD_WEIGHT_PACKS, 2, u8Temp);
            }
            memmove(uBuf, (u8*)(&UART1_RXBuff[11]), i * 8);                     //ǰ��д����յ�������
            if(i < 10)
            {
                memset(uBuf + i * 8, 0xFF, (10 - i) * 8);                       //����д��FF
            }
            I2C1_WriteNBytes(EEPROM_ADDRESS, AD_WEIGHT_ADDR(s_CurrentPack), 80, uBuf); //д����ձ�
            MBASC_SendMsg(UART1_RXBuff, 11);                                    //Ӧ��ǰ11���ֽ� 
            return;

        default:
            break;
        }
    }
    ProductPara.bFlashWEn = FLASH_WR_DISABLE;

    MBASC_SendMsg(UART1_RXBuff, 6);
    if(CommChangeFlag == Bit_SET)
    {   
        while(0 != u8SendNum);                                                  //�ȴ��������ݷ������
        CommChangeFlag = Bit_RESET;
        UART1_Init(&UartComm);                                                  //��ʼ������
    }
}


void MBASC_Fun26(void)
{
    u32 i;
    u32 Data_Buf;
    u16 ReadAdr = 0;
    u16 Register_Num = 0;

    Register_Num = (u16)UART1_RXBuff[4] * 256 + UART1_RXBuff[5];    
    if(UART1_RXBuff[2] - UartComm.SlaveAddr !=0) 
        ReadAdr = (u16)UART1_RXBuff[2] * 256 + UART1_RXBuff[3];
    else
        ReadAdr = 0x71 * 256 + UART1_RXBuff[3];

    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00;
    SendBuf[SendLen++] = 0x26;
    SendBuf[SendLen++] = Register_Num * 2;		
    
    if (!((((ReadAdr >= 0x7180) && (ReadAdr <= 0x718E) && (ReadAdr + Register_Num) <= (0x718E + 2))
       || ((ReadAdr >= 0x7190) && (ReadAdr <= 0x719E) && (ReadAdr + Register_Num) <= (0x719E + 2))
       || ((ReadAdr >= 0x71A0) && (ReadAdr <= 0x71AE) && (ReadAdr + Register_Num) <= (0x71AE + 2))
       || ((ReadAdr >= 0x71B0) && (ReadAdr <= 0x71BE) && (ReadAdr + Register_Num) <= (0x71BE + 2))
       || ((ReadAdr >= 0x71C0) && (ReadAdr <= 0x71CE) && (ReadAdr + Register_Num) <= (0x71CE + 2)))
       && ((0 != Register_Num) && (0 == (Register_Num & 0x01)) && (0 == (ReadAdr & 0x01)))))
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }
    for (long k = 0; k < Register_Num; ReadAdr += 2, k += 2)
    {
        switch (ReadAdr)
        {

        case 0x7180:                                                            //��������                                                
            Data_Buf = 0;
            break;

        case 0x7182:                                                            //�������AD                                                       
            Data_Buf = 0;
            break;

        case 0x7184:                                                            //����������AD
            Data_Buf = 0;
            break;

        case 0x7186:
            Data_Buf = 0;
            break;

        case 0x7188:                                                            
            Data_Buf = 0;
            break;

        case 0x718A:                                                            
            Data_Buf = 0;
            break;

        case 0x718C:                                                           
            Data_Buf = 0;
            break;

        case 0x718E:                                                            
            Data_Buf = 0;
            break;

        case 0x7190:                                                            

        case 0x7192:

        case 0x7194:

        case 0x7196:

        case 0x7198:

        case 0x719A:

        case 0x719C:

        case 0x719E:
            Data_Buf = 0;
            break;

        case 0x71A0:                                                           

        case 0x71A2:

        case 0x71A4:

        case 0x71A6:

        case 0x71A8:

        case 0x71AA:

        case 0x71AC:

        case 0x71AE:
            Data_Buf = 0;
            break;

        case 0x71B0:                                                            

        case 0x71B2:

        case 0x71B4:

        case 0x71B6:

        case 0x71B8:

        case 0x71BA:

        case 0x71BC:

        case 0x71BE:
            Data_Buf = 0;
            break;

        case 0x71C0:                                                            

        case 0x71C2:

        case 0x71C4:

        case 0x71C6:

        case 0x71C8:

        case 0x71CA:

        case 0x71CC:

        case 0x71CE:
            Data_Buf = 0;
            break;

        default:
            MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }

        for (i = 4; i > 0; i--)
        {
            SendBuf[SendLen++] = (u8)(Data_Buf >> ((i - 1) * 8));
        }
    }

    MBASC_SendMsg(SendBuf, SendLen);
}



void MBASC_Fun27(void)
{
    u32 index = 0;
//  u8 bFlashWStatus = ProductPara.bFlashWEn;
 
    u16 ReadAdr = 0;
    u16 Register_Num = 0;
    ProductPara.bFlashWEn = FLASH_WR_ENABLE;
    
    Register_Num = (u16)UART1_RXBuff[4] * 256 + UART1_RXBuff[5];    
    if(UART1_RXBuff[2] - UartComm.SlaveAddr !=0) 
        ReadAdr = (u16)UART1_RXBuff[2] * 256 + UART1_RXBuff[3];
    else
        ReadAdr = 0x71 * 256 + UART1_RXBuff[3]; 
    
    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00;
    SendBuf[SendLen++] = 0x27;
    SendBuf[SendLen++] = Register_Num * 2;

    if ((0 != (Register_Num & 0x01)) || (0 == Register_Num) || (0 != (ReadAdr & 0x01))
            || ((Register_Num * 2) != UART1_RXBuff[6]))
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }
    else if (!(((ReadAdr >= 0x7180) && (ReadAdr <= 0x718E) && (ReadAdr + Register_Num) <= (0x718E + 2))
           || ((ReadAdr >= 0x7190) && (ReadAdr <= 0x719E) && (ReadAdr + Register_Num) <= (0x719E + 2))
           || ((ReadAdr >= 0x71A0) && (ReadAdr <= 0x71AE) && (ReadAdr + Register_Num) <= (0x71AE + 2))
           || ((ReadAdr >= 0x71B0) && (ReadAdr <= 0x71BE) && (ReadAdr + Register_Num) <= (0x71BE + 2))
           || ((ReadAdr >= 0x71C0) && (ReadAdr <= 0x71CE) && (ReadAdr + Register_Num) <= (0x71CE + 2))))
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

    if ((ProductPara.bFlashWEn == FLASH_WR_DISABLE) &&
	!(((ReadAdr >= 0x7190) && (ReadAdr <= 0x719E) )
        || ((ReadAdr >= 0x71A0) && (ReadAdr <= 0x71AE) )
        || ((ReadAdr >= 0x71B0) && (ReadAdr <= 0x71BE) )
        || ((ReadAdr >= 0x71C0) && (ReadAdr <= 0x71CE) )))
    {
        MBASC_SendErr(MB_EX_SLAVE_DEVICE_FAILURE);
        return;
    }

    for (long k = 0; k < Register_Num; ReadAdr += 2, k += 2)
    {
        switch (ReadAdr)
        {

        case 0x7180:                                                            
            index += 4;
            break;

        case 0x7182:                                                           
            index += 4;
            break;

        case 0x7184:
            index += 4;
            break;

        case 0x7186:
            index += 4;
            break;

        case 0x7188:                                                            
            index += 4;
            break;

        case 0x718A:                                                            
            index += 4;
            break;

        case 0x718C:                                                            
            index += 4;
            break;

        case 0x718E:                                                            
            index += 4;
            break;

        case 0x7190:                                                            

        case 0x7192:

        case 0x7194:

        case 0x7196:

        case 0x7198:

        case 0x719A:

        case 0x719C:

        case 0x719E:
            index += 4;
            break;

        case 0x71A0:                                                            

        case 0x71A2:

        case 0x71A4:

        case 0x71A6:

        case 0x71A8:

        case 0x71AA:

        case 0x71AC:

        case 0x71AE:
            index += 4;
            break;

        case 0x71B0:                                                            

        case 0x71B2:

        case 0x71B4:

        case 0x71B6:

        case 0x71B8:

        case 0x71BA:

        case 0x71BC:

        case 0x71BE:
            index += 4;
            break;

        case 0x71C0:                                                            

        case 0x71C2:

        case 0x71C4:

        case 0x71C6:

        case 0x71C8:

        case 0x71CA:

        case 0x71CC:

        case 0x71CE:
            index += 4;
            break;

        default:
            index += 4;
            break;
        }
    }
    MBASC_SendMsg(UART1_RXBuff, 6);
}



//**************************************************************************************************
// ����         : MBASC_Fun2A()
// ��������     : 2016-09-19
// ����         : ÷����
// ����         : �������ù�˾���ơ���Ʒ���롢�汾�š��豸ID���ͻ�����
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : ע�⣺��5������һ��ֵʱ�������ܴ����Խ��ջ�����Ҫ�㹻��
// �޸�����     :
//**************************************************************************************************
void MBASC_Fun2A(void)
{                                                      
    u32 index = 0;
    u8 Object_Len[6]={0};                                                       
    u8 Len[5]={0};
    u8 Length[6] = {0}; 

    u16 ReadAdr = 0;
    u16 Object_Num = 0;
    
    if(UartComm.SlaveAddr != UART1_RXBuff[2])                                   //���ӻ���ַ��Ϊ��0x45ʱ��Ҳ���Զ�д�Ĵ���
    {
        ReadAdr = UART1_RXBuff[2] * 256 + UART1_RXBuff[3];
    }
    else
    {
        ReadAdr = 0x71 * 256 + UART1_RXBuff[3];
    }
    Object_Num = UART1_RXBuff[4] * 256 + UART1_RXBuff[5];                       //д�������

    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00;
    SendBuf[SendLen++] = 0x2A;
    SendBuf[SendLen++] = UART1_RXBuff[2];
    SendBuf[SendLen++] = UART1_RXBuff[3];
    SendBuf[SendLen++] = UART1_RXBuff[4];
    SendBuf[SendLen++] = UART1_RXBuff[5]; 
    
    if (!(((ReadAdr >= 0x71E0) && (ReadAdr <= 0x71E5) && (ReadAdr + Object_Num <= 0x71E5 + 1)) && (0 != Object_Num)))//��ַ����E0-E5֮�� �����������5���ߵ���0
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

    if (ProductPara.bFlashWEn == FLASH_WR_DISABLE)//���flash������д��
    {
        MBASC_SendErr(MB_EX_SLAVE_DEVICE_FAILURE);
        return;
    }
    
    Object_Len[ReadAdr - 0x71E0] = UART1_RXBuff[6];                             //���շ������Ķ���ĳ���                        
    Len[0] = Object_Len[ReadAdr - 0x71E0];
    for(u8 i = 0; i < Object_Num - 1; i++)
    {
       Length[i + 1] = Length[i] + Len[i] + 1;
       Object_Len[ReadAdr - 0x71E0 + (i + 1)] = UART1_RXBuff[6 + Length[i + 1]];
    }
       
    for (u8 k = 0; k < Object_Num; ReadAdr++, k++)
    {
        switch (ReadAdr)
        {
        case 0x71E0:
            I2C1_WriteNBytes(EEPROM_ADDRESS, COMPANY, Object_Len[0] + 1, &UART1_RXBuff[6 + index]);            
            index += Object_Len[0] + 1 ;                                        //��Ҫд������ݵĳ���д�ڵ�һ��λ�ã��������
            break;
        
        case 0x71E1:           
            I2C1_WriteNBytes(EEPROM_ADDRESS, DEV_ENCODING, Object_Len[1] + 1, &UART1_RXBuff[6 + index]);            
            index += Object_Len[1] + 1;
            break;
            
        case 0x71E2:          
            I2C1_WriteNBytes(EEPROM_ADDRESS, HWVERSION, Object_Len[2] + 1, &UART1_RXBuff[6 + index]);
            index += Object_Len[2] + 1;
            break;
            
        case 0x71E3:          
            I2C1_WriteNBytes(EEPROM_ADDRESS, SFVERSION, Object_Len[3] + 1, &UART1_RXBuff[6 + index]);
            index += Object_Len[3] + 1;
            break;
            
        case 0x71E4:           
            I2C1_WriteNBytes(EEPROM_ADDRESS, DEV_ID, Object_Len[4] + 1, &UART1_RXBuff[6 + index]);            
            index += Object_Len[4] + 1;
            break;

        case 0x71E5:           
            I2C1_WriteNBytes(EEPROM_ADDRESS, CUSTOMERCODE, Object_Len[5] + 1, &UART1_RXBuff[6 + index]);            
            index += Object_Len[5] + 1;
            break;
            
        default:
            MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }
    }
    MBASC_SendMsg(UART1_RXBuff, SendLen);
}


//**************************************************************************************************
// ����         : MBASC_Fun2B()
// ��������     : 2016-09-19
// ����         : ÷����
// ����         : ���ڶ�ȡ�豸���к���Ϣ
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : һ���5������ʱ�������ܴ�ע��SendBuf�ĳ���
// �޸�����     :
//**************************************************************************************************
void MBASC_Fun2B(void)
{
    u16 Object_Num = 0, ReadAdr = 0;
    u8 CompanyNameLen = 0, ProductionCodeLen = 0, HardwareVersionLen = 0;
    u8 SoftwareVersionLen = 0, DeviceIdLen = 0, CustomerCodeLen = 0;
    
    if(UartComm.SlaveAddr != UART1_RXBuff[2])                                   //���ӻ���ַ��Ϊ��0x45ʱ��Ҳ���Զ�д�Ĵ���
    {
        ReadAdr = UART1_RXBuff[2] * 256 + UART1_RXBuff[3];
    }
    else
    {
        ReadAdr = 0x71 * 256 + UART1_RXBuff[3];
    }
    Object_Num = UART1_RXBuff[4] * 256 + UART1_RXBuff[5];                       //���������   
    
    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00;
    SendBuf[SendLen++] = 0x2B;
    SendBuf[SendLen++] = UART1_RXBuff[4];
    SendBuf[SendLen++] = UART1_RXBuff[5];                      
    
    if (!(((ReadAdr >= 0x71E0) && (ReadAdr <= 0x71E5) && (ReadAdr + Object_Num) <= (0x71E5 + 1))
        && (0 != Object_Num)))
    {
        MBASC_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }   
    
    for (u8 k = 0; k < Object_Num; ReadAdr++, k++)
    {
        switch (ReadAdr)
        {
        case 0x71E0:
            SendBuf[SendLen++] = 0xE0;
            CompanyNameLen = I2C1_ReadByte(EEPROM_ADDRESS, COMPANY);            //��ȡ��һ��λ�õ����ݵĳ���
            if((0 == CompanyNameLen) || (0x32 < CompanyNameLen))                //���Ȳ���ȷ
            {
                SendBuf[SendLen++] = 0x01;                                      //Ӧ��һ���ֽ� 0x00                             
                SendBuf[SendLen++] = 0x00;
            }
            else
            {
                SendBuf[SendLen++] = CompanyNameLen;
                for (u8 i = 0; i < CompanyNameLen; i++)
                {                             
                    SendBuf[SendLen++] = I2C1_ReadByte(EEPROM_ADDRESS, COMPANY + 1 + i);
                }
            }
            break;
        
        case 0x71E1:
            SendBuf[SendLen++] = 0xE1;
            ProductionCodeLen = I2C1_ReadByte(EEPROM_ADDRESS, DEV_ENCODING);
            if((0 == ProductionCodeLen) || (0x32 < ProductionCodeLen))        
            {
                SendBuf[SendLen++] = 0x01;                                      //Ӧ��һ���ֽ� 0x00                             
                SendBuf[SendLen++] = 0x00;
            }
            else
            {
                SendBuf[SendLen++] = ProductionCodeLen;
                for (u8 i = 0; i < ProductionCodeLen; i++)
                {                             
                    SendBuf[SendLen++] = I2C1_ReadByte(EEPROM_ADDRESS, DEV_ENCODING + 1 + i);
                }
            }
            break;
            
        case 0x71E2:
            SendBuf[SendLen++] = 0xE2;
            HardwareVersionLen = I2C1_ReadByte(EEPROM_ADDRESS, HWVERSION);
            if((0 == HardwareVersionLen) || (0x32 < HardwareVersionLen))      
            {
                SendBuf[SendLen++] = 0x01;                                      //Ӧ��һ���ֽ� 0x00                             
                SendBuf[SendLen++] = 0x00;
            }
            else
            {
                SendBuf[SendLen++] = HardwareVersionLen;
                for (u8 i = 0; i < HardwareVersionLen; i++)
                {                             
                    SendBuf[SendLen++] = I2C1_ReadByte(EEPROM_ADDRESS, HWVERSION + 1 + i);
                }
            }
            break;
            
        case 0x71E3:
            SendBuf[SendLen++] = 0xE3;
            SoftwareVersionLen = I2C1_ReadByte(EEPROM_ADDRESS, SFVERSION);
            if((0 == SoftwareVersionLen) || (0x32 < SoftwareVersionLen))       
            {
                SendBuf[SendLen++] = 0x01;                                      //Ӧ��һ���ֽ� 0x00                             
                SendBuf[SendLen++] = 0x00;
            } 
            else
            {
                SendBuf[SendLen++] = SoftwareVersionLen;
                for (u8 i = 0; i < SoftwareVersionLen; i++)
                {                             
                    SendBuf[SendLen++] = I2C1_ReadByte(EEPROM_ADDRESS, SFVERSION + 1 + i);
                }
            }
            break;
            
        case 0x71E4:
            SendBuf[SendLen++] = 0xE4;
            CustomerCodeLen = I2C1_ReadByte(EEPROM_ADDRESS, DEV_ID);
            if((0 == CustomerCodeLen) || (0x32 < CustomerCodeLen))            //û��д����ȥ���򷵻ش�����
            {
                SendBuf[SendLen++] = 0x01;                                      //Ӧ��һ���ֽ� 0x00                             
                SendBuf[SendLen++] = 0x00;
            } 
            else
            {
                SendBuf[SendLen++] = CustomerCodeLen;
                for (u8 i = 0; i < CustomerCodeLen; i++)
                {                             
                    SendBuf[SendLen++] = I2C1_ReadByte(EEPROM_ADDRESS, DEV_ID + 1 + i);
                }
            }
            break;
            
        case 0x71E5:
            SendBuf[SendLen++] = 0xE5;
            DeviceIdLen = I2C1_ReadByte(EEPROM_ADDRESS, CUSTOMERCODE);
            if((0 == DeviceIdLen) || (0x32 < DeviceIdLen))                     //û��д����ȥ����ֱ�ӷ���
            {
                SendBuf[SendLen++] = 0x01;                                      //Ӧ��һ���ֽ� 0x00                             
                SendBuf[SendLen++] = 0x00;
            } 
            else
            {
                SendBuf[SendLen++] = DeviceIdLen;
                for (u8 i = 0; i < DeviceIdLen; i++)
                {                             
                    SendBuf[SendLen++] = I2C1_ReadByte(EEPROM_ADDRESS, CUSTOMERCODE + 1 + i);
                }
            }            
            break;            
            
        default:
            break; 
        }
    }
    MBASC_SendMsg(SendBuf, SendLen);
}



#define MB_RECDATA_SUCCESSED              0x00
#define MB_RECDATA_FAILED                 0x01
#define MB_CHECKSUM_ERROR                 0x02 
#define APPEXISTFLAGADDR                  0x7FF                   
//**************************************************************************************************
// ����         : MBASC_Fun41()
// ��������     : 2016-09-19
// ����         : ÷����
// ����         : ����ϵͳ����boot��ʼִ�У�
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     :
//**************************************************************************************************
void MBASC_Fun41(void)
{
    u16 ReadAdr = 0;
    u16 DataLength = 0;

    ReadAdr = (u16)(UART1_RXBuff[2] * 256 + UART1_RXBuff[3]);
    DataLength = (u16)(UART1_RXBuff[4] * 256 + UART1_RXBuff[5]);
        
    SendLen = 0;
    SendBuf[SendLen++] = (MBASC_GetSlaveAddr(UART1_RXBuff)) ? UartComm.SlaveAddr : 0x00;
    SendBuf[SendLen++] = 0x41;
    SendBuf[SendLen++] = UART1_RXBuff[2];
    SendBuf[SendLen++] = UART1_RXBuff[3];
    SendBuf[SendLen++] = UART1_RXBuff[4];
    SendBuf[SendLen++] = UART1_RXBuff[5];                          
    
    if((0x0001 != ReadAdr) || (0 != DataLength))
    {
        return;
    }
    else 
    {
        ProductPara.bFlashWEn = FLASH_WR_ENABLE;
        I2C1_WriteByte(EEPROM_ADDRESS, APPEXISTFLAGADDR, 0x0C);
        if(0x0C == I2C1_ReadByte(EEPROM_ADDRESS, APPEXISTFLAGADDR))
        {
            SendBuf[SendLen++] = MB_RECDATA_SUCCESSED;
            MBASC_SendMsg(SendBuf, SendLen);
            while(0 != u8SendNum);                                              //�ȴ����ݷ�����ϣ��˾�Ҫ���ϣ�����ִ�д˹�����������Ӧ
            NVIC_SystemReset();    //Initiates a system reset request to reset the MCU.
        }
        else
        {
            SendBuf[SendLen++] = MB_RECDATA_FAILED;
            MBASC_SendMsg(SendBuf, SendLen); 
        }
    } 
}     
     

//**************************************************************************************************
// ����         : MBASC_Function()
// ��������     : 2016-09-05
// ����         : ׯ��Ⱥ
// ����         : modbus asciiͨ�Ŵ���
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     :  ����У������������������0x05      2016.09.08
//                 ���ӷǷ����ݴ����룬������0x03��ASCII��ʽ���ݣ���3A 0D 0A�⣬��������ֻ��Ϊ30~39��41~46��    2016.09.10                            
//**************************************************************************************************
void MBASC_Function(void)
{
    u8 RecvLen = 0;
    if(2 == MODBUS_ASCII_RecvData(UART1_RXBuff, &RecvLen))//У�����
    {
        return;
    }  
    
    if(RecvLen <= 0)
    {
        return;                                                                 //û�н��ܵ����ݣ�����Ӧ
    }

    else if((UartComm.SlaveAddr != MBASC_GetSlaveAddr(UART1_RXBuff))&&
         (MB_ADDRESS_BROADCAST != MBASC_GetSlaveAddr(UART1_RXBuff)))
    {
        return;                                                                 //���յĴӻ���ַ����Ӧ������Ӧ
    }
    else
    {
        switch (MBASC_GetFunCode(UART1_RXBuff))  //�жϴ��ڻ������ڶ��ֽ�
        {
          case MB_FUNC_READ_HOLDING_REGISTER:
            MBASC_Fun03();	                                                //�������Ĵ��������ֽ����ݣ�
            break;

          case MB_FUNC_READ_INPUT_REGISTER:
            MBASC_Fun04();	                                                //��˫���Ĵ������������ݣ�
            break;

          case MB_FUNC_WRITE_SINGLE_COIL:
            MBASC_Fun05();	                                                //д��Ȧ
            break;

          case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            MBASC_Fun10();                                                      //д����Ĵ���
            break;

          case 0x26:
            MBASC_Fun26();                                                      //д����Ĵ���
            break;
            
          case 0x27:
            MBASC_Fun27();                                                      //д����Ĵ���
            break;
            
          case 0x2A:
            MBASC_Fun2A();                                                      //д����Ĵ���
            break;
            
          case 0x2B:
            MBASC_Fun2B();                                                      //д����Ĵ���
            break;
            
          case 0x41:
            MBASC_Fun41();                                                      //д����Ĵ���
            break;
            
          default:
            SendLen = 0;
            SendBuf[SendLen++] = MBASC_GetSlaveAddr(UART1_RXBuff);
            SendBuf[SendLen++] = 0x80 | (u8)MBASC_GetFunCode(UART1_RXBuff);
            SendBuf[SendLen++] = MB_EX_ILLEGAL_FUNCTION;
            MBASC_SendMsg(SendBuf, SendLen);
            break;
         }
     }
}

//**************************************************************************************************
// ����         : MBASC_AutoUpLoadFrame()
// ��������     : 2016-09-014
// ����         : ÷����
// ����         : modbus ascii��ʽ�Զ��ϴ���Ϣ
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : 
// �޸�����     : 1.�޸��ϴ����ݸ�ʽ������0x45(�ӻ���ַ)   0x04(������)   �Ĵ�������*2(8*2)  
//                2.����Э���޸����ݷ��͸�ʽ   2016.09.12
//**************************************************************************************************
#define AutoUpLoadRegisterNum       4                                          //�Զ��ϴ����� �Ĵ�������*2  
void MBASC_AutoUpLoadFrame(void)
{
 
    SendLen = 0;
    SendBuf[SendLen++] = UartComm.SlaveAddr;
    SendBuf[SendLen++] = MB_FUNC_READ_INPUT_REGISTER;                           //04������
    SendBuf[SendLen++] = AutoUpLoadRegisterNum;		                //�Ĵ�������*2
     
 
     SendBuf[SendLen++] = (uint16_t)g_wRADValue>>8;
     SendBuf[SendLen++] =  (uint16_t)g_wRADValue & 0x00FF;
     SendBuf[SendLen++] = ADXL345_BUF[6];
     SendBuf[SendLen++] = ADXL345_BUF[7];
     SendBuf[SendLen++] = ADXL345_BUF[8];
     SendBuf[SendLen++] = ADXL345_BUF[9]; 
     SendBuf[SendLen++] = ADXL345_BUF[10];
     SendBuf[SendLen++] = ADXL345_BUF[11];
  

    MBASC_SendMsg_NoLimit(SendBuf, SendLen);
}
