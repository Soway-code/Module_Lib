#include "AT24cxx.h"
extern volatile uint16_t MB_DATA[125];
u8 const SoftwareVer[] = "SV1.1.15";
u8 const User_Default_Param[PRO_DEFAULT_LEN] =
{
    0x01,								        //参数初始化标志位
    0x71,								        //设备地址
    0x03,								        //波特率
    0x03,								        //奇偶校验
    0x01,                                                                       //补偿使能
    0x02,								        //滤波系数
    0x01,								        //自动发送周期
    0x01,			                                                //工作模式
    0x64,				                                        //铲与铲间隔时间 100->10s
    
    0xFF,0xFF,0xFF,0xFF,                                                        //空载值10组数据缓存 20 bytes
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    
    0xFF,0xFF,0xFF,0xFF,                                                        //满载值10组数据缓存 20 bytes
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,
    
    0x3F,0x80,0x00,0x00,                                                        //用户输出系数K    默认为1.0
    0x00,0x00,0x00,0x00,                                                        //用户输出系数B                                                        
    
    0x06, 0x40,                                                                 //额定载重值 2 bytes  1600
    0x00,                              	                                        //记录铲数 1 byte
    0x00,                              	                                        //工作状态 1 byte  LOAD_EMPTY_STATE
    0x02, 0xBC,               			                                //超载系数 4 bytes  0.7 -> 700 放大100倍
    0x04, 0xB0,             			                                //方差值 4 bytes  1.2  -> 1200 放大100倍
    0x00, 0x00,                                                                 //最小高度值 2 bytes  0 
    0x00, 0x64,                                                                 //最小AD值 2 bytes    100
    0x00, 0x32,                                                                 //中间高度值 2 bytes  50
    0x02, 0x58,                                                                 //中间AD值 2 bytes    600
    0x00, 0x64,                                                                 //最大高度值 2 bytes  100
    0x04, 0x4C,                                                                 //最大AD值 2 bytes    1100
    0x0B, 0xB8,                                                                 //最大延时 2 bytes   3000 单位 0.1S
    
    0x00, 0x00,                                                                 //整车重量/载荷重量
    0x00, 0x01,                                                                 //载重测量方案
    0x00, 0xC8,                                                                 //超载阀值            200
    0x00, 0x05,                                                                 //超载阀值偏差
    0x00, 0x96,                                                                 //重载阀值            150
    0x00, 0x05,                                                                 //重载阀值偏差
    0x00, 0x32,                                                                 //空载阀值             50
    0x00, 0x05,                                                                 //空载阀值偏差
    0x00, 0x64,                                                                 //轻载阀值             100
    0x00, 0x05,                                                                 //轻载阀值偏差    
    0x00,
};

u8 Cur_Param[PRO_DEFAULT_LEN] = {0};

ProductParaTypeDef ProductPara;
RunVarTypeDef RunVar;
UartCommTypeDef UartComm;
UserParamTypeDef UserParam;

extern u32 AD_Buff[5][10];                                                             //AD值缓存
extern u32 Weight_Buff[5][10];                                                         //载重值缓存
u8 A_W_Buff[80];

extern u16 g_wDlyMaxTime;                                                      //最大延时时间 0.1S 单位
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
        if (Cur_Param[i] != bak1)//基址值与bak1值不同
            errno1 |= 0x01;

        if (Cur_Param[i] != bak2)//基址值与bak2值不同
            errno1 |= 0x02;

        if (bak1 != bak2)//bak1与bak2值不同
            errno1 |= 0x04;

        if(0x00 == errno1)//都相同，继续
            continue;

        switch (errno1)//errno不可能取01 02 04，矛盾
        {
        case 0x03:////基址值与bak1,bak2值都不同 但bak1,bak2同
            Cur_Param[i] = bak1;
            I2C1_WriteByte(EEPROM_ADDRESS, RUN_ADDR_BASE, Cur_Param[i]);
            break;
        case 0x05://基址值与bak2值同，bak1不同于另两者
            I2C1_WriteByte(EEPROM_ADDRESS, USER_DEFAULT_PARA_BAK1 + i, Cur_Param[i]);
            break;
        case 0x06://基址值与bak1值不同   bak2不同于另两者
            I2C1_WriteByte(EEPROM_ADDRESS, USER_DEFAULT_PARA_BAK2 + i, Cur_Param[i]);
            break;
        case 0x07://三者都不同。都写入默认值
            Cur_Param[i] = User_Default_Param[i];
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, RUN_ADDR_BASE + i, 1, &Cur_Param[i]);
            break;
        }
    }    
}


/*******************************************************************************
名称         : void Check_Software_Ver(void)
创建日期     : 2017-02-21
作者         : 梅梦醒
功能         : 检查软件最新版本号和EEPROM中写入的版本号是否一致，不一致写入最新的版本号
输入参数     : 无
输出参数     : 无
返回结果     : 无
注意和说明   : 
修改内容     : 
********************************************************************************/
void Check_Software_Ver(void)
{
    u8 OldVerLen = 0, NewVerLen = 0, SVerBuff[30], i;
    NewVerLen = sizeof(SoftwareVer) - 1;                                       //最新的版本数组的长度(sizeof得到的长度包括'\0',要去掉)
    OldVerLen = I2C1_ReadByte(EEPROM_ADDRESS, SFVERSION);                       //从EEPROM中读取的版本数组的长度
    if((0 == OldVerLen) || (0x14 < OldVerLen) || (OldVerLen != NewVerLen))      //如果以前没写过或字节数超过30个或与最新版本长度不相等，就写入版本信息
    {
        ProductPara.bFlashWEn = FLASH_WR_ENABLE;                                
        I2C1_WriteByte(EEPROM_ADDRESS, SFVERSION, NewVerLen);                   
        I2C1_WriteNBytes(EEPROM_ADDRESS, SFVERSION + 1, NewVerLen, SoftwareVer);
    }
    else
    {
        for (i = 0; i < OldVerLen; i ++)                                           //读取EEPROM中存放的软件版本
        {                             
            SVerBuff[i] = I2C1_ReadByte(EEPROM_ADDRESS, SFVERSION + 1 + i);         //SFVERSION地址存放的是：版本长度（1字节）+版本号
        }
        if(memcmp(SVerBuff, SoftwareVer, NewVerLen))
        {
            ProductPara.bFlashWEn = FLASH_WR_ENABLE;                                //必须使能写EEPROM
            I2C1_WriteByte(EEPROM_ADDRESS, SFVERSION, NewVerLen);                   //把版本号的长度写入EEPROM
            I2C1_WriteNBytes(EEPROM_ADDRESS, SFVERSION + 1, NewVerLen, SoftwareVer);//把版本号写入EEPROM
        }
    }
}


void Rd_Dev_Param(void)//读取设备参数  上电初始化。
{
    u8 cyBuf[4];
    u32 i, j;
    u16 wTemp;

    I2C1_ReadNBytes(EEPROM_ADDRESS,  USER_DEFAULT_PARA_BAK1 - 5, 4, cyBuf);  //读取4个检测字节。
    
    if((User_Default_Param[0] != I2C1_ReadByte(EEPROM_ADDRESS, RUN_ADDR_BASE)) ||	//若EEPROM无数据，写入出厂值
       (0x11 != cyBuf[0]) || (0x34 != cyBuf[1]) || (0x56 != cyBuf[2]) || (0x7B != cyBuf[3]) )
    {
    	ProductPara.bFlashWEn = FLASH_WR_ENABLE;
        I2C1_WNBytesMul3T(EEPROM_ADDRESS, RUN_ADDR_BASE, PRO_DEFAULT_LEN, User_Default_Param);//将默认参数写入基地址，和两个备份地址
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
    
    I2C1_ReadNBytes(EEPROM_ADDRESS, RUN_ADDR_BASE, PRO_DEFAULT_LEN, Cur_Param);//将基地址中数据读取到Cur_Param中。

    Check_User_Backup();//检查基址和备份地址中数据是否一致。
    Check_Software_Ver();//检查软件版本。
    
    UartComm.SlaveAddr = I2C1_ReadByte(EEPROM_ADDRESS, SLAVE_ADDR);
	MB_DATA[48] = UartComm.SlaveAddr;
	UartComm.BaudRate = I2C1_ReadByte(EEPROM_ADDRESS, BAUDRATE);
	MB_DATA[49] = UartComm.BaudRate;
    UartComm.Parity = I2C1_ReadByte(EEPROM_ADDRESS, PARITY);
	MB_DATA[50] = I2C1_ReadByte(EEPROM_ADDRESS, PARITY1);
	
    MB_DATA[52] = I2C1_ReadByte(EEPROM_ADDRESS, OFFSET_ENABLE);	//不使能补偿
	
	

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
    
    I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_PACKS, 1, &((u8*)&RunVar.TotalPacks)[1]);         //获得标定数组的总包数
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
        for(i = 0; i < RunVar.TotalPacks; i++)                                  //获得标定数组的值
        {
            I2C1_ReadNBytes(EEPROM_ADDRESS, AD_WEIGHT_ADDR(i), 80, A_W_Buff);
            for(j = 0; j < 10; j++)
            {
                AD_Buff[i][j] = HexToUlong(A_W_Buff + 8 * j);                   //AD值
                Weight_Buff[i][j] = HexToUlong(A_W_Buff + 8 * j + 4); //重量值
            }
        }
        
        for(i = RunVar.TotalPacks; i < 5; i++)   //20190705 未标定的数组默认为0xFFFFFFFF 防止初始化为0
        {
           for(j = 0; j < 10; j++)
            {
                AD_Buff[i][j] = 0xFFFFFFFF;                   //AD值
                Weight_Buff[i][j] = 0xFFFFFFFF;                 //重量值
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
	MB_DATA[63] = I2C1_ReadByte(EEPROM_ADDRESS, VEHICLE_WEIGHT);	//整车重量/载荷重量
	MB_DATA[64] = I2C1_Read2Bytes(EEPROM_ADDRESS,OVERLOAD_LIMIT);	//超载阀值

  	MB_DATA[65] = I2C1_ReadByte(EEPROM_ADDRESS, OVERLOAD_LIMIT_DEVIATION);	//超载阀值偏差
	MB_DATA[66] = I2C1_ReadByte(EEPROM_ADDRESS, LOAD_MEASURE_SCHEME);	//载重测量方案
	MB_DATA[67] = I2C1_ReadByte(EEPROM_ADDRESS, LOAD_LIMIT);	//重载阀值
	MB_DATA[68] = I2C1_ReadByte(EEPROM_ADDRESS, LOAD_LIMIT_DEVIATION);	//重载阀值偏差
	MB_DATA[69] = I2C1_ReadByte(EEPROM_ADDRESS, EMPTYLOAD_LIMIT);	//空载阀值
	MB_DATA[70] = I2C1_ReadByte(EEPROM_ADDRESS, EMPTYLOAD_LIMIT_DEVIATION);	//空载阀值偏差
	uint32_t USART_BaudRate = 0,USART_Parity = 0,bits = 8;
	switch (UartComm.BaudRate)
    {
    case 1:
        USART_BaudRate = 2400;
        break;
        
    case 2:
        USART_BaudRate = 4800;
        break;
		
    case 3:                                           //默认为9600
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
        USART_Parity = MODBUS_PARITY_ODD;//奇校验位
		bits = 9;
        break;
    case 2:
        USART_Parity = MODBUS_PARITY_EVEN;//偶校验位
		bits = 9;
        break;
    case 3:
        USART_Parity = MODBUS_PARITY_NONE;//无奇偶校验位
		bits = 8;
        break;        
                                 
    default:
		USART_Parity = MODBUS_PARITY_NONE;//无奇偶校验位
		bits = 8;
        break;
    }
	MB_CfgCh(UartComm.SlaveAddr,MODBUS_SLAVE,0,MODBUS_MODE_ASCII,1,USART_BaudRate,bits,USART_Parity,1,MODBUS_WR_EN);

}



