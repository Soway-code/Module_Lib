/*******************************************************************************
** Name:          UserApp.c
** Auth:          mabuxing
** Vir.:          V1.0.0
** Func:          
** Date:          2020-06-19
** Copyright (C)  SOWAY, 2020-2030
*******************************************************************************/

/*******************************************************************************
** corrector:
** Correction record:
** Correction Date:
** Copyright (C)  SOWAY, 2020-2030
*******************************************************************************/

#include  "UserApp.h"
#include "Usart.h"
#include  "includes.h"
#include "Dmaconfig.h"
#include "ADC.h"
#include "calculate.h"
#include "mpu6050.h"
#include "math.h"
//static uint8_t DMA_Queue[DMA_QUEUE_SIZE] = {0};
//static uint8_t DMA_SendBuff[SEND_BUFF_SIZE] = {0};
//static uint8_t Usart_RecvBuff[RECV_BUFF_SIZE] = {0};

#define GD32F103_FEEDWATCHDOG()		fwdgt_counter_reload()
#define UPLOADDATA_LEN 512
//Receive data cycle buffer
#define MODBUS_RCVBUFSIZE		400
static uint8_t mdbus_rcvbuf[MODBUS_RCVBUFSIZE];
//Send data parameter
#define MODBUS_SNDBUFSIZE		400
static uint8_t mdbus_sndbuf[MODBUS_SNDBUFSIZE];
//Parse data parameter
#define MODBUS_PARSEBUFSIZE		400
static uint8_t mdbus_parsebuf[MODBUS_PARSEBUFSIZE];
//Temparary data buffer
#define TMPBUF_SIZE 200
#define DWTMPBUF_SIZE   (TMPBUF_SIZE >> 2)


#define MODBUS_RTU_DLY      3
#define MODBUS_ASCII_DLY    20

/* Private function prototypes -----------------------------------------------*/
static void UserApp_ParaInit( void );
static bool UserApp_ReadLiquidLevelProcess( void* pdat );
static bool UserApp_ReadAccValueProcess( void* pdat );
bool UserApp_ModBusProcess( void* pdat );
//static void UserApp_USART_Callback( void );

float X_G,Y_G,Z_G; 		
double X_Angle,Y_Angle,Z_Angle;

/* �������Ƕ���������ֵ������оƬ�̶��� */
int32_t X_ACCEL_OFFSET=30;
int32_t Y_ACCEL_OFFSET=160;
int32_t Z_ACCEL_OFFSET=0;

/* ���������ٶ���������ֵ */
int32_t X_G_OFFSET=32840;
int32_t Y_G_OFFSET=32840;
int32_t Z_G_OFFSET=32760;

/* Private user code ---------------------------------------------------------*/
/*5
static S_USART_ParaTypeDef USART1_RunPara = 
{
	DMA_Queue,
	DMA_QUEUE_SIZE,
	BAUD_9600,			//baudrate
	PARITY_NONE,		//parity
	UserApp_USART_Callback,
};
*/
//Little-endian format
const uint8_t CalibrateInitPara[24] = {
	0xCF,0xDE,0xEE,0xCC,0xFC,0xE1,0x30,0x3F,	//double//press calibrate K	
	0x24,0x68,0x78,0x3E,0x2E,0x8B,0x32,0xC0,	//double//press calibrate B 
	0x00,0x59,0x8E,0x46,						//float //temperature calibrate K
	0,0,0,0
};


static S_MODBUS_RUNPARA	ModBus_RunPara = 
{
	0x01,	//slaver address
	BAUD_9600,				//uint8_t //baud rate
	PARITY_NONE,			//parity
	MODBUS_ASCII,
	mdbus_sndbuf,
	MODBUS_SNDBUFSIZE,
	mdbus_parsebuf,
	MODBUS_RCVBUFSIZE,
	My_USART_DMA_SendData,
	My_USART_GetOneFrameDMAData,
};

uint8_t const User_Default_Param[PRO_DEFAULT_LEN] =
{
	0x5A,
	FLASH_INIT_FLAG,							//������ʼ����־λ
    
	0x71,								        //�豸��ַ
	0x03,								        //������(9600)
	0x03,								        //��żУ��(��У��)
	0x02,								        //�Ƿ񶳽��豸      
	0x03,								        //�˲�ϵ��
	0x01,								        //�Զ���������
	0x3F, 0x80, 0x00, 0x00,				        //����ϵ��K
	0x3F, 0x80, 0x00, 0x00,				        //����ϵ��B
	0x01,								        //����ʹ��
	

    0x06, 0x40,                                                  //�����ֵ 2 bytes  1600
    0x00,                              	                          //��¼���� 1 byte
    0x00,                              	                         //����״̬ 1 byte  LOAD_EMPTY_STATE
    0x02, 0xBC,               			                          //����ϵ�� 4 bytes  0.7 -> 700 �Ŵ�100��
    0x04, 0xB0,             			                           //����ֵ 4 bytes  1.2  -> 1200 �Ŵ�100��
    0x00, 0x00,                                                     //��С�߶�ֵ 2 bytes  0 
    0x00, 0x64,                                                    //��СADֵ 2 bytes    100
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
};

static void GD32F103_FDWDGInit( void )
{
	rcu_osci_on(RCU_IRC40K);
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K));
	
	/*	FWDG clock is independent lsi 40KHz, DIV128 IS 312.5Hz, that is 3.2 ms per clock
	reload value is 1000, delay time 2500 * 3.2ms = 8 s	*/
	fwdgt_config( 2500, FWDGT_PSC_DIV256 );
	fwdgt_enable();

}
//static void Boot_ParseReceiveModbusData( uint8_t* psrc, uint32_t rcvlen, uint8_t msgtype )
//{
//		uint16_t i = 0;
//	i++;
//	
//}
/*******************************************************************************
Name:    
Func: 
Para:   
Retn:
Desc: 
Date: 2020-06-19
*******************************************************************************/
void UserApp_Process( void )
{
//
	uint16_t ad[22],weight[20];						//ad��ֵ������
	
	static S_COMM_PARA commpara = { 0x02, 0x03, 0x03, 0x02 };	//
	uint8_t count = 0,i=0,j=0;
	S_PARSE_TAG s_parsepara;
	S_COMM_BUFPARA usart_dma_para;
	rcu_config();
	timer_config();
	dma_config();
	UserApp_ParaInit( );
	count = Flash_Read_OneByte(NOW_ADDR+UPGRADE_MAEKS);		//�����Ѿ��궨����
	for(;i<count;i++)
	{
		ad[j] = Flash_Read_fourByte(AD1_ADDR+UPGRADE_MAEKS+(i*4));
		weight[j++] = Flash_Read_fourByte(LOAD1_ADDR+UPGRADE_MAEKS+(i*4));
	}
	WeightProcessing_init(ad,weight,count);
	
	//DMA & Usart parameters init
	usart_dma_para.prcv =  mdbus_rcvbuf;
	usart_dma_para.rcvsize = MODBUS_RCVBUFSIZE;
	usart_dma_para.psnd = mdbus_sndbuf;
	usart_dma_para.sndsize = MODBUS_SNDBUFSIZE;
	
	//Parse parameters init
	s_parsepara.pbuf = mdbus_parsebuf;
	s_parsepara.bufsize = MODBUS_PARSEBUFSIZE;
	//s_parsepara.pparse = Boot_ParseReceiveModbusData;
	
	//Usart and DMA config
	commpara.BaudRate= Cur_Param[BAUDRATE];
	commpara.SlaveAddr = ModBus_RunPara.SlaveAddr;
	commpara.Parity = Cur_Param[PARITY];
	USART_CommunicateInitial( 0, &usart_dma_para, &commpara );		//0-USART0 1-USART1 2-USART2 ...
	DMA_Rcv_TransDataParaConfig( &usart_dma_para );
	USART_ConfigPaserParameters( &s_parsepara );

	//MPU6050 Init 
	MPU_Init();
	
	//USART Init 
	F3_CommunicateInit( &ModBus_RunPara );
	GD32F103_FDWDGInit();
	GD32F103_TimerSet( eTmr_ReadPressTimer, UserApp_ReadLiquidLevelProcess, NULL, 100 );
	GD32F103_TimerSet( eTmr_ReadAccTimer, UserApp_ReadAccValueProcess, NULL, 1000 );
	while(1)
	{
		GD32F103_Systick_TimerProcess( );
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
//static bool PowerOnFlag = true;
static uint16_t  PowerOnCnt = 0;
uint8_t PowerOnCnt1 =0;
static bool UserApp_ReadLiquidLevelProcess( void* pdat )
{
	PowerOnCnt++;
	PowerOnCnt1++;
	if(PowerOnCnt1 >= (Flash_Read_fourByte(FIlTERING_TIME+UPGRADE_MAEKS)+2))
	{
		PowerOnCnt1 = 0;
		ADC_Filter();
	}
	GD32F103_FEEDWATCHDOG();
	//if( PowerOnCnt > 600 && PowerOnFlag )	//ÿ��10��
	{

		ModBus_RunPara.SlaveAddr = Cur_Param[SLAVE_ADDR];
		F3_CommunicateInit( &ModBus_RunPara );

	}
	
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
static bool UserApp_ReadAccValueProcess( void* pdat )
{
	int16_t aacx,aacy,aacz;		//���ٶȴ�����ԭʼ����
	int16_t gyrox,gyroy,gyroz;	//������ԭʼ���� 
//	MMA8652_ReadData( );
					//�õ����ٶȴ���������
	aacx = MPU6050_Get_Data(MPU_ACCEL_XOUTH_REG);
	aacy = MPU6050_Get_Data(MPU_ACCEL_YOUTH_REG);
	aacz = MPU6050_Get_Data(MPU_ACCEL_ZOUTH_REG);
	
	//�õ�����������
	gyrox = MPU6050_Get_Data(MPU_GYRO_XOUTH_REG);
	gyroy = MPU6050_Get_Data(MPU_GYRO_YOUTH_REG);
	gyroz = MPU6050_Get_Data(MPU_GYRO_ZOUTH_REG);
	
	X_G = ((float)gyrox + (float)X_G_OFFSET)/2048.0;
	Y_G = ((float)gyroy + (float)Y_G_OFFSET)/2048.0;
	Z_G = ((float)gyroz + (float)Z_G_OFFSET)/2048.0;

	X_Angle = (atan((aacx+X_ACCEL_OFFSET) / 16384.0)* 57.29577*2) + 90;
	Y_Angle = (atan((aacy+Y_ACCEL_OFFSET) / 16384.0)* 57.29577*2) + 90;
	Z_Angle = (atan((aacz+Z_ACCEL_OFFSET) / 16384.0)* 57.29577*2); 
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
bool UserApp_ModBusProcess( void* pdat )
{
	MBASCII_Function( );
	return false;
}

/*******************************************************************************
Name:
Func:
Para:
Retn:
Desc:
Date: 2020-07-13
*******************************************************************************/
bool UserApp_AutoLoadProcess( void* pdat )
{
	MBASCII_AutoUpLoadFrame( );
	return true;
}


/*******************************************************************************
Name:    
Func: 
Para:   
Retn:
Desc: 
Date: 2020-06-19
*******************************************************************************/
extern const uint32_t UserBuadrate[USER_BAUDRATE_NUM];
static void UserApp_ParaInit( void )
{
	uint8_t di;
    uint8_t ptmp[8];
	
	if(Flash_Read_fourByte(FIlTERING_TIME+UPGRADE_MAEKS) > 250)
		Flash_Write_OneByte(FIlTERING_TIME+UPGRADE_MAEKS,0x23);
	
	if( *(uint8_t*)(PARA_OFFSET_ADDRESS+1) != FLASH_INIT_FLAG )
	{		
		Flash_Write_MultiBytes( PARA_OFFSET_ADDRESS, (uint8_t*)User_Default_Param, PRO_DEFAULT_LEN );
		Flash_Write_OneByte(NOW_ADDR+UPGRADE_MAEKS,0x00);		//�Ѿ��궨����
		Flash_Write_OneByte(SIGN_ADDR+UPGRADE_MAEKS,0x00);		//�궨������
		
		Flash_Write_OneByte(FIlTERING_TIME+UPGRADE_MAEKS,0x23);
//		Flash_Write_MultiBytes( (PARA_OFFSET_ADDRESS + P_CALIBRATE_K_ADD),CalibrateInitPara, 24 );//ע��д���ַҪ����8�ֽڶ���
	}
	
    //��ȡ�û�����
	Flash_Read_MultiBytes( PARA_OFFSET_ADDRESS, Cur_Param,CUR_PARA_LEN );//ע����u64�ĳ��ȣ��������
    
//5	JHM1203_Para.P_Calibrate_K = 1;
//5	JHM1203_Para.P_Calibrate_B = 0;
	
//5	memcpy( &JHM1203_Para.T_Calibrate_K, &Cur_Param[T_CALIBRATE_K_ADD], 8 );
	
    ProductPara.CalibTemp = (ptmp[0] << 8)|ptmp[1];
    //��ȡ��������
//	UserApp_ReadParaArray( CALIBOILRAG, ptmp, 4 );
    ProductPara.OilRange = 0;
    for(di = 0; di < 4; di++)
    {
		ProductPara.OilRange = (ProductPara.OilRange << 8) + ptmp[di];
    }
    
    //�����ʷ�Χ
    if((0x01 > Cur_Param[BAUDRATE])||(0x07 < Cur_Param[BAUDRATE]))
    {
		Cur_Param[BAUDRATE] = 0x03;
    }
    //��żУ�鷶Χ
    if((0x01 > Cur_Param[PARITY])||(0x03 < Cur_Param[PARITY]))
    {
      Cur_Param[PARITY] = 0x03;
    }
    //�ӻ���ַ
	ModBus_RunPara.SlaveAddr = 255;
    //������
    ModBus_RunPara.baudrate = Cur_Param[BAUDRATE];
	//USART1_RunPara.baudrate = ModBus_RunPara.baudrate;
    //��żУ��
    ModBus_RunPara.parity = Cur_Param[PARITY];
//	USART1_RunPara.parity = ModBus_RunPara.parity;
    //�˲��ȼ�
    UserParam.FilterLevel = Cur_Param[FILTER_LEVEL];
    //�˲��л�
    Switch_Fiter(UserParam.FilterLevel);
    //�Զ��ϴ�����
    ProductPara.AutoUpLoadTime = Cur_Param[AUTO_UPLOAD_TIME];
	if( ProductPara.AutoUpLoadTime > 1)
	GD32F103_TimerSet( eTmr_AutoLoadTimer, UserApp_AutoLoadProcess, NULL, (ProductPara.AutoUpLoadTime * 1000) );
    //����ϵ��K
    UserParam.UserFlowK = HexToFloat(&Cur_Param[USER_FLOW_K]);
    //����ϵ��B
    UserParam.UserFlowB = HexToFloat(&Cur_Param[USER_FLOW_B]);
	
    //����ʹ��
    ProductPara.CompenEn = Cur_Param[COMPENSATE];

//    //�������
//    //ProductPara.CapMin = ArrToHex(&Cur_Param[CAPMIN]);
//	memcpy( &ProductPara.CapMin, &Cur_Param[CAPMIN], 4 );
//    //��������
//    //ProductPara.CapMax = ArrToHex(&Cur_Param[CAPMAX]);
//	memcpy( &ProductPara.CapMax, &Cur_Param[CAPMAX], 4 );
	//��������
//	ProductPara.CapMax = 11300;
//	ProductPara.CapMin = 1300;
    ProductPara.CapRange = ProductPara.CapMax - ProductPara.CapMin;
    //��������ֵ
//	memcpy( &ProductPara.AddLiqCnt, &Cur_Param[ADDLIQLMT], 2 );
//    //���μ�����ֵ
//    ProductPara.RestLiqCnt = ProductPara.AddLiqCnt / 2;
//    //����ʱ����ֵ
//	memcpy( &ProductPara.AddLiqTime, &Cur_Param[ADDLIQTIME], 2 );
//    //©������ֵ
//    //ProductPara.SubLiqCnt = (Cur_Param[SUBLIQLMT] << 8)|Cur_Param[SUBLIQLMT +1];
//	memcpy( &ProductPara.SubLiqCnt, &Cur_Param[SUBLIQLMT], 2 );
//    //©��ʱ����ֵ
//	memcpy( &ProductPara.SubLiqTime, &Cur_Param[SUBLIQTIME], 2 );
//    //ÿ15s���ϱ仯����ֵ
//    memcpy( &ProductPara.UpThreshold, &Cur_Param[UP_THRETHOLD], 2 );
//	//ÿ15s���±仯����ֵ
//    memcpy( &ProductPara.DownThreshold, &Cur_Param[DOWN_THRETHOLD], 2 );
    //���ƫ����
//    memcpy( &ProductPara.OrignOffset, &Cur_Param[ORIGN_OFFSET], 2 );
//    //������������
//    ProductPara.Def_Range = HexToFloat(&Cur_Param[DEF_RANGE]);
//    //�����������
//    ProductPara.Def_CapMin = HexToFloat(&Cur_Param[DEF_CAPMIN]);
//    //������������
//    ProductPara.Def_CapMax = HexToFloat(&Cur_Param[DEF_CAPMAX]);

    //�¶�1ϵ��K
    //ProductPara.Temp_K = HexToFloat(&Cur_Param[TEMP_K]);
//	memcpy( &ProductPara.Temp_K, &Cur_Param[T_CALIBRATE_K_ADD], 4 );
//    //�¶�1ϵ��B
//    ProductPara.Temp_B = HexToFloat(&Cur_Param[TEMP_B]);
//    //�¶�2ϵ��K
//    ProductPara.Temp2_K = HexToFloat(&Cur_Param[TEMP2_K]);
//    //�¶�2ϵ��B
//    ProductPara.Temp2_B = HexToFloat(&Cur_Param[TEMP2_B]);
//    //Һλ�Ƚ�ʱ����   
//	ProductPara.LiquidCmpTc = Cur_Param[LIQUIDCMPTC];
//    if((0x02 > ProductPara.LiquidCmpTc || (0x05 < ProductPara.LiquidCmpTc)))
//    {
//		ProductPara.LiquidCmpTc = 0x03;
//		UserApp_ParaArrayUpdata( LIQUIDCMPTC, &ProductPara.LiquidCmpTc, 1, false );
//    }

//    ProductPara.FuelType = Cur_Param[FUELTYPE];                                 //ȼ������
//    ProductPara.BoxShape = Cur_Param[BOXSHAPE];                                 //������״
//	memcpy( &ProductPara.BoxPara[0], &Cur_Param[BOXPARA0], 8 );					//���䳤�����ߡ��񡢲���
//    
	
//    //Һλ���DA  ����    
//    ProductPara.LiquitHeightforDAMax = HexToFloat(&Cur_Param[HEIGHTDAMAX]);
//	memcpy( &ProductPara.SensorLen, &Cur_Param[RANGE], 2 );
//    //ProductPara.SensorLen = (Cur_Param[RANGE] << 8) + Cur_Param[RANGE + 1];     //����������
//    ProductPara.Range = ProductPara.SensorLen;                            //����������=����-ä��������ä���͹̶�33mm��


//    //�����궨����
//	UserApp_ReadParaArray(CALIB_HEIGH_RELATIONSHIP,CALIB_Table,CALIN_HEIGH_TABLE_NUM);



    //������ʼ����־
    RunVar.OilInit = 1;
}




uint8_t Cur_Param[CUR_PARA_LEN] = {0};                                            	//ϵͳ��ǰ����
uint8_t CALIB_Table[CALIN_HEIGH_TABLE_NUM];                                          //�����궨����

ProductParaTypeDef ProductPara;                                                 //��Ʒ����
RunVarTypeDef RunVar;                                                           //���б���
UserParamTypeDef UserParam;                                                     //�û�����



void Switch_Fiter(uint8_t value)
{
    switch( value )
    {	
    case 1:
        UserParam.FilterBufMax = 1;    
        UserParam.FilterCycle = 1;      //ʵʱ�˲�1s
        break;
    case 2:
        UserParam.FilterBufMax = 18;    
        UserParam.FilterCycle = 180;    //ƽ���˲�180s
        break;
    case 3:
        UserParam.FilterBufMax = 96;    //ƽ���˲�960s
        UserParam.FilterCycle = 960;
        break;
    default:
        UserParam.FilterBufMax = 18;    
        UserParam.FilterCycle = 180;
        break;
    }
}

/*******************************************************************************
Name:
Func:
Para:
Retn:
Desc:
Date: 2020-07-14
*******************************************************************************/
void UserApp_ReadParaArray( uint32_t Address,  uint8_t *pData, uint32_t Datalen )
{
	memcpy( pData, &Cur_Param[Address], Datalen);
}

/*******************************************************************************
Name:
Func:
Para:
Retn:
Desc:
Date: 2020-07-14
*******************************************************************************/
bool UserApp_ParaArrayUpdata( uint32_t Address,  uint8_t *pData, uint32_t Datalen, bool Initflag )
{
	memcpy( &Cur_Param[Address], pData, Datalen );
 
	if(!Flash_Read_OneByte(PARA_OFFSET_ADDRESS))
		return false;
	if(!Flash_Write_MultiBytes(PARA_OFFSET_ADDRESS, Cur_Param, CUR_PARA_LEN))
		return false;
	if( Initflag )
	{
		UserApp_ParaInit( );
//5		My_USART_DMA_Init( &USART1_RunPara );
	}
	return true;
}


/************************ (C) COPYRIGHT SOWAY ******* END OF FILE *************/
