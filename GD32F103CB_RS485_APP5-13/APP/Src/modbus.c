#include  "Modbus.h"
#include "Baseincludes.h"
#include "Usart.h"
extern uint16_t FilterAdc;
extern uint16_t ton;				//����
static S_SNDDAT_TAG s_mdbs_snd = {0};
static S_PARSE_TAG s_mdbs_parse = {0};
extern double X_Angle,Y_Angle,Z_Angle;
extern float X_G,Y_G,Z_G; 	
static UartCommTypeDef UartComm;                                                       //���ڲ���


//2020-11-20 Added a variable to show real time oil_value
//extern __IO uint32_t tmpOilRealTimeValue;

extern uint8_t const User_Default_Param[PRO_DEFAULT_LEN];                          //�û�ȱʡ����



uint32_t dwEepromHeightADValue;                                                      //��������Һλ�߶�
uint32_t dwEepromOilValue;                                                           //������������
uint32_t dwUpEepromHeightADValue;                                                    //��һ����������Һλ�߶�
uint32_t dwUpEepromOilValue;                                                         //��һ��������������
uint32_t dwTankOilValue;                                                             //��ǰ��������
float dwTable_k;                                                               //������ϵб��

/*******************************************************************************
Name:    
Func: 
Para:   
Retn:
Desc:
Date: 2020-07-13
*******************************************************************************/
bool F3_CommunicateInit( PS_MODBUS_RUNPARA pMdbsPara )
{
    //Receive, send and paser parameter init
	if ((!pMdbsPara->recvdata) || (!pMdbsPara->senddata) /*|| (!pMdbsPara->pparse)*/)
		return false;
	
	if ( (!pMdbsPara->psendbuff) || (!pMdbsPara->pparsebuff))
		return false;
	
	if ( (!pMdbsPara->sendsize) || (!pMdbsPara->parsesize))
		return false;

	s_mdbs_snd.psndbuf = pMdbsPara -> psendbuff;
	s_mdbs_snd.bufsize = pMdbsPara -> sendsize;
	s_mdbs_snd.senddata = pMdbsPara -> senddata;
	
	s_mdbs_parse.pbuf = pMdbsPara -> pparsebuff;
	s_mdbs_parse.bufsize = pMdbsPara -> parsesize;
	s_mdbs_parse.recvdata = pMdbsPara -> recvdata;	

	UartComm.SlaveAddr = pMdbsPara -> SlaveAddr;	//�豸��ַ
	UartComm.BaudRate = pMdbsPara ->baudrate;		//������
	UartComm.Parity = pMdbsPara ->parity;			//������ż��
    return true;
}


/*******************************************************************************
Name:    
Func: 
Para:   
Retn:
Desc:
Date: 2020-07-13
*******************************************************************************/
void MBASCII_Function(void)
{ 
	uint8_t* ptmp = s_mdbs_parse.pbuf;
	uint32_t RecvLen = s_mdbs_parse.recvdata( ptmp, s_mdbs_parse.bufsize );
	if (RecvLen < 3)
		return;

	if (*ptmp != ASCII_FRAME_HEADER)
		return;  
	
	ptmp = s_mdbs_parse.pbuf + RecvLen - 2;
	if (*ptmp != ASCII_FRAME_ENTER)
		return;
	
	ptmp ++;
	if (*ptmp != ASCII_FRAME_NEWLINE)
		return;
	
	if (!COMMON_ConverAscii2CommonMode(s_mdbs_parse.pbuf, RecvLen))
		return;
	
	ptmp = s_mdbs_parse.pbuf;
	RecvLen = (RecvLen - 5) / 2;    //Remove 0x3A 0x0D 0x0A and LRC
	
	if ((*ptmp) && (*ptmp) != UartComm.SlaveAddr)
		return;
	

	switch (s_mdbs_parse.pbuf[1])
	{
	case MB_FUNC_READ_HOLDING_REGISTER:	//�������Ĵ���0x03
		MBASCII_Fun03();
		break;
		
	case MB_FUNC_READ_INPUT_REGISTER:		//�������Ĵ���0x04
		MBASCII_Fun04();
		break;
		
	case MB_FUNC_WRITE_SINGLE_COIL:		//д������Ȧ
		MBASCII_Fun05();
		break;
		
	case MB_FUNC_WRITE_MULTIPLE_REGISTERS://д����Ĵ���0x10
		if((RecvLen - 7) != s_mdbs_parse.pbuf[6])
		{
			return;
		}
		MBASCII_Fun10();
		break;
	  
	case 0x25:
		MBASCII_Fun25();
		break;
	  
	case 0x26:
		MBASCII_Fun26();
		break;
	  
	case 0x27:
		MBASCII_Fun27();
		break;
	  
	case 0x2A:
		MBASCII_Fun2A();
		break;
	  
	case 0x2B:
		MBASCII_Fun2B();
		break;

	case 0x41:
		MBASCII_Fun41();
		break;
	  
	default:
		s_mdbs_snd.sendlen = 0;
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0];
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x80 | s_mdbs_parse.pbuf[1];
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = MB_EX_ILLEGAL_FUNCTION;
		MBASCII_SendMsg(s_mdbs_snd.psndbuf, s_mdbs_snd.sendlen);
		break;
	}
}

/*******************************************************************************
Name:    
Func: 
Para:   
Retn:
Desc:
Date: 2020-07-13
*******************************************************************************/
void MBASCII_SendMsg(uint8_t *pDataBuff, uint32_t datlen)
{
	uint8_t lrcchk;
	uint8_t* pans;
	
//	Delay_ms(39);
	GD32F103_Systick_Delayxms(39);
	memcpy(s_mdbs_snd.psndbuf,pDataBuff,datlen);
	if(( MB_ADDRESS_BROADCAST != s_mdbs_snd.psndbuf[0] ) || (s_mdbs_snd.psndbuf[1] == 0x03))
	{
		lrcchk = COMMON_GeneratorLRCCheckCode( s_mdbs_snd.psndbuf, datlen);
		pans = s_mdbs_snd.psndbuf + datlen;
		*pans = lrcchk;
		datlen ++;
		if (!COMMON_ConvertHex2Ascii( s_mdbs_snd.psndbuf, datlen ))
			return;
		datlen *= 2;
		pans = s_mdbs_snd.psndbuf;
		*pans++ = ASCII_FRAME_HEADER;
		pans += datlen;
		COMMON_Bits16Convert2Bits8( pans, ASCII_FRAME_ENDFLAG, BIGENDDIAN );
		datlen += 3;
		//s_mdbs_snd.senddata( s_mdbs_snd.psndbuf, s_mdbs_snd.bufsize,datlen );
		My_USART_DMA_SendData(datlen);
	}
}


//**************************************************************************************************
// ����         	: MBASCII_SendErr()
// ��������     	: 2015-07-13
// ����        	 	: ��С��
// ����         	: ����MODBUS ����
// �������     	: uint8_t ErrCode
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_SendErr(uint8_t ErrCode)
{
	s_mdbs_snd.sendlen = 1;
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] |= 0x80;
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = ErrCode;

	MBASCII_SendMsg(s_mdbs_snd.psndbuf,s_mdbs_snd.sendlen);
}



//**************************************************************************************************
// ����         	: Read_LoadAndWeightTable()
// ��������     	: 2015-07-13
// ����        	 	: ׯ��Ⱥ
// �޸���               : ׯ��Ⱥ
// ����         	: ��ȡ����ADֵ�ͳ��ض�Ӧ��
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:֡��ʽ
//**************************************************************************************************

void Read_CALIBAndHeighTable(void)
{
  uint8_t i,j=0;
  uint8_t dwSendLen = 0;
  uint16_t dwPacketSum;//�ܰ���
  uint16_t dwPacketNum;//���ڼ�������
  uint8_t dwTable[200];
  dwPacketSum = s_mdbs_parse.pbuf[7]*256 + s_mdbs_parse.pbuf[8];
  dwPacketNum = s_mdbs_parse.pbuf[9]*256 + s_mdbs_parse.pbuf[10];

  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
  dwTable[dwSendLen++] = MB_FUNC_READ_HOLDING_REGISTER;
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[2];
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[3];
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[4];
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[5];
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[6];
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[7];//�ܰ���
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[8];
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[9];//�ڼ�����
  dwTable[dwSendLen++] = s_mdbs_parse.pbuf[10];
	
	if(dwPacketNum == 1)
		j = 40;
	else 
		j = 0;
	if(dwPacketNum > 1)
	{
		for(i = 0; i < 80; i++)
			dwTable[dwSendLen++] = 0;
		MBASCII_SendMsg(dwTable,dwSendLen);
		return ;
	}
  if(0x05 >= dwPacketSum)//�ܰ���
  {
    if(dwPacketNum < 0x05)
    {
      for(i = 0; i < 10; i++)
      {
		dwTable[dwSendLen++] = Flash_Read_OneByte(AD1_ADDR+UPGRADE_MAEKS+(j+3));
		dwTable[dwSendLen++] = Flash_Read_OneByte(AD1_ADDR+UPGRADE_MAEKS+(j+2));
		dwTable[dwSendLen++] = Flash_Read_OneByte(AD1_ADDR+UPGRADE_MAEKS+(j+1));
		dwTable[dwSendLen++] = Flash_Read_OneByte(AD1_ADDR+UPGRADE_MAEKS+(j+0));
		  
		dwTable[dwSendLen++] = Flash_Read_OneByte(LOAD1_ADDR+UPGRADE_MAEKS+(j+3));
		dwTable[dwSendLen++] = Flash_Read_OneByte(LOAD1_ADDR+UPGRADE_MAEKS+(j+2));
		dwTable[dwSendLen++] = Flash_Read_OneByte(LOAD1_ADDR+UPGRADE_MAEKS+(j+1));
		dwTable[dwSendLen++] = Flash_Read_OneByte(LOAD1_ADDR+UPGRADE_MAEKS+(j+0));
		 j += 4; 
      }
      MBASCII_SendMsg(dwTable,dwSendLen);
    }
  }
}


//**************************************************************************************************
// ����         	: MBASCII_Fun03()
// ��������     	: 2016-09-01
// ����        	 	: ׯ��Ⱥ
// ����         	: Modbus 03���ܴ���
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_Fun03(void)
{
  uint8_t i;
  uint8_t Addrflg;//�Ĵ�����ַ��־ ���ڱ�ʶֻ��һ���ӻ���ַ��Ϣʱ��������֡�Ĵӻ���ַ
  uint8_t ReadAdrH;//�Ĵ�����ַ��8λ
  uint8_t ReadAdrL;//�Ĵ�����ַ��8λ
  uint16_t Data_Buf;//��������
  uint16_t RegisterNum = (uint16_t)s_mdbs_parse.pbuf[4]*256 + s_mdbs_parse.pbuf[5];//�Ĵ�������

  ReadAdrH = s_mdbs_parse.pbuf[2];
  ReadAdrL = s_mdbs_parse.pbuf[3];
        
  s_mdbs_snd.sendlen = 0;
  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0]? UartComm.SlaveAddr : 0x00;
  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = MB_FUNC_READ_HOLDING_REGISTER;//������
  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = RegisterNum * 2;	//���ݳ���
        
  Addrflg = 0;
   
//  //�жϷ��ʼĴ�����ַ�Ƿ�����Ч��Χ��
//  if(!(((ReadAdrL >= ASCII_HOLDING_REG_REGION1_BGEIN) && (ReadAdrL <= ASCII_HOLDING_REG_REGION1_END)
//     && (ReadAdrL + RegisterNum <= (ASCII_HOLDING_REG_REGION1_END + 1))&&(ReadAdrH == UartComm.SlaveAddr))
//     ||((ReadAdrL == ASCII_HOLDING_REG_REGION3_BGEIN) && (RegisterNum == 0x2A)
//     && (RegisterNum*2 == s_mdbs_parse.pbuf[6])&&(ReadAdrH == UartComm.SlaveAddr))
//     ||((ASCII_HOLDING_REG_REGION1_BGEIN == ReadAdrL)&&(1 == RegisterNum)))||(0 == RegisterNum))
//  {
//    //���طǷ����ݵ�ַ
//    MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
//    return;
//  }

  //������������궨����
  if(0x80 == ReadAdrL)
  {
    //��ȡ��Ӧ��
    Read_CALIBAndHeighTable();
    return;
  }
        
  for(long k = 0; k <  RegisterNum; ReadAdrL++, k++)
  {
    switch(ReadAdrL)
    {
    case 0x30://�ӻ���ַ
      Addrflg = 1;
      Data_Buf = UartComm.SlaveAddr;
    break;
            
    case 0x31://������
      Data_Buf = UartComm.BaudRate;
    break;
            
    case 0x32://��żУ��
      Data_Buf = UartComm.Parity;
    break;
            
    case 0x33:
      Data_Buf = 0;
    break;
    
    case 0x34://����ʹ��
      Data_Buf = ProductPara.CompenEn;
    break;    
            
    case 0x35://�˲��ȼ�
      Data_Buf = UserParam.FilterLevel;
    break;
            
    case 0x36://�Զ��ϴ�����
      Data_Buf = ProductPara.AutoUpLoadTime;
    break;
            
    case 0x37://��������ϵ��K
      Data_Buf = (uint32_t)(UserParam.UserFlowK * 100);
    break;
            
    case 0x38://��������ϵ��B
      Data_Buf = (uint32_t)(UserParam.UserFlowB * 100);
    break;
    
    case 0x39:
      Data_Buf = 0;	//ÿ15s����������ֵ
      break;

    case 0x3A:
      Data_Buf = 0;	//ÿ15s���½�����ֵ
      break; 
	
     case 0x3D:                                                            
       Data_Buf = 0x01;                            
       break;
   
         case 0x3F:                                                            //��������/�غ�����
            Data_Buf = 0;                                       
            break;

        case 0x40:                                                            //���ط�ֵ                
            Data_Buf = 0;			                
            break;

        case 0x41:                                                            //���ط�ֵƫ��
            Data_Buf = 0;
            break;

        case 0x42:                                                            //���ز�������
            Data_Buf = 0;
            break;

        case 0x43:                                                            //���ط�ֵ
            Data_Buf = 0;
            break;

        case 0x44:                                                            //���ط�ֵƫ��
            Data_Buf = 0;
            break;

        case 0x45:                                                            //���ط�ֵ
            Data_Buf = 0;
            break;

        case 0x46:                                                            //���ط�ֵƫ��
            Data_Buf = 0;
            break;

        case 0x47:                                                              
            Data_Buf = 0;                                       //������ֵ
            break;

        case 0x48:                                                              
            Data_Buf = 0;                               //������ֵƫ��
            break;

        case 0x49:                                                              
            Data_Buf = 0;
            break;

        case 0x4A:                                                             
            Data_Buf = 0;
            break;

        case 0x4B:                                                             
            Data_Buf = 0;
            break;

        case 0x4C:                                                             
            Data_Buf = 0;
            break;

        case 0x4D:                                                             
            Data_Buf = 0;
            break;

        case 0x4E:                                                             
            Data_Buf = 0;
            break;

        case 0x4F:                                                             
            Data_Buf = 0;
            break;            
		case 0x50: 		//�˲�ʱ��
			Data_Buf = Flash_Read_fourByte(FIlTERING_TIME+UPGRADE_MAEKS);
            break;  			
        case 0x60:                                                            //���ģʽ 0-RTU��1-ASCII
            Data_Buf = 0;				        
            break;
            
        case 0x70:                                                            
            Data_Buf = 0;				        
            break; 
            
        case 0x71:                                                            
            Data_Buf = 0;				        
            break;

        case 0x72:                                                            
            Data_Buf = 0;				        
            break;

        case 0x73:                                                            
            Data_Buf = 0;				        
            break;      
//		 case 0x7178:     //��ȡ�궨�ܰ���
//                     
//            I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_PACKS, 2, uBuf); //д����ձ�
//            //Data_Buf = (u16)uBuf[0] * 256 + uBuf[1];
//            Data_Buf = ArrayToUshort(uBuf);
//            break;
            
//        case 0x7180:                                                            //��ȡ �궨�� ���50��ԭʼADֵ�ͳ����غ����� 
//            s_TotalPacks = (u16)(UART1_RXBuff[7] * 256 + UART1_RXBuff[8]);
//            s_CurrentPack = (u16)(UART1_RXBuff[9] * 256 + UART1_RXBuff[10]);
//            if((s_TotalPacks > 5) || (s_CurrentPack > 4) || (s_CurrentPack >= s_TotalPacks)
//               || (UART1_RXBuff[6] != Register_Num * 2)/* || (s_CurrentPack >= RunVar.TotalPacks)*/)
//            {
//                MBASC_SendErr(MB_EX_ILLEGAL_DATA_VALUE);
//                return;
//            }
//            I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_ADDR(s_CurrentPack), 80, SendBuf + 11);
//            memcpy(SendBuf, UART1_RXBuff, 11);
//            MBASC_SendMsg(SendBuf, 91);
//            return;
    default:
      Data_Buf = 0;
    break;
    }
          
    for(i = 2; i > 0; i--)
    {
      s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = (uint8_t)(Data_Buf >> ((i - 1) * 8));
    }
  }
  //ֻ��һ���ӻ���ַ��Ϣ����Ӧ      
  if((1 == Addrflg)&&(2 == s_mdbs_snd.psndbuf[2]))
  {
    MBASCII_SendMsg(s_mdbs_snd.psndbuf,s_mdbs_snd.sendlen);
  }
  //�������Ϣ���������ݵ���Ӧ
  else
  {
    MBASCII_SendMsg(s_mdbs_snd.psndbuf,s_mdbs_snd.sendlen);
  }
}

//**************************************************************************************************
// ����         	: MBASCII_Fun04()
// ��������     	: 2016-09-01
// ����        	 	: ׯ��Ⱥ
// ����         	: Modbus 04���ܴ���
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************
bool Read_Add_Sub_FLag = false;                                        //�����ݱ�ʶ
void MBASCII_Fun04(void)
{
    uint8_t i;
    uint8_t ReadAdrH;//�Ĵ�����ַ��8λ
    uint8_t ReadAdrL;//�Ĵ�����ַ��8λ
    uint32_t Data_Buf;//��������

    uint16_t RegisterNum = (uint16_t)s_mdbs_parse.pbuf[4]*256 + s_mdbs_parse.pbuf[5];//�Ĵ�������


    ReadAdrH = s_mdbs_parse.pbuf[2];
    ReadAdrL = s_mdbs_parse.pbuf[3];

    s_mdbs_snd.sendlen = 0;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;//�ӻ���ַ
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = MB_FUNC_READ_INPUT_REGISTER;//������
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = RegisterNum * 2;//���ݳ���

    //�����ȡ��Χ���
    if(!(((ReadAdrL <= ASCII_INPUT_REG_REGION1_END)
    && ((ReadAdrL + RegisterNum) <= (ASCII_INPUT_REG_REGION1_END + 2))))
    ||(0 == RegisterNum)||(0 != (RegisterNum & 0x01))|| (0 != (ReadAdrL & 0x01))||(ReadAdrH != UartComm.SlaveAddr))
    {
        //���طǷ����ʵ�ַ
        MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

    for(long k = 0; k < RegisterNum; ReadAdrL += 2, k += 2)
    {
        switch(ReadAdrL)
        {
            case 0x00:                                                            //����״̬��01-���أ�02-���أ�03-���أ�04-װ�أ�05-ж��(����)��
				Data_Buf = 0;            	                                
				break;

			case 0x02:                                                            //����
				Data_Buf = 0;                                                   	                                        
				break;

			case 0x04:                                                            //�����غ�����                                                                                   
				Data_Buf = ton*10;		                                
				break;

			case 0x06:                                                            //�������ֵ
				Data_Buf = 0;
				break;

			case 0x08:                                                            //ԭʼ�˲�ADֵ
				Data_Buf = FilterAdc;	                                				                        //NC
				break;
			
			case 0x10:   //X                                                             //X��Ƕ�(0.1��)
                Data_Buf = (u32)X_Angle*10;
                break;
			case 0x12:      //Y                                                          //Y��Ƕ�(0.1��)
                Data_Buf = (u32)Y_Angle*10;
                break;
			case 0x14:       //Z                                                         //Z��Ƕ�(0.1��)
                Data_Buf = (u32)Z_Angle*10;
                break;
			
			case 0x16:
				Data_Buf =(u32)(X_G*10);
				break;
			
			case 0x18:
				Data_Buf =(u32)(Y_G*10);
				break;
			
			case 0x1A:
				Data_Buf =(u32)(Z_G*10);
				break;
			
			case 0x0A:                                                            //�������		                                
//            myOutData = Runing(g_wFADValue); 
				Data_Buf = 0; 					                       
				break;

			case 0x0C:
				Data_Buf = 0;                        
				break;

			case 0x0E:
				Data_Buf = 0;                             
				break;

			default:
				break;
        } 
        for(i = 4; i > 0; i--)
        {
            s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = (uint8_t)(Data_Buf >> ((i - 1) * 8));
        }
    }
    //Modbus������Ϣ      
    MBASCII_SendMsg(s_mdbs_snd.psndbuf,s_mdbs_snd.sendlen);
}

//**************************************************************************************************
// ����         	: MBASCII_Fun05()
// ��������     	: 2016-09-01
// ����        	 	: ׯ��Ⱥ
// ����         	: Modbus 05���ܴ���
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_Fun05(void)
{
  uint8_t ReadAdrH;//�Ĵ�����8λ��ַ
  uint8_t ReadAdrL;//�Ĵ�����8λ��ַ
  
  ReadAdrH = s_mdbs_parse.pbuf[2];
  ReadAdrL = s_mdbs_parse.pbuf[3];
  
  s_mdbs_snd.sendlen = 0;
  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = MB_FUNC_WRITE_SINGLE_COIL;
  if((ReadAdrL < ASCII_SINGLE_COIL_ADDR_BGEIN) || (ReadAdrL > ASCII_SINGLE_COIL_ADDR_END) || (ReadAdrH != UartComm.SlaveAddr))
  {
    MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
    return;
  }

  if(!(((s_mdbs_parse.pbuf[4] == 0xFF) && (s_mdbs_parse.pbuf[5] == 0x00))
            || ((s_mdbs_parse.pbuf[4] == 0x00) && (s_mdbs_parse.pbuf[5] == 0x00))))
  {
    MBASCII_SendErr(MB_EX_ILLEGAL_DATA_VALUE);
    return;
  }
  
  switch(ReadAdrL)
  {
  case 0x50:
    if((s_mdbs_parse.pbuf[4] == 0x00) && (s_mdbs_parse.pbuf[5] == 0x00))//�궨��Һλ
    {
		ProductPara.CapMin = RunVar.RawCap;
//		UserApp_ParaArrayUpdata(CAPMIN,(uint8_t*)&RunVar.RawCap,4, false);
    }
    else if((s_mdbs_parse.pbuf[4] == 0xFF) && (s_mdbs_parse.pbuf[5] == 0x00))//�궨��Һλ
    {
		ProductPara.CapMax = RunVar.RawCap;
//		UserApp_ParaArrayUpdata(CAPMAX,(uint8_t*)&RunVar.RawCap,4, false);
    }
	
    //CapCalibrate();
////    if((s_mdbs_parse.pbuf[4] == 0x00) && (s_mdbs_parse.pbuf[5] == 0x00))//�궨��Һλ
////    {
////      Cur_Param[CAPMIN] = (unsigned char)(RunVar.RawCap >> 24);
////      Cur_Param[CAPMIN+1] = (unsigned char)(RunVar.RawCap >> 16);
////      Cur_Param[CAPMIN+2] =(unsigned char)(RunVar.RawCap >> 8);
////      Cur_Param[CAPMIN+3] = (unsigned char)RunVar.RawCap;
////      RunVar.CalState |= CalibrateLow;
////    }
////    else if((s_mdbs_parse.pbuf[4] == 0xFF) && (s_mdbs_parse.pbuf[5] == 0x00))//�궨��Һλ
////    {
////      Cur_Param[CAPMAX] = (unsigned char)(RunVar.RawCap >> 24);
////      Cur_Param[CAPMAX+1] = (unsigned char)(RunVar.RawCap >> 16);
////      Cur_Param[CAPMAX+2] =(unsigned char)(RunVar.RawCap >> 8);
////      Cur_Param[CAPMAX+3] = (unsigned char)RunVar.RawCap;
////      RunVar.CalState |= CalibrateHeight;
////    }
////    CapCalibrate();
    break;
    
  case 0x51:
////    if ((s_mdbs_parse.pbuf[4] == 0xFF) && (s_mdbs_parse.pbuf[5] == 0x00))
////    {
////      Cur_Param[0] = User_Default_Param[0] + 1;//�ָ���������
////      I2C1_WNBytesMul3T(EEPROM_ADDRESS, RUN_ADDR_BASE, 1, Cur_Param);
////#if IWDG_EN > 0
////            IWDG_ReloadCounter();
////#endif
////            Rd_Dev_Param();
////#if IWDG_EN > 0
////            IWDG_ReloadCounter();
////#endif
////    }
    break;
    
  case 0x52://�����豸

    break;
    
  case 0x53:
////    if ((s_mdbs_parse.pbuf[4] == 0xFF) && (s_mdbs_parse.pbuf[5] == 0x00))
////    {
////      ProductPara.bFlashWEn = FLASH_WR_ENABLE;//Flashдʹ��
////    }
////    else
////    {
////      ProductPara.bFlashWEn = FLASH_WR_DISABLE;//Flashд��ֹ
////    }
    break;
    
  default:
    break;
  }
  //Modbus���Ͳ����ɹ�
  MBASCII_SendMsg(s_mdbs_snd.psndbuf, 6);
}

//**************************************************************************************************
// ����         	: Write_CALIBAndHeighTable()
// ��������     	: 2015-07-13
// ����        	 	: ׯ��Ⱥ 
// �޸���               : ׯ��Ⱥ
// ����         	: �����͸߶�ADֵ������������Ӧ��
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	: ֡��ʽ
//**************************************************************************************************

void Write_CALIBAndHeighTable(void)
{
  uint8_t opara[4];
  uint16_t i = 11,j;
  uint16_t PacketSum;//�ܰ���
  uint16_t PacketNum;//�����
  uint16_t RegisterNum;//�Ĵ�������
  uint32_t Oiltmp;
  uint8_t temp = 0xFF;
	uint8_t count = 0,count1 = 0,count2 = 0;		//�궨ֵ����
	uint8_t count3;
	uint32_t ad,weight;						//ad��ֵ������
	uint8_t error = 0; 					//�����־
  PacketSum = s_mdbs_parse.pbuf[7]*256 + s_mdbs_parse.pbuf[8];//�ܰ���
  PacketNum = s_mdbs_parse.pbuf[9]*256 + s_mdbs_parse.pbuf[10];//�ڼ���
  RegisterNum = (uint16_t)s_mdbs_parse.pbuf[4]*256 + s_mdbs_parse.pbuf[5];//�Ĵ�������

	//����ܰ�������5�����ߵ�һ���Ĵ�������С��10
  if((0x05 < PacketSum)||(0x000A > RegisterNum))
  {
    s_mdbs_snd.sendlen = 0;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = MB_FUNC_WRITE_MULTIPLE_REGISTERS;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = ((uint16_t)s_mdbs_parse.pbuf[4]*256 + s_mdbs_parse.pbuf[5]) * 2;

    //���طǷ����ݵ�ַ����
    MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
    return;
  }
	count = (s_mdbs_parse.pbuf[6]-4)/8;	//�ֽ�����ȥ4�ֽ�
	if(count >10)
	  count = 10;
	count1 = Flash_Read_OneByte(SIGN_ADDR+UPGRADE_MAEKS);		//������Ҫ�ӵڼ���д��
	count2 = Flash_Read_OneByte(NOW_ADDR+UPGRADE_MAEKS);		//�����Ѿ��궨����
	
  if((count1 > 21) || (count2 > 21))
	{
		count1 = 0;
		count2 = 0;
	}
  //�ܰ����ҼĴ�������ҪС��42
  if((0x01 <= PacketSum)&&(0x05 >= PacketSum)&&(0x002A >= RegisterNum))
  {
    if(PacketNum < 0x05)
    {
		count3 = count;
		if(count2 != 0)
		{	
			count2--;
			i += 8;
			count --;
		}
		for(;count>0;count--)
		{
			if(count1 > 19)
				count1 = 0;
			ad = s_mdbs_parse.pbuf[i++];
			ad <<= 8;
			ad |= s_mdbs_parse.pbuf[i++];
			ad <<= 8;
			ad |= s_mdbs_parse.pbuf[i++];
			ad <<= 8;
			ad |= s_mdbs_parse.pbuf[i++];
	
			weight = s_mdbs_parse.pbuf[i++];
			weight <<= 8;
			weight |= s_mdbs_parse.pbuf[i++];
			weight <<= 8;
			weight |= s_mdbs_parse.pbuf[i++];
			weight <<= 8;
			weight |= s_mdbs_parse.pbuf[i++];
			if(ad <= 0x0FFF)							//12λADֵ�ڷ�Χ����0X0FFF
			{
				Flash_Write_fourByte((AD1_ADDR+UPGRADE_MAEKS+(count1*0x04)),ad);
				Flash_Write_fourByte((LOAD1_ADDR+UPGRADE_MAEKS+(count1*0x04)),weight);
				count1++;
			}
			else
				error += 1;
		}
//	if(error == 0)
	{
		count2 = count3 + count2 - error;
		if(count2 > 20)
			count2 = 20;
		Flash_Write_OneByte(NOW_ADDR+UPGRADE_MAEKS,count2);		//�Ѿ��궨����
	}
	//if(error == 0)					//���ִ��󲻼�¼����
		Flash_Write_OneByte(SIGN_ADDR+UPGRADE_MAEKS,count1);		//�궨������
	
      MBASCII_SendMsg(s_mdbs_snd.psndbuf,11);
      return;
    }
	
  }
  else//�����ͷ��ʵ�ַ�������
  {
    s_mdbs_snd.sendlen = 0;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = MB_FUNC_WRITE_MULTIPLE_REGISTERS;
    //s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = ((uint16_t)s_mdbs_parse.pbuf[4]*256 + s_mdbs_parse.pbuf[5]) * 2;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x02;

    MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
    return;
  }
}


//**************************************************************************************************
// ����         	: MBASCII_Fun10()
// ��������     	: 2016-09-01
// ����        	 	: ׯ��Ⱥ
// ����         	: Modbus 10���ܴ���
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_Fun10(void)
{
        uint8_t i = 0;
	uint8_t uint8_tTemp[4];
        uint8_t ReadAdrH;
        uint8_t ReadAdrL;
    uint16_t uint16_tTemp;
	uint16_t index = 0;
        uint8_t ComTime;
        
	uint16_t RegisterNum = (uint16_t)s_mdbs_parse.pbuf[4]*256 + s_mdbs_parse.pbuf[5];
	
	ReadAdrH = s_mdbs_parse.pbuf[2];
        ReadAdrL = s_mdbs_parse.pbuf[3];
        
	s_mdbs_snd.sendlen = 0;
    	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
    	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = MB_FUNC_WRITE_MULTIPLE_REGISTERS;
    	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = RegisterNum * 2;

        //�����ȡ��Χ���
    if (!(((ReadAdrL >= ASCII_MUL_REG_REGION1_BGEIN) && (ReadAdrL <= ASCII_MUL_REG_REGION1_END)
            && (ReadAdrL + RegisterNum <= (ASCII_MUL_REG_REGION1_END + 1)))
            //|| ((ReadAdrL >= ASCII_MUL_REG_REGION2_BGEIN) && (ReadAdrL <= ASCII_MUL_REG_REGION2_END)
            //    && (ReadAdrL + RegisterNum <= (ASCII_MUL_REG_REGION2_END + 1)))
            || ((ReadAdrL == ASCII_MUL_REG_REGION3_BGEIN))//&&(0x000A <= RegisterNum)&&(0x002A >= RegisterNum))
            || ((ReadAdrL >= ASCII_MUL_REG_EXTERN0_BEGIN)&&(ReadAdrL <= ASCII_MUL_REG_EXTERN0_END)
                &&(ReadAdrL + RegisterNum <= (ASCII_MUL_REG_EXTERN0_END + 1)))                                  //�Ź��㲥��ַ
            )||(0 == RegisterNum)||((RegisterNum * 2) != s_mdbs_parse.pbuf[6])||((ReadAdrH != UartComm.SlaveAddr) && (ReadAdrH != 0)))
    	{
		MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
	       return;
	}

               //д�궨����
    if(0x80 == ReadAdrL)
    {
            //uprintf("receive OK!\r\n");
          Write_CALIBAndHeighTable();
          return;
      } 
	if((ReadAdrL != 0x34) && (RegisterNum != 0x1c))
	for (long k = 0; k < RegisterNum; ReadAdrL++, k++)
	{
          uint16_tTemp = (uint16_t)s_mdbs_parse.pbuf[7+index] * 256 + s_mdbs_parse.pbuf[8+index];
          
          switch(ReadAdrL)
          {
          case 0x30://�豸��ַ
            UartComm.SlaveAddr = s_mdbs_parse.pbuf[8 + index];
//			if( UartComm.SlaveAddr > 0x44 || UartComm.SlaveAddr < 0x41)
//				UartComm.SlaveAddr = 0x41;
			UserApp_ParaArrayUpdata( SLAVE_ADDR, &UartComm.SlaveAddr, 1, true );
            break;
          case 0x31://������
            if((s_mdbs_parse.pbuf[8 + index] < 0x01)||(s_mdbs_parse.pbuf[8 + index] > 0x07))
            {
				UartComm.BaudRate = 0x03;
            }
            else
            {
              UartComm.BaudRate = s_mdbs_parse.pbuf[8+index];
            }
			UserApp_ParaArrayUpdata( BAUDRATE, &UartComm.BaudRate, 1, true );
            break;
            
          case 0x32://��żУ��
            if((s_mdbs_parse.pbuf[8 + index] < 0x01)||(s_mdbs_parse.pbuf[8 + index] > 0x03))
            {
              UartComm.Parity = 0x03;
            }
            else
            {
              UartComm.Parity = s_mdbs_parse.pbuf[8+index];
            }
			UserApp_ParaArrayUpdata( PARITY, &UartComm.Parity, 1, true );
            break;
          case 0x34://����ʹ��
            if((0x00 < s_mdbs_parse.pbuf[8+index]) && (0x03 > s_mdbs_parse.pbuf[8+index]))
            {
				ProductPara.CompenEn = s_mdbs_parse.pbuf[8+index];
            }
			UserApp_ParaArrayUpdata( COMPENSATE, &ProductPara.CompenEn, 1, false );
            break;
          case 0x35://�˲�ϵ��
            if((0x00 < s_mdbs_parse.pbuf[8+index]))
            {
				UserParam.FilterLevel = s_mdbs_parse.pbuf[8+index];
				Switch_Fiter(UserParam.FilterLevel);
            }
			UserApp_ParaArrayUpdata( FILTER_LEVEL, &UserParam.FilterLevel, 1, false );
            break;
          case 0x36://�Զ��ϴ�����
            ProductPara.AutoUpLoadTime = s_mdbs_parse.pbuf[8+index];
            if ((ProductPara.AutoUpLoadTime < 0x01)||(ProductPara.AutoUpLoadTime > 0x04))
            {
              ProductPara.AutoUpLoadTime = 0x01;
            }
			if(ProductPara.AutoUpLoadTime > 1)
			GD32F103_TimerSet( eTmr_AutoLoadTimer, UserApp_AutoLoadProcess, NULL, (ProductPara.AutoUpLoadTime * 1000) );
			UserApp_ParaArrayUpdata( AUTO_UPLOAD_TIME, &ProductPara.AutoUpLoadTime, 1, false );
            break;
            
          case 0x37://����ϵ��K
            UserParam.UserFlowK = uint16_tTemp / 100.0;
            floatToHexArray(UserParam.UserFlowK, uint8_tTemp, 4);
			UserApp_ParaArrayUpdata( USER_FLOW_K, uint8_tTemp, 4, false );
            break;
            
          case 0x38://����ϵ��B
            UserParam.UserFlowB = uint16_tTemp / 100.0;
            floatToHexArray(UserParam.UserFlowB, uint8_tTemp, 4);
			UserApp_ParaArrayUpdata( USER_FLOW_B, uint8_tTemp, 4, false );
            break;
            
		case 0x39:	//����						                
            index += 2;
            break;

        case 0x3A:	//����						                
            index += 2;
            break;            

        case 0x3B:	//����						                
            index += 2;
            break; 

        case 0x3C:	//����						                
            index += 2;
            break; 
            
        case 0x3D:                                                             //������λ
//             uTemp = (u8)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//             if(uTemp != 0xFFFF)
//             {
//                RunVar.Weight_Unit = uTemp;
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, WEIGHT_UNIT, 1, &RunVar.Weight_Unit);
//             }
//            index += 2;
            break; 
            
        case 0x3E:	//����					                
            index += 2;
            break; 
            
        case 0x3F:						                //��������/�غ�����
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.VehicleWeight = uTemp;
//                UshortToArray(RunVar.VehicleWeight, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, VEHICLE_WEIGHT, 2, u8Temp);
//            }
//            index += 2;
            break;

        case 0x40:						                //���ط�ֵ
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.OverloadLimit = uTemp;
//                UshortToArray(RunVar.OverloadLimit, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, OVERLOAD_LIMIT, 2, u8Temp);
//            }
//            index += 2;
            break; 

        case 0x41:						                //���ط�ֵƫ��
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.OverloadLimitDevation = uTemp;
//                UshortToArray(RunVar.OverloadLimitDevation, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, OVERLOAD_LIMIT_DEVIATION, 2, u8Temp);  
//            }
//            index += 2;
            break;

        case 0x42:						                //���ز�������
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.LoadMeasureScheme = uTemp;
//                if((RunVar.LoadMeasureScheme != 1) && (RunVar.LoadMeasureScheme != 2))
//                {
//                    RunVar.LoadMeasureScheme = 2;
//                }
//                UshortToArray(RunVar.LoadMeasureScheme, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_MEASURE_SCHEME, 2, u8Temp); 
//            }
//            index += 2;
            break;

        case 0x43:						                //���ط�ֵ
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.LoadLimit = uTemp;
//                UshortToArray(RunVar.LoadLimit, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_LIMIT, 2, u8Temp);  
//            }
//            index += 2;
            break;

        case 0x44:						                //���ط�ֵƫ��
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.LoadLimitDevation = uTemp;
//                UshortToArray(RunVar.LoadLimitDevation, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_LIMIT_DEVIATION, 2, u8Temp); 
//            }
//            index += 2;
            break;

        case 0x45:						                //���ط�ֵ
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.EmptyLimit = uTemp;
//                UshortToArray(RunVar.EmptyLimit, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, EMPTYLOAD_LIMIT, 2, u8Temp);
//            }            
//            index += 2;
            break;

        case 0x46:						                //���ط�ֵƫ��
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.EmptyLimitDevation = uTemp;
//                UshortToArray(RunVar.EmptyLimitDevation, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, EMPTYLOAD_LIMIT_DEVIATION, 2, u8Temp);
//            }
//            index += 2;
            break;

        case 0x47://������ֵ
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.LightLimit = uTemp;
//                UshortToArray(RunVar.LightLimit, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LIGHTLOAD_LIMIT, 2, u8Temp); 
//            }
//            index += 2;
            break;

        case 0x48:	//������ֵƫ��				                
//            uTemp = (u16)(UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index]);
//            if(uTemp != 0xFFFF)
//            {
//                RunVar.LightLimitDevation = uTemp;
//                UshortToArray(RunVar.LightLimitDevation, u8Temp);
//                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LIGHTLOAD_LIMIT_DEVIATION, 2, u8Temp);
//            }
//            index += 2;            
            break;

        case 0x49:						                
            index += 2;
            break;

        case 0x4A:                                                             
            index += 2;
            break;                                                                                      
            
         case 0x4B:                                                             
            index += 2;
            break;           
            
        case 0x4C:                                                             
            index += 2;
            break;
            
        case 0x4D:                                                             
            index += 2;
            break;

        case 0x4E:                                                             
            index += 2;
            break;
            
        case 0x4F:                                                             
            index += 2;
            break;
        case 0x50: 		//�˲�ʱ��
			if(s_mdbs_parse.pbuf[8+index] > 250)
				Flash_Write_OneByte(FIlTERING_TIME+UPGRADE_MAEKS,0x23);
			else
				Flash_Write_OneByte(FIlTERING_TIME+UPGRADE_MAEKS,s_mdbs_parse.pbuf[8+index]);
			 index += 2;
            break;
        case 0x60:                                                            //���ģʽ 0-RTU��1-ASCII
//            ProductPara.cyMbtWorkType = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
//            if((ProductPara.cyMbtWorkType != 0) && (ProductPara.cyMbtWorkType != 1))
//                ProductPara.cyMbtWorkType = 1;
//            I2C1_WNBytesMul3T(EEPROM_ADDRESS, MBT_WORK_TYPE, 1, &ProductPara.cyMbtWorkType);
//            index += 2;				        
            break;

        case 0x70:						                //���ٶ�
//            Speed = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
//            myOutData = Runing(g_wRADValue);                                    //����һ���ٶȼ��һ������
//            index += 2;
            break;
            
        case 0x71:
//            Acc_Flag = UART1_RXBuff[7+index] * 256 + UART1_RXBuff[8+index];
//            if((Acc_Flag == 1) && (Acc_Flag_Cnt++ >= 4))
//            {
//                Acc_Flag_Cnt = 0;
//                RunVar.AdcMaxCnt = 20;
//            }
//            else
//            {
//                RunVar.AdcMaxCnt = ProductPara.cyFilterLevel * 90;
//            }
            break;
            
        case 0x72:
            break;

        case 0x73:
            break;             

//        case 0x7180: /*������5�飬ÿ��10�����ݣ�ÿ������4���ֽ�*/                
//            s_TotalPacks = ArrayToUshort(UART1_RXBuff + 7);
//            s_CurrentPack = ArrayToUshort(UART1_RXBuff + 9);
//            if((s_TotalPacks > 5) || (s_CurrentPack > 4) || (s_CurrentPack >= s_TotalPacks) 
//               || (Register_Num < 6) || (Register_Num > 42))
//            {
//                MBASC_SendErr(MB_EX_ILLEGAL_DATA_VALUE);
//                return;
//            }          
//            
//            for(i = 0; i < 10; i++)
//            {
//                if((HexToUlong(UART1_RXBuff + 8 * i + 11) != 0xFFFFFFFF)
//                   && (HexToUlong(UART1_RXBuff + 8 * i + 15) != 0xFFFFFFFF))    //�������ݲ�Ϊ0xFFFFFFFF��д������
//                {
//                    AD_Buff[s_CurrentPack][i] = HexToUlong(UART1_RXBuff + 8 * i + 11);
//                    Weight_Buff[s_CurrentPack][i] = HexToUlong(UART1_RXBuff + 8 * i + 15);
//                }
//                else
//                {
//                    for(j = i; j < 10; j++)
//                    {
//                        AD_Buff[s_CurrentPack][j] = 0xFFFFFFFF;
//                        Weight_Buff[s_CurrentPack][j] = 0xFFFFFFFF;
//                    }
//                    break;
//                }
//            }
//            if(s_CurrentPack == 0)                                              //��0��ʱд���ܰ���
//            {
//                UshortToArray(s_TotalPacks, u8Temp);
//                I2C1_WriteNBytes(EEPROM_ADDRESS, AD_WEIGHT_PACKS, 2, u8Temp);
//            }
//            memmove(uBuf, (u8*)(&UART1_RXBuff[11]), i * 8);                     //ǰ��д����յ�������
//            if(i < 10)
//            {
//                memset(uBuf + i * 8, 0xFF, (10 - i) * 8);                       //����д��FF
//            }
//            I2C1_WriteNBytes(EEPROM_ADDRESS, AD_WEIGHT_ADDR(s_CurrentPack), 80, uBuf); //д����ձ�
//            MBASC_SendMsg(UART1_RXBuff, 11);                                    //Ӧ��ǰ11���ֽ� 
//            return;

        default:
            break;
          }
          
          index += 2;
        }
        
        for(i = 0; i < 6; i++)
        {
          s_mdbs_snd.psndbuf[i] = s_mdbs_parse.pbuf[i];
        }
        
        MBASCII_SendMsg(s_mdbs_snd.psndbuf,6);
}

//**************************************************************************************************
// ����         	: MBASCII_Fun25()
// ��������     	: 2016-10-20
// ����        	 	: ׯ��Ⱥ
// ����         	: Modbus 25���ܴ���
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************
#define	MODBUS_LOGIC_TRUE	0xFF00	//Xieyuyin defined 2021.01.26
void MBASCII_Fun25(void)
{
    uint8_t ReadAdrH;
    uint8_t ReadAdrL;
	uint16_t logicv;
	float fTemp = 0;
    
	logicv = (s_mdbs_parse.pbuf[4] << 8) | s_mdbs_parse.pbuf[5];
	
    ReadAdrH = s_mdbs_parse.pbuf[2];
    ReadAdrL = s_mdbs_parse.pbuf[3];
    
    s_mdbs_snd.sendlen = 0;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x25;

    //���������Χ���

    if ((ReadAdrL > ASCII_SPECICAL_COIL_REGION1_END) || (ReadAdrH != UartComm.SlaveAddr))
    {
        MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

    if (!(((s_mdbs_parse.pbuf[4] == 0xFF) && (s_mdbs_parse.pbuf[5] == 0x00))
            || ((s_mdbs_parse.pbuf[4] == 0x00) && (s_mdbs_parse.pbuf[5] == 0x00))))
    {
        MBASCII_SendErr(MB_EX_ILLEGAL_DATA_VALUE);
        return;
    }



    switch(ReadAdrL)
    {
    case 0x00:
		if((s_mdbs_parse.pbuf[4] == 0x00) && (s_mdbs_parse.pbuf[5] == 0x00))//�궨��Һλ
		{
//5			JHM1203_Set_P_CaliValue( 0, LOW_LEVEL );
			//UserApp_ParaArrayUpdata(DEF_CAPMIN,(uint8_t*)&RunVar.RawCap,4, false);
		}
		else if((s_mdbs_parse.pbuf[4] == 0xFF) && (s_mdbs_parse.pbuf[5] == 0x00))//�궨��Һλ
		{
//5			JHM1203_Set_P_CaliValue( 100000, HIGH_LEVEL );
			//UserApp_ParaArrayUpdata(DEF_CAPMAX,(uint8_t*)&RunVar.RawCap,4, false);
		}
////      if((s_mdbs_parse.pbuf[4] == 0x00) && (s_mdbs_parse.pbuf[5] == 0x00))//�궨��Һλ
////      {
////        Cur_Param[CAPMIN] = (unsigned char)(RunVar.RawCap >> 24);
////        Cur_Param[CAPMIN+1] = (unsigned char)(RunVar.RawCap >> 16);
////        Cur_Param[CAPMIN+2] =(unsigned char)(RunVar.RawCap >> 8);
////        Cur_Param[CAPMIN+3] = (unsigned char)RunVar.RawCap;
////        RunVar.CalState |= CalibrateLow;
////      }
////      else if((s_mdbs_parse.pbuf[4] == 0xFF) && (s_mdbs_parse.pbuf[5] == 0x00))//�궨��Һλ
////      {
////        Cur_Param[CAPMAX] = (unsigned char)(RunVar.RawCap >> 24);
////        Cur_Param[CAPMAX+1] = (unsigned char)(RunVar.RawCap >> 16);
////        Cur_Param[CAPMAX+2] =(unsigned char)(RunVar.RawCap >> 8);
////        Cur_Param[CAPMAX+3] = (unsigned char)RunVar.RawCap;
////        RunVar.CalState |= CalibrateHeight;
////      }
////      CapCalibrate();
      break;
	case 0x01:
	//5	fTemp = JHM1203_Set_T_CaliValue( CALIB_R120 );
	//	UserApp_ParaArrayUpdata(T_CALIBRATE_K_ADD, (uint8_t*)&fTemp, 4, false);
		break;
	
	case 0x3F:
	{
		if (logicv == MODBUS_LOGIC_TRUE)
		{
			
		}
		break;
	}
	
    case 0x40:
      //NVIC_SystemReset( );
      break;
      
    default:
      break;
    }
    
    MBASCII_SendMsg(s_mdbs_snd.psndbuf, 6);
}

//**************************************************************************************************
// ����         	: MBASCII_Fun26()
// ��������     	: 2016-10-20
// ����        	 	: ׯ��Ⱥ
// ����         	: Modbus 26���ܴ���
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_Fun26(void)
{
    uint8_t i;
    uint8_t ReadAdrH;
    uint8_t ReadAdrL;
    uint32_t Data_Buf;
    float fTemp = 0;
    
    ReadAdrH = s_mdbs_parse.pbuf[2];
    ReadAdrL = s_mdbs_parse.pbuf[3];
    
    uint16_t Register_Num = (uint16_t)s_mdbs_parse.pbuf[4] * 256 + s_mdbs_parse.pbuf[5];
    
    s_mdbs_snd.sendlen = 0;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x26;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = Register_Num * 2;      //���ݳ���

//    if (!(((ReadAdrL <= ASCII_DB_HOLD_REG_REGION1_END) && (ReadAdrL + Register_Num) <= (ASCII_DB_HOLD_REG_REGION1_END + 2))
//               || ((ReadAdrL >= ASCII_DB_HOLD_REG_REGION2_BEGIN) && (ReadAdrL <= ASCII_DB_HOLD_REG_REGION2_END) && (ReadAdrL + Register_Num) <= (ASCII_DB_HOLD_REG_REGION2_END + 2))
//               || ((ReadAdrL >= ASCII_DB_HOLD_REG_REGION3_BEGIN) &&(ReadAdrL <= ASCII_DB_HOLD_REG_REGION3_END) && (ReadAdrL + Register_Num) <= (ASCII_DB_HOLD_REG_REGION3_END + 2)))
//               || (0 == Register_Num) || (0 != (Register_Num & 0x01)) || (0 != (ReadAdrL & 0x01)) || (ReadAdrH != UartComm.SlaveAddr))
//    {
//        MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
//        return;
//    }
    
    for (long k = 0; k < Register_Num; ReadAdrL += 2, k += 2)
    {
      switch(ReadAdrL)
      {
      case 0x0E:
        fTemp = ProductPara.Floater.Scale[(ReadAdrL - ASCII_DB_HOLD_REG_REGION1_BEGIN) >> 1];
        Data_Buf = *(uint32_t*) & fTemp;
        break;
	  
	 

//	  case 0x6A: 		//ѹ�����ֵ5/ѹ������ֵ5			 25��
//			Data_Buf =	(*((__IO uint16_t*)(CAlIBARRAY+PARA_OFFSET_ADDRESS+36)))<<16;
//			Data_Buf |= *((__IO uint16_t*)(CAlIBARRAY+PARA_OFFSET_ADDRESS+38));  	
//		 break;

      case 0x7A:  
			Data_Buf =	0;
			break;
	  case 0x7C:
			Data_Buf =	0;
			break;
      case 0x7E:
//        fTemp = ProductPara.Floater.DACode[(ReadAdrL -(ASCII_DB_HOLD_REG_REGION1_BEGIN + 0x10)) >> 1];
//        Data_Buf = *(uint32_t*) & fTemp;
		Data_Buf = *(uint32_t*) &RunVar.TempInSensor;	
        break;
        
      case 0x80:
        Data_Buf = *(uint32_t*) &RunVar.RawCap;
        break;
        
      case 0x82:
        Data_Buf = *(uint32_t*) &ProductPara.CapMin;
        break;
        
      case 0x84:
        Data_Buf = *(uint32_t*)&ProductPara.CapMax;
        break;
        
      case 0x86:
        //Data_Buf = *(uint32_t*) & ProductPara.DAMin;
        break;
        
      case 0x88:
        //Data_Buf = *(uint32_t*) & ProductPara.DAMax;
        break;        
        
      case 0x90:
        Data_Buf = *(uint32_t*) & ProductPara.Temp_K;
        break;
	  
      case 0x92:
        Data_Buf = *(uint32_t*) & ProductPara.Temp_B;
        break;
        
      case 0x94:
        Data_Buf = *(uint32_t*) & ProductPara.Temp2_K;
        break;
        
      case 0x96:
        Data_Buf = *(uint32_t*) & ProductPara.Temp2_B;
        break;
        
//      case 0xB6:
//        if(true == DAOilDebug.bDADebugEn)
//        {
//          Data_Buf = *(uint32_t*) & DAOilDebug.usDACode;
//        }
//        else if(true == DAOutDebug.bDADebugEn)
//        {
//          Data_Buf = *(uint32_t*) & DAOutDebug.usDACode;
//        }
//        else
//        {
//          Data_Buf = 0;
//        }
//        break;
        
      default:
        Data_Buf = 0;
        break;
      }
      
      for (i = 4; i > 0; i--)
      {
        s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = (uint8_t)(Data_Buf >> ((i - 1) * 8));
      }
    }

    MBASCII_SendMsg(s_mdbs_snd.psndbuf, s_mdbs_snd.sendlen);
}

//**************************************************************************************************
// ����         	: MBASCII_Fun27()
// ��������     	: 2016-10-20
// ����        	 	: ׯ��Ⱥ
// ����         	: Modbus 27���ܴ���
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_Fun27(void)
{
    uint8_t ReadAdrH;
    uint8_t ReadAdrL;
	int16_t int16_tTemp[2];
	uint32_t uint32_tTemp = 0;
    
    uint16_t Register_Num = (uint16_t)s_mdbs_parse.pbuf[4] * 256 + s_mdbs_parse.pbuf[5];
    uint32_t index = 0;

    ReadAdrH = s_mdbs_parse.pbuf[2];
    ReadAdrL = s_mdbs_parse.pbuf[3];
    
    s_mdbs_snd.sendlen = 0;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x27;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = Register_Num * 2;

    if (!(((ReadAdrL <= ASCII_DB_HOLD_REG_REGION1_END) && (ReadAdrL + Register_Num) <= (ASCII_DB_HOLD_REG_REGION1_END + 2))
               || ((ReadAdrL >= ASCII_DB_HOLD_REG_REGION2_BEGIN) && (ReadAdrL <= ASCII_DB_HOLD_REG_REGION2_END) && (ReadAdrL + Register_Num) <= (ASCII_DB_HOLD_REG_REGION2_END + 2))
               || ((ReadAdrL >= ASCII_DB_HOLD_REG_REGION3_BEGIN) && (ReadAdrL <= ASCII_DB_HOLD_REG_REGION3_END) && (ReadAdrL + Register_Num) <= (ASCII_DB_HOLD_REG_REGION3_END + 2)))
             ||(0 != (Register_Num & 0x01))||(0 == Register_Num)||(0 != (ReadAdrL & 0x01))||((Register_Num * 2) != s_mdbs_parse.pbuf[6])||(ReadAdrH != UartComm.SlaveAddr))
    {
        MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }
	 int16_tTemp[0] = (int16_t)s_mdbs_parse.pbuf[7+index] * 256 + s_mdbs_parse.pbuf[8+index];
	 int16_tTemp[1] = (int16_t)s_mdbs_parse.pbuf[9+index] * 256 + s_mdbs_parse.pbuf[10+index];
    for (long k = 0; k < Register_Num; ReadAdrL += 2, k += 2)
    {
      switch(ReadAdrL)
      {
      case 0x0E:
////        if(DAOilDebug.bDADebugEn == true)
////        {
////          ProductPara.Floater.Scale[(ReadAdrL - ASCII_DB_HOLD_REG_REGION1_BEGIN) >> 1] = (unsigned short)HexToFloat(&s_mdbs_parse.pbuf[7 + index]);
////          I2C1_WNBytesMul3T(EEPROM_ADDRESS, CALIB_FLOATER_SCALE_1 + 2 * ((ReadAdrL - ASCII_DB_HOLD_REG_REGION1_BEGIN) >> 1),
////                                 2, (uint8_t *)&(ProductPara.Floater.Scale[(ReadAdrL - ASCII_DB_HOLD_REG_REGION1_BEGIN) >> 1]));
////        }
        break;
        
      case 0x1E:
////        if(DAOilDebug.bDADebugEn == true)
////        {
////          ProductPara.Floater.DACode[(ReadAdrL - ASCII_DB_HOLD_REG_REGION1_BEGIN - 0x10) >> 1] = (unsigned short)HexToFloat(&s_mdbs_parse.pbuf[7 + index]);
////          I2C1_WNBytesMul3T(EEPROM_ADDRESS, CALIB_FLOATER_DA_1 + 2 * ((ReadAdrL - ASCII_DB_HOLD_REG_REGION1_BEGIN - 0x10) >> 1),
////                                 2, (uint8_t *)&(ProductPara.Floater.DACode[(ReadAdrL -ASCII_DB_HOLD_REG_REGION1_BEGIN - 0x10) >> 1]));
////        }
        break;
	  
    
      case 0x80:
   
        break;
        
      case 0x82:	// low level
//        COMMON_Bits8Convert2Bits32( &uint32_tTemp,&s_mdbs_parse.pbuf[7 + index], BIGENDDIAN );
//        JHM1203_Set_P_CaliValue( uint32_tTemp, LOW_LEVEL );
//		UserApp_ParaArrayUpdata(DEF_CAPMIN, &s_mdbs_parse.pbuf[7 + index], 4, false);
        break;
        
      case 0x84:	//high level
//		COMMON_Bits8Convert2Bits32( &uint32_tTemp,&s_mdbs_parse.pbuf[7 + index], BIGENDDIAN );
//		JHM1203_Set_P_CaliValue( uint32_tTemp, HIGH_LEVEL );
//		UserApp_ParaArrayUpdata(DEF_CAPMAX, &s_mdbs_parse.pbuf[7 + index], 4, false);
        break;
        
      case 0x86:
//		ProductPara.DAMin = HexToFloat(&s_mdbs_parse.pbuf[7 + index]);
//		UserApp_ParaArrayUpdata(DAMIN, &s_mdbs_parse.pbuf[7 + index], 4, false);
        break;
        
      case 0x88:
//        ProductPara.DAMax = HexToFloat(&s_mdbs_parse.pbuf[7 + index]);
//		UserApp_ParaArrayUpdata(DAMAX, &s_mdbs_parse.pbuf[7 + index], 4, false);
        break;        
        
      case 0x90:
	
        break;
        
      case 0x92:
		ProductPara.Temp_B = HexToFloat(&s_mdbs_parse.pbuf[7 + index]);
	
        break;
        
      case 0x94:
		ProductPara.Temp2_K = HexToFloat(&s_mdbs_parse.pbuf[7 + index]);
	
        break;
        
      case 0x96:
        ProductPara.Temp2_B = HexToFloat(&s_mdbs_parse.pbuf[7 + index]);
		
        break;
	  case 0x98:	// low level
        COMMON_Bits8Convert2Bits32( &uint32_tTemp,&s_mdbs_parse.pbuf[7 + index], BIGENDDIAN );
//5        JHM1203_Set_P_CaliValue( uint32_tTemp, LOW_LEVEL );
	
        break;
        
      case 0x9A:	//high level
		COMMON_Bits8Convert2Bits32( &uint32_tTemp,&s_mdbs_parse.pbuf[7 + index], BIGENDDIAN );
//5		JHM1203_Set_P_CaliValue( uint32_tTemp, HIGH_LEVEL );
	
        break;

        
      default:
        break;
      }
      
      index += 4;
    }

    MBASCII_SendMsg(s_mdbs_snd.psndbuf, 6);
}

//**************************************************************************************************
// ����         	: MBASCII_Fun2A()
// ��������     	: 2016-10-20
// ����        	 	: ׯ��Ⱥ
// ����         	: д�汾��Ϣ
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************
//const char SensorSoftVersion[] = "SV1.0.4";		//����汾  
const char SensorSoftVersion[] = "GD1.0.0F";		//����汾 
const char SensorCompany[] = "SOWAY";				//��˾
void MBASCII_Fun2A(void)
{
    uint8_t i,j,objlen;
    uint8_t ReadAdr = s_mdbs_parse.pbuf[3];
    uint8_t Register_Num = s_mdbs_parse.pbuf[5];
    s_mdbs_snd.sendlen = 0;

    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x2A;
    s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[2];
    if(!((s_mdbs_parse.pbuf[2] == UartComm.SlaveAddr)
       &&(ReadAdr >= ASCII_PRODUCTION_INF_BEGIN)
       &&(ReadAdr <= ASCII_PRODUCTION_INF_END)
       &&((ReadAdr + Register_Num) <= (ASCII_PRODUCTION_INF_END + 1))
       &&(0 != Register_Num)&&(0x00 == s_mdbs_parse.pbuf[4])))
    {
      MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
      return;
    }
    j = 6;

    for(i = 0; i < Register_Num; i++)
    {
      switch(ReadAdr)
      {
      case 0xE0://��������
        objlen = s_mdbs_parse.pbuf[j] + 1;
        //I2C1_WriteNBytes(EEPROM_ADDRESS, COMPANY, objlen, &s_mdbs_parse.pbuf[j]);
        j += objlen;
        ReadAdr++;
        break;
        
      case 0xE1://��Ʒ����
        objlen = s_mdbs_parse.pbuf[j] + 1;
//		UserApp_ParaArrayUpdata(DEV_ENCODING, &s_mdbs_parse.pbuf[j], objlen, false);
        j += objlen;
        ReadAdr++;
        break;
        
      case 0xE2://Ӳ���汾
        objlen = s_mdbs_parse.pbuf[j] + 1;
//		UserApp_ParaArrayUpdata(SENSOR_HV, &s_mdbs_parse.pbuf[j], objlen, false);
        j += objlen;
        ReadAdr++;
        break;
        
      case 0xE3://����汾
        objlen = s_mdbs_parse.pbuf[j] + 1;
        //I2C1_WriteNBytes(EEPROM_ADDRESS, SENSOR_SV, objlen, &s_mdbs_parse.pbuf[j]);                                  
        j += objlen;
        ReadAdr++;
        break;
        
      case 0xE4://�豸ID
        objlen = s_mdbs_parse.pbuf[j] + 1;
//		UserApp_ParaArrayUpdata(DEV_ID, &s_mdbs_parse.pbuf[j], objlen, false);
        j += objlen;
        ReadAdr++;
        break;
        
      case 0xE5://�ͻ�����
        objlen = s_mdbs_parse.pbuf[j] + 1;
//		UserApp_ParaArrayUpdata(CUSTOMERCODE, &s_mdbs_parse.pbuf[j], objlen, false);
        j += objlen;
        ReadAdr++;
        break;
        
      default:
        break;
      }
    }

    
    MBASCII_SendMsg(s_mdbs_snd.psndbuf, 6);
}

//**************************************************************************************************
// ����         	: MBASCII_Fun2B()
// ��������     	: 2016-10-20
// ����        	 	: ׯ��Ⱥ
// ����         	: ���汾��Ϣ
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_Fun2B(void)
{
  uint8_t i,objlen;
  uint8_t ReadAdr = s_mdbs_parse.pbuf[3];
  uint8_t Register_Num = s_mdbs_parse.pbuf[5];
  s_mdbs_snd.sendlen = 0;

  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x2B;
  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[4];
  s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[5];
  if(!((s_mdbs_parse.pbuf[2] == UartComm.SlaveAddr)
       &&(ReadAdr >= ASCII_PRODUCTION_INF_BEGIN)
       &&(ReadAdr <= ASCII_PRODUCTION_INF_END)
       &&((ReadAdr + Register_Num) <= (ASCII_PRODUCTION_INF_END + 1))
       &&(0 != Register_Num)&&(0x00 == s_mdbs_parse.pbuf[4])))
  {
    MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
    return;
  }
  for(i = 0; i < Register_Num; i++)
  {
    switch(ReadAdr)
    {
    case 0xE0://��������
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0xE0;
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = strlen(SensorCompany);
		memcpy( &s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen], SensorCompany, strlen(SensorCompany) );
		s_mdbs_snd.sendlen+=strlen(SensorCompany);
      //ReadAdr++;
      break;
      
    case 0xE1://��Ʒ����
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0xE1;
//		UserApp_ReadParaArray(DEV_ENCODING,&objlen,1);
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = objlen;
		if((objlen > (s_mdbs_snd.bufsize/3))||(0 == objlen))
		{
			s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen - 1] = 1;
			s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0;
			break;
		}
//		UserApp_ReadParaArray(DEV_ENCODING+1,&s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen],objlen);
		s_mdbs_snd.sendlen += objlen;
		//ReadAdr++;
		break;
      
    case 0xE2://Ӳ���汾
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0xE2;
//		UserApp_ReadParaArray(SENSOR_HV,&objlen,1);
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = objlen;
      if((objlen > (s_mdbs_snd.bufsize/3))||(0 == objlen))
      {
        s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen - 1] = 1;
        s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0;
        break;
      }
//	  UserApp_ReadParaArray(SENSOR_HV+1,&s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen],objlen);
      s_mdbs_snd.sendlen += objlen;
      //ReadAdr++;
      break;
    
    case 0xE3://����汾
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0xE3;
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = strlen(SensorSoftVersion);
		memcpy( &s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen], SensorSoftVersion, strlen(SensorSoftVersion) );
		s_mdbs_snd.sendlen+=strlen(SensorSoftVersion);
		//ReadAdr++;
		break;
      
    case 0xE4:
      s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0xE4;
//		UserApp_ReadParaArray(DEV_ID,&objlen,1);
      s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = objlen;
      if((objlen > (s_mdbs_snd.bufsize/3))||(0 == objlen))
      {
        s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen - 1] = 1;
        s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0;
        break;
      }
//	  UserApp_ReadParaArray(DEV_ID+1,&s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen],objlen);
      s_mdbs_snd.sendlen += objlen;
      //ReadAdr++;
      break;
      
    case 0xE5:
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0xE5;
//		UserApp_ReadParaArray(CUSTOMERCODE,&objlen,1);
		s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = objlen;
		if((objlen > (s_mdbs_snd.bufsize/3))||(0 == objlen))
		{
			s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen - 1] = 1;
			s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0;
			break;
		}
//		UserApp_ReadParaArray(CUSTOMERCODE+1,&s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen],objlen);
		s_mdbs_snd.sendlen += objlen;
		//ReadAdr++;
		break;
      
    default:
      break;
    }
    
    ReadAdr++;
  }
  
  MBASCII_SendMsg(s_mdbs_snd.psndbuf, s_mdbs_snd.sendlen);
}


//**************************************************************************************************
// ����         	: MBASCII_Fun41()
// ��������     	: 2016-10-20
// ����        	 	: ׯ��Ⱥ
// ����         	: ������������
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_Fun41(void)
{
	//uint8_t Temp = 0xFF;
	uint8_t buf[1] = {0xFF};
	uint16_t ReadAdr = (uint16_t)s_mdbs_parse.pbuf[2] * 256 + s_mdbs_parse.pbuf[3];
	uint16_t DataLen = (uint16_t)s_mdbs_parse.pbuf[4] * 256 + s_mdbs_parse.pbuf[5];

	s_mdbs_snd.sendlen = 0;
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = s_mdbs_parse.pbuf[0] ? UartComm.SlaveAddr : 0x00;
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x41;
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x00;
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x01;

	if(0x0001 != ReadAdr)
	{
		MBASCII_SendErr(MB_EX_ILLEGAL_DATA_ADDRESS);
		return;
	}
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x00;
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x01;

	if(0x0000 != DataLen)
	{
		s_mdbs_snd.psndbuf[6] = 0x01;
		MBASCII_SendMsg(s_mdbs_snd.psndbuf, 7);
		return;
	}
  
	s_mdbs_snd.psndbuf[s_mdbs_snd.sendlen++] = 0x00;
	MBASCII_SendMsg(s_mdbs_snd.psndbuf, 7);
//	while( !usart_flag_get(USART0,USART_FLAG_TC) );
//	UserApp_ParaArrayUpdata( 0, &Temp, 1, false );
	Flash_Write_MultiBytes(UPGRADE_MAEKS,buf, 1);		//������־
   // Delay_ms(20);
	GD32F103_Systick_Delayxms(20);


    NVIC_SystemReset();
}



//**************************************************************************************************
// ����         	: MBASCII_AutoUpLoadFrame()
// ��������   	        : 2016-09-01
// ����         	: ׯ��Ⱥ
// ����         	: �Զ��ϴ�
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void MBASCII_AutoUpLoadFrame( void )
{
	uint8_t i;
	uint32_t pTemp;
        
	s_mdbs_snd.psndbuf[0] = UartComm.SlaveAddr;
	s_mdbs_snd.psndbuf[1] = MB_FUNC_READ_INPUT_REGISTER;
	s_mdbs_snd.psndbuf[2] = 32;

	pTemp = (uint32_t)RunVar.LiquidAD;
       
	for (i = 4; i > 0; i--)
	{
	  s_mdbs_snd.psndbuf[7 - i] = (uint8_t)(pTemp >> ((i - 1) * 8));
	}
	
	pTemp = (uint32_t)((RunVar.TempInSensor + 273.15) * 10);                                           //Һ���¶�
	for(i = 4; i > 0; i--)
	{
	  s_mdbs_snd.psndbuf[11 - i] = (uint8_t)(pTemp >> ((i - 1) * 8));
	}
	
//	pTemp = (uint32_t)((RunVar.TempInAir + 273.15) * 10);                                              //�����¶�
	pTemp = (uint32_t)(RunVar.TempInAir);									//Modified by XieYuyin 2020-11-20
	for(i = 4; i > 0; i--)
	{
	  s_mdbs_snd.psndbuf[15 - i] = (uint8_t)(pTemp >> ((i - 1) * 8));
	}
	

	 pTemp = ProductPara.AddOil;   
	for(i = 4; i > 0; i--)
	{
	  s_mdbs_snd.psndbuf[19 - i] = (uint8_t)(pTemp >> ((i - 1) * 8));                                             //������
	}
	

	pTemp = ProductPara.SubOil;  
	for(i = 4; i > 0; i--)
	{
	  s_mdbs_snd.psndbuf[23 - i] = (uint8_t)(pTemp >> ((i - 1) * 8));                                             //©����
	}
	
	pTemp = RunVar.OilQuantity; 
	for (i = 4; i > 0; i--)
	{
	  s_mdbs_snd.psndbuf[27 - i] = (uint8_t)(pTemp >> ((i - 1) * 8));
	}
	
	
	pTemp = RunVar.LiquidPercent;                                             //Һλ�ٷֱ�
	for (i = 4; i > 0; i--)
	{
	  s_mdbs_snd.psndbuf[31 - i] = (uint8_t)(pTemp >> ((i - 1) * 8));
	}
	
	s_mdbs_snd.sendlen = 35;
//	pTemp = (uint32_t)RunVar.LiquidHeight;                                                        //Һλ�߶�      
//	pTemp = tmpOilRealTimeValue;								//Added by XieYuyin 2020-11-20
	
	for (i = 4; i > 0; i--)
	{
	  s_mdbs_snd.psndbuf[35 - i] = (uint8_t)(pTemp >> ((i - 1) * 8));
	}
        
	MBASCII_SendMsg(s_mdbs_snd.psndbuf, s_mdbs_snd.sendlen);
}
