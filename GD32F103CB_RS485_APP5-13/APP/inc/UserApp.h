#ifndef _USERAPP
#define _USERAPP

//Include files
#include  "includes.h"
/*
Version describle:
    1.V_Xyy_000.01 is the first version edited by Xie_Yuyin 2020-11-13
	1)Modified the oil display percent algorithm: Act oil h / tank height
	2)No fileter original data upload
*/
#define	SOFTWARE_VER	V_Xyy_000.01

//USART_DMA.H�ĺ궨��
#define USER_BAUDRATE_NUM	8

#define  IDLE_STATE             false
#define  RECV_OK                true
#define  SENDING                true

#define  BAUD_1200              0
#define  BAUD_2400              1
#define  BAUD_4800              2
#define  BAUD_9600              3
#define  BAUD_19200             4
#define  BAUD_38400             5
#define  BAUD_57600             6
#define  BAUD_115200            7

#define  PARITY_NONE            3
#define  PARITY_ODD            	1
#define  PARITY_EVEN            2

//
#define CUR_PARA_LEN			1024

#define UPGRADE_MAEKS			0x8003000
#define	FLASH_INIT_FLAG			0x55		//0x55

#define	PARA_OFFSET_ADDRESS		(UPGRADE_MAEKS + 0x01)//0x0801F800//0x08005800									
#define RUN_ADDR_BASE			0x01					//������ʼ����־λ������ÿ���ϵ綼дEEPROMP
#define SLAVE_ADDR		       (RUN_ADDR_BASE	    + 0x01)
#define BAUDRATE		       (SLAVE_ADDR	    	+ 0x01)
#define PARITY			       (BAUDRATE	    	+ 0x01)
#define FORMART			       (PARITY	    		+ 0x01)
#define FILTER_LEVEL		   (FORMART	            + 0x01)
#define AUTO_UPLOAD_TIME	   (FILTER_LEVEL	    + 0x01)
#define USER_FLOW_K		       (AUTO_UPLOAD_TIME    + 0x01)
#define USER_FLOW_B		       (USER_FLOW_K	    	+ 0x04)
#define COMPENSATE		       (USER_FLOW_B	    	+ 0x04)
#define USER_DEFAULT_LEN	   (COMPENSATE          + 0x01)		//16byte

//---------------------------------------------------
#define LOAD_RATED_VALUE 		   (COMPENSATE                    + 0x04)   //�����ֵ 2 bytes
#define LOAD_SHOVEL_NUM			   (LOAD_RATED_VALUE               + 0x02)   //��¼���� 1 byte
#define LOAD_WORK_STATE             (LOAD_SHOVEL_NUM                + 0x01)   //����״̬ 1 byte
#define LOAD_OVER_K					(LOAD_WORK_STATE                + 0x01)   //����ϵ�� 2 bytes
#define LOAD_VARIANCE_VALUE          (LOAD_OVER_K                    + 0x02)   //����ֵ 2 bytes
#define TEST_MIN_HIGH			        (LOAD_VARIANCE_VALUE            + 0x02)   //��С�߶�ֵ 2 bytes
#define TEST_MIN_ADC			        (TEST_MIN_HIGH                  + 0x02)   //��СADֵ 2 bytes
#define TEST_MID_HIGH			        (TEST_MIN_ADC                   + 0x02)   //�м�߶�ֵ 2 bytes
#define TEST_MID_ADC			        (TEST_MID_HIGH                  + 0x02)   //�м�ADֵ 2 bytes
#define TEST_MAX_HIGH			        (TEST_MID_ADC                   + 0x02)   //���߶�ֵ 2 bytes
#define TEST_MAX_ADC			        (TEST_MAX_HIGH                  + 0x02)   //���ADֵ 2 bytes
#define MAX_DLY_TIME                     (TEST_MAX_ADC                   + 0x02)   //װ��������ӳ�ʱ�� 2 bytes ��λΪ0.1S
         
#define VEHICLE_WEIGHT                   (MAX_DLY_TIME                   + 0x02)   //��������/�غ�����
#define LOAD_MEASURE_SCHEME                (VEHICLE_WEIGHT                 + 0x02)   //���ز�������
#define OVERLOAD_LIMIT                    (LOAD_MEASURE_SCHEME            + 0x02)   //���ط�ֵ
#define OVERLOAD_LIMIT_DEVIATION          (OVERLOAD_LIMIT                 + 0x02)   //���ط�ֵƫ��
#define LOAD_LIMIT                         (OVERLOAD_LIMIT_DEVIATION       + 0x02)   //���ط�ֵ 
#define LOAD_LIMIT_DEVIATION                (LOAD_LIMIT                     + 0x02)   //���ط�ֵƫ��
#define EMPTYLOAD_LIMIT                     (LOAD_LIMIT_DEVIATION           + 0x02)   //���ط�ֵ
#define EMPTYLOAD_LIMIT_DEVIATION           (EMPTYLOAD_LIMIT                + 0x02)   //���ط�ֵƫ��
#define LIGHTLOAD_LIMIT                      (EMPTYLOAD_LIMIT_DEVIATION      + 0x02)   //���ط�ֵ
#define LIGHTLOAD_LIMIT_DEVIATION            (LIGHTLOAD_LIMIT                + 0x02)   //���ط�ֵƫ�� 
#define WEIGHT_UNIT                          (LIGHTLOAD_LIMIT_DEVIATION      + 0x02)
#define FIlTERING_TIME						 (WEIGHT_UNIT      + 0x06)
#define PRO_DEFAULT_LEN                      (FIlTERING_TIME                    + 0x04)   //Ĭ�ϲ������� 91���ֽ�     

#define SIGN_ADDR						(WEIGHT_UNIT+0x3E3)		//��־�ӵڼ���д��
#define NOW_ADDR						(SIGN_ADDR+0x01)			//Ŀǰ�Ѿ��궨�е�����
#define AD1_ADDR						(NOW_ADDR+0x01)				//10������ֵAD�궨;	21��
#define LOAD1_ADDR						(AD1_ADDR+0x54)				//10������ֵ�����궨;	21��

#define CALIN_HEIGH_TABLE_NUM	       400

//#define DMA_QUEUE_SIZE		2048
//#define SEND_BUFF_SIZE 		600
//#define RECV_BUFF_SIZE 		600
//#define FLASH_PARA_SIZE 	32


extern uint8_t CALIB_Table[CALIN_HEIGH_TABLE_NUM];
extern uint8_t Cur_Param[CUR_PARA_LEN];

extern ProductParaTypeDef ProductPara;
extern RunVarTypeDef RunVar;
extern UserParamTypeDef UserParam;

void Switch_Fiter(uint8_t value);

bool UserApp_ModBusProcess( void* pdat );
void UserApp_Process( void );
bool UserApp_AutoLoadProcess( void* pdat );
void UserApp_ReadParaArray( uint32_t Address,  uint8_t *pData, uint32_t Datalen );
bool UserApp_ParaArrayUpdata( uint32_t Address,  uint8_t *pData, uint32_t Datalen, bool Initflag );

#endif

/************************ (C) COPYRIGHT SOWAY ******* END OF FILE *************/
