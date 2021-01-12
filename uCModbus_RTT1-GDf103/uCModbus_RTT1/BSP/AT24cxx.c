#include "AT24cxx.h"
extern volatile uint16_t MB_DATA[125];
u8 const SoftwareVer[] = "SV1.1.15";
u8 const User_Default_Param[PRO_DEFAULT_LEN] =
{
    0x01,								        //������ʼ����־λ
    0x71,								        //�豸��ַ
    0x03,								        //������
    0x03,								        //��żУ��
    0x01,                                                                       //����ʹ��
    0x02,								        //�˲�ϵ��
    0x01,								        //�Զ���������
    0x01,			                                                //����ģʽ
    0x64,				                                        //��������ʱ�� 100->10s
    
    0xFF,0xFF,0xFF,0xFF,                                                        //����ֵ10�����ݻ��� 20 bytes
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    
    0xFF,0xFF,0xFF,0xFF,                                                        //����ֵ10�����ݻ��� 20 bytes
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    
    0x3F,0x80,0x00,0x00,                                                        //�û����ϵ��K    Ĭ��Ϊ1.0
    0x00,0x00,0x00,0x00,                                                        //�û����ϵ��B                                                        
    
    0x06, 0x40,                                                                 //�����ֵ 2 bytes  1600
    0x00,                              	                                        //��¼���� 1 byte
    0x00,                              	                                        //����״̬ 1 byte  LOAD_EMPTY_STATE
    0x02, 0xBC,               			                                //����ϵ�� 4 bytes  0.7 -> 700 �Ŵ�100��
    0x04, 0xB0,             			                                //����ֵ 4 bytes  1.2  -> 1200 �Ŵ�100��
    0x00, 0x00,                                                                 //��С�߶�ֵ 2 bytes  0 
    0x00, 0x64,                                                                 //��СADֵ 2 bytes    100
    0x00, 0x32,                                                                 //�м�߶�ֵ 2 bytes  50
    0x02, 0x58,                                                                 //�м�ADֵ 2 bytes    600
    0x00, 0x64,                                                                 //���߶�ֵ 2 bytes  100
    0x04, 0x4C,                                                                 //���ADֵ 2 bytes    1100
    0x0B, 0xB8,                                                                 //�����ʱ 2 bytes   3000 ��λ 0.1S
    
    0x00, 0x00,                                                                 //��������/�غ�����
    0x00, 0x01,                                                                 //���ز�������
    0x00, 0xC8,                                                                 //���ط�ֵ            200
    0x00, 0x05,                                                                 //���ط�ֵƫ��
    0x00, 0x96,                                                                 //���ط�ֵ            150
    0x00, 0x05,                                                                 //���ط�ֵƫ��
    0x00, 0x32,                                                                 //���ط�ֵ             50
    0x00, 0x05,                                                                 //���ط�ֵƫ��
    0x00, 0x64,                                                                 //���ط�ֵ             100
    0x00, 0x05,                                                                 //���ط�ֵƫ��    
    0x00,
};

u8 Cur_Param[PRO_DEFAULT_LEN] = {0};

ProductParaTypeDef ProductPara;
RunVarTypeDef RunVar;
UartCommTypeDef UartComm;
UserParamTypeDef UserParam;

extern u32 AD_Buff[5][10];                                                             //ADֵ����
extern u32 Weight_Buff[5][10];                                                         //����ֵ����
u8 A_W_Buff[80];

extern u16 g_wDlyMaxTime;                                                      //�����ʱʱ�� 0.1S ��λ
TEST_PARA g_tpTestPar;

void EEProm_Init(void)
{
    IIC_Init();
}

void Check_User_Backup(void)
{
    u32 i = 0, errno1 = 0;
    u8 bak1 = 0, bak2 = 0;
    
    for (i = 0; i < PRO_DEFAULT_LEN; i++)
    {
        errno1 = 0;
        
        bak1 = I2C1_ReadByte(EEPROM_ADDRESS, i + USER_DEFAULT_PARA_BAK1);
        bak2 = I2C1_ReadByte(EEPROM_ADDRESS, i + USER_DEFAULT_PARA_BAK2);
        if (Cur_Param[i] != bak1)//��ֵַ��bak1ֵ��ͬ
            errno1 |= 0x01;

        if (Cur_Param[i] != bak2)//��ֵַ��bak2ֵ��ͬ
            errno1 |= 0x02;

        if (bak1 != bak2)//bak1��bak2ֵ��ͬ
            errno1 |= 0x04;

        if(0x00 == errno1)//����ͬ������
            continue;

        switch (errno1)//errno������ȡ01 02 04��ì��
        {
        case 0x03:////��ֵַ��bak1,bak2ֵ����ͬ ��bak1,bak2ͬ
            Cur_Param[i] = bak1;
            I2C1_WriteByte(EEPROM_ADDRESS, RUN_ADDR_BASE, Cur_Param[i]);
            break;
        case 0x05://��ֵַ��bak2ֵͬ��bak1��ͬ��������
            I2C1_WriteByte(EEPROM_ADDRESS, USER_DEFAULT_PARA_BAK1 + i, Cur_Param[i]);
            break;
        case 0x06://��ֵַ��bak1ֵ��ͬ   bak2��ͬ��������
            I2C1_WriteByte(EEPROM_ADDRESS, USER_DEFAULT_PARA_BAK2 + i, Cur_Param[i]);
            break;
        case 0x07://���߶���ͬ����д��Ĭ��ֵ
            Cur_Param[i] = User_Default_Param[i];
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, RUN_ADDR_BASE + i, 1, &Cur_Param[i]);
            break;
        }
    }    
}


/*******************************************************************************
����         : void Check_Software_Ver(void)
��������     : 2017-02-21
����         : ÷����
����         : ���������°汾�ź�EEPROM��д��İ汾���Ƿ�һ�£���һ��д�����µİ汾��
�������     : ��
�������     : ��
���ؽ��     : ��
ע���˵��   : 
�޸�����     : 
********************************************************************************/
void Check_Software_Ver(void)
{
    u8 OldVerLen = 0, NewVerLen = 0, SVerBuff[30], i;
    NewVerLen = sizeof(SoftwareVer) - 1;                                       //���µİ汾����ĳ���(sizeof�õ��ĳ��Ȱ���'\0',Ҫȥ��)
    OldVerLen = I2C1_ReadByte(EEPROM_ADDRESS, SFVERSION);                       //��EEPROM�ж�ȡ�İ汾����ĳ���
    if((0 == OldVerLen) || (0x14 < OldVerLen) || (OldVerLen != NewVerLen))      //�����ǰûд�����ֽ�������30���������°汾���Ȳ���ȣ���д��汾��Ϣ
    {
        ProductPara.bFlashWEn = FLASH_WR_ENABLE;                                
        I2C1_WriteByte(EEPROM_ADDRESS, SFVERSION, NewVerLen);                   
        I2C1_WriteNBytes(EEPROM_ADDRESS, SFVERSION + 1, NewVerLen, SoftwareVer);
    }
    else
    {
        for (i = 0; i < OldVerLen; i ++)                                           //��ȡEEPROM�д�ŵ�����汾
        {                             
            SVerBuff[i] = I2C1_ReadByte(EEPROM_ADDRESS, SFVERSION + 1 + i);         //SFVERSION��ַ��ŵ��ǣ��汾���ȣ�1�ֽڣ�+�汾��
        }
        if(memcmp(SVerBuff, SoftwareVer, NewVerLen))
        {
            ProductPara.bFlashWEn = FLASH_WR_ENABLE;                                //����ʹ��дEEPROM
            I2C1_WriteByte(EEPROM_ADDRESS, SFVERSION, NewVerLen);                   //�Ѱ汾�ŵĳ���д��EEPROM
            I2C1_WriteNBytes(EEPROM_ADDRESS, SFVERSION + 1, NewVerLen, SoftwareVer);//�Ѱ汾��д��EEPROM
        }
    }
}


void Rd_Dev_Param(void)//��ȡ�豸����  �ϵ��ʼ����
{
    u8 cyBuf[4];
    u32 i, j;
    u16 wTemp;

    I2C1_ReadNBytes(EEPROM_ADDRESS,  USER_DEFAULT_PARA_BAK1 - 5, 4, cyBuf);  //��ȡ4������ֽڡ�
    
    if((User_Default_Param[0] != I2C1_ReadByte(EEPROM_ADDRESS, RUN_ADDR_BASE)) ||	//��EEPROM�����ݣ�д�����ֵ
       (0x11 != cyBuf[0]) || (0x34 != cyBuf[1]) || (0x56 != cyBuf[2]) || (0x7B != cyBuf[3]) )
    {
    	ProductPara.bFlashWEn = FLASH_WR_ENABLE;
        I2C1_WNBytesMul3T(EEPROM_ADDRESS, RUN_ADDR_BASE, PRO_DEFAULT_LEN, User_Default_Param);//��Ĭ�ϲ���д�����ַ�����������ݵ�ַ
        cyBuf[0] = 0x11;
        cyBuf[1] = 0x34;
        cyBuf[2] = 0x56;
        cyBuf[3] = 0x7B;
        for (i = 0; i < 4; i++)
        {
            I2C1_WriteByte(EEPROM_ADDRESS, USER_DEFAULT_PARA_BAK1 - 5 + i, cyBuf[i]);
        }
        ProductPara.bFlashWEn = FLASH_WR_DISABLE;
    }
    
    I2C1_ReadNBytes(EEPROM_ADDRESS, RUN_ADDR_BASE, PRO_DEFAULT_LEN, Cur_Param);//������ַ�����ݶ�ȡ��Cur_Param�С�

    Check_User_Backup();//����ַ�ͱ��ݵ�ַ�������Ƿ�һ�¡�
    Check_Software_Ver();//�������汾��
    
    UartComm.SlaveAddr = I2C1_ReadByte(EEPROM_ADDRESS, SLAVE_ADDR);
	MB_DATA[48] = UartComm.SlaveAddr;
	UartComm.BaudRate = I2C1_ReadByte(EEPROM_ADDRESS, BAUDRATE);
	MB_DATA[49] = UartComm.BaudRate;
    UartComm.Parity = I2C1_ReadByte(EEPROM_ADDRESS, PARITY);
	MB_DATA[50] = I2C1_ReadByte(EEPROM_ADDRESS, PARITY1);
	
    MB_DATA[52] = I2C1_ReadByte(EEPROM_ADDRESS, OFFSET_ENABLE);	//��ʹ�ܲ���
	
	

    ProductPara.cyAutoUpLoadTime = Cur_Param[AUTO_UPLOAD_TIME];
    ProductPara.cyFilterLevel = I2C1_ReadByte(EEPROM_ADDRESS, FILTER_LEVEL);
	MB_DATA[53] = ProductPara.cyFilterLevel;
	if(ProductPara.cyFilterLevel == 0)
		  I2C1_WriteByte(EEPROM_ADDRESS,FILTER_LEVEL, 0x02);
	
    RunVar.AdcMaxCnt = ProductPara.cyFilterLevel * 90;
    ProductPara.cyMbtWorkType = Cur_Param[MBT_WORK_TYPE];
    ProductPara.OffsetEn = Cur_Param[OFFSET_ENABLE];        
    g_cyShovelDTime = Cur_Param[LOAD_SHOVE_DTIME];
    
    UserParam.UserFlowK = HexToFloat(&Cur_Param[USER_FLOW_K]);
    UserParam.UserFlowB = HexToFloat(&Cur_Param[USER_FLOW_B]);
    
    RunVar.VehicleWeight = ArrayToUshort(&Cur_Param[VEHICLE_WEIGHT]);
    RunVar.LoadMeasureScheme = ArrayToUshort(&Cur_Param[LOAD_MEASURE_SCHEME]);
    RunVar.OverloadLimit = ArrayToUshort(&Cur_Param[OVERLOAD_LIMIT]);
    RunVar.OverloadLimitDevation = ArrayToUshort(&Cur_Param[OVERLOAD_LIMIT_DEVIATION]);
    RunVar.LoadLimit = ArrayToUshort(&Cur_Param[LOAD_LIMIT]);
    RunVar.LoadLimitDevation = ArrayToUshort(&Cur_Param[LOAD_LIMIT_DEVIATION]);
    RunVar.EmptyLimit = ArrayToUshort(&Cur_Param[EMPTYLOAD_LIMIT]);
    RunVar.EmptyLimitDevation = ArrayToUshort(&Cur_Param[EMPTYLOAD_LIMIT_DEVIATION]);
    RunVar.Weight_Unit = Cur_Param[WEIGHT_UNIT];
    RunVar.LightLimit = ArrayToUshort(&Cur_Param[LIGHTLOAD_LIMIT]);
    RunVar.LightLimitDevation = ArrayToUshort(&Cur_Param[LIGHTLOAD_LIMIT_DEVIATION]);
    
    I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_PACKS, 1, &((u8*)&RunVar.TotalPacks)[1]);         //��ñ궨������ܰ���
    I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_PACKS + 1, 1, &((u8*)&RunVar.TotalPacks)[0]);
    if((RunVar.TotalPacks == 0) || (RunVar.TotalPacks > 5))
    {
        for(i = 0; i < 5; i++)
        {
            for(j = 0; j < 10; j++)
            {
                AD_Buff[i][j] = 0xFFFFFFFF;
                Weight_Buff[i][j] = 0xFFFFFFFF;
            }
        }
    }
    else
    {
        for(i = 0; i < RunVar.TotalPacks; i++)                                  //��ñ궨�����ֵ
        {
            I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_ADDR(i), 80, A_W_Buff);
            for(j = 0; j < 10; j++)
            {
                AD_Buff[i][j] = HexToUlong(A_W_Buff + 8 * j);                   //ADֵ
                Weight_Buff[i][j] = HexToUlong(A_W_Buff + 8 * j + 4); //����ֵ
            }
        }
        
        for(i = RunVar.TotalPacks; i < 5; i++)   //20190705 δ�궨������Ĭ��Ϊ0xFFFFFFFF ��ֹ��ʼ��Ϊ0
        {
           for(j = 0; j < 10; j++)
            {
                AD_Buff[i][j] = 0xFFFFFFFF;                   //ADֵ
                Weight_Buff[i][j] = 0xFFFFFFFF;                 //����ֵ
            }
        }
    }
  
    /*
    for(i = 0;i<5;i++)
    {
        for(j=0;j<10;j++)
        {
            uprintf("AD_Buff[%d][%d]=%8x\r\n",i,j,AD_Buff[i][j]);
            uprintf("Weight_Buff[%d][%d]=%8x\r\n",i,j,Weight_Buff[i][j]);
        }
    }
   */ 
    
    for (i = 0; i < LOAD_DATA_FIFO_NUM; i++)
    {
        g_wEmptyBuffer[i] = Cur_Param[LOAD_EMPTY_FIFO + i * 2];
        g_wEmptyBuffer[i] <<= 8;
        g_wEmptyBuffer[i] += Cur_Param[LOAD_EMPTY_FIFO + i * 2 + 1];
    }
    
    for (i = 0; i < LOAD_DATA_FIFO_NUM; i++)
    {
        g_wFullBuffer[i] = Cur_Param[LOAD_FULL_FIFO + i * 2];
        g_wFullBuffer[i] <<= 8;
        g_wFullBuffer[i] += Cur_Param[LOAD_FULL_FIFO + i * 2 + 1];
    }
            
    g_wRatedValue = Cur_Param[LOAD_RATED_VALUE];
    g_wRatedValue <<= 8;
    g_wRatedValue += Cur_Param[LOAD_RATED_VALUE + 1];
    
    wTemp = Cur_Param[LOAD_OVER_K];
    wTemp <<= 8;
    wTemp += Cur_Param[LOAD_OVER_K + 1];
    g_fOverValueK = wTemp;
    g_fOverValueK /= 100.0;
    g_cyRecordShovelNum = Cur_Param[LOAD_SHOVEL_NUM];
    g_cyWorkState = Cur_Param[LOAD_WORK_STATE];
    wTemp = Cur_Param[LOAD_VARIANCE_VALUE];
    wTemp <<= 8;
    wTemp += Cur_Param[LOAD_VARIANCE_VALUE + 1];
    g_fVarianceValue = wTemp;
    g_fVarianceValue /= 100.0;

//I2C1_ReadNBytes(EEPROM_ADDRESS, DEV_ID, DEFAULT_DEV_ID_LEN, Dev_Id);
    g_tpTestPar.wMinH = Cur_Param[TEST_MIN_HIGH];
    g_tpTestPar.wMinH <<= 8;
    g_tpTestPar.wMinH += Cur_Param[TEST_MIN_HIGH + 1];

    g_tpTestPar.wMinAdc = Cur_Param[TEST_MIN_ADC];
    g_tpTestPar.wMinAdc <<= 8;
    g_tpTestPar.wMinAdc += Cur_Param[TEST_MIN_ADC + 1];

    g_tpTestPar.wMidH = Cur_Param[TEST_MID_HIGH];
    g_tpTestPar.wMidH <<= 8;
    g_tpTestPar.wMidH += Cur_Param[TEST_MID_HIGH + 1];

    g_tpTestPar.wMidAdc = Cur_Param[TEST_MID_ADC];
    g_tpTestPar.wMidAdc <<= 8;
    g_tpTestPar.wMidAdc += Cur_Param[TEST_MID_ADC + 1];

    g_tpTestPar.wMaxH = Cur_Param[TEST_MAX_HIGH];
    g_tpTestPar.wMaxH <<= 8;
    g_tpTestPar.wMaxH += Cur_Param[TEST_MAX_HIGH + 1];

    g_tpTestPar.wMaxAdc = Cur_Param[TEST_MAX_ADC];
    g_tpTestPar.wMaxAdc <<= 8;
    g_tpTestPar.wMaxAdc += Cur_Param[TEST_MAX_ADC + 1];
    
    g_wDlyMaxTime = Cur_Param[MAX_DLY_TIME];
    g_wDlyMaxTime <<= 8;
    g_wDlyMaxTime += Cur_Param[MAX_DLY_TIME + 1];
	MB_DATA[54] = 1;
	MB_DATA[63] = I2C1_ReadByte(EEPROM_ADDRESS, VEHICLE_WEIGHT);	//��������/�غ�����
	MB_DATA[64] = I2C1_Read2Bytes(EEPROM_ADDRESS,OVERLOAD_LIMIT);	//���ط�ֵ

  	MB_DATA[65] = I2C1_ReadByte(EEPROM_ADDRESS, OVERLOAD_LIMIT_DEVIATION);	//���ط�ֵƫ��
	MB_DATA[66] = I2C1_ReadByte(EEPROM_ADDRESS, LOAD_MEASURE_SCHEME);	//���ز�������
	MB_DATA[67] = I2C1_ReadByte(EEPROM_ADDRESS, LOAD_LIMIT);	//���ط�ֵ
	MB_DATA[68] = I2C1_ReadByte(EEPROM_ADDRESS, LOAD_LIMIT_DEVIATION);	//���ط�ֵƫ��
	MB_DATA[69] = I2C1_ReadByte(EEPROM_ADDRESS, EMPTYLOAD_LIMIT);	//���ط�ֵ
	MB_DATA[70] = I2C1_ReadByte(EEPROM_ADDRESS, EMPTYLOAD_LIMIT_DEVIATION);	//���ط�ֵƫ��
	uint32_t USART_BaudRate = 0,USART_Parity = 0,bits = 8;
	switch (UartComm.BaudRate)
    {
    case 1:
        USART_BaudRate = 2400;
        break;
        
    case 2:
        USART_BaudRate = 4800;
        break;
		
    case 3:                                           //Ĭ��Ϊ9600
        USART_BaudRate = 9600;
        break;

    case 4:
        USART_BaudRate = 19200;
        break;

    case 5:
        USART_BaudRate = 38400;
        break;

    case 6:
        USART_BaudRate = 57600;
        break;

    case 7:
        USART_BaudRate = 115200;
        break;
      
    default:
		 USART_BaudRate = 9600;
        break;

    }
	switch (MB_DATA[50])
    {
    case 1:
        USART_Parity = MODBUS_PARITY_ODD;//��У��λ
		bits = 9;
        break;
    case 2:
        USART_Parity = MODBUS_PARITY_EVEN;//żУ��λ
		bits = 9;
        break;
    case 3:
        USART_Parity = MODBUS_PARITY_NONE;//����żУ��λ
		bits = 8;
        break;        
                                 
    default:
		USART_Parity = MODBUS_PARITY_NONE;//����żУ��λ
		bits = 8;
        break;
    }
	MB_CfgCh(UartComm.SlaveAddr,MODBUS_SLAVE,0,MODBUS_MODE_ASCII,1,USART_BaudRate,bits,USART_Parity,1,MODBUS_WR_EN);

}



