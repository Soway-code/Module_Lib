#ifndef _MODBUS_H
#define _MODBUS_H
#include  "includes.h"

#define LITTLEENDIAN    0
#define BIGENDDIAN      1

#define ASCII_FRAME_HEADER  0x3A
#define ASCII_FRAME_ENTER   0x0D
#define ASCII_FRAME_NEWLINE 0x0A
#define ASCII_FRAME_ENDFLAG 0x0D0A


//------------------------define modbus parse function type----------------------------
typedef void (*ModbusParseFunc)(uint8_t *psrc, uint32_t len, uint8_t type );
typedef bool (*ModbusSendDataFunc)(uint16_t DataLen);
typedef uint16_t  (*ModbusRecvDataFunc)( uint8_t* pRecvBuff, uint16_t RvBuffSize );

//--------------define send data structure------------------------------
typedef struct{
    uint8_t* psndbuf;
    uint16_t bufsize;
	uint16_t sendlen;
	ModbusSendDataFunc senddata;
}S_SNDDAT_TAG,*PS_SNDDAT_TAG;

//--------------define parse structure----------------------------------
//typedef struct{
//    uint8_t*             pbuf;
//    uint32_t             bufsize;
//	ModbusRecvDataFunc recvdata;
//    //ModbusParseFunc pparse;
//}S_PARSE_TAG, *PS_PARSE_TAG;

typedef enum{
    MODBUS_RTU = 1,
    MODBUS_ASCII
}MODBUS_FRAMETYPE;

typedef struct{
	uint8_t SlaveAddr;
	uint8_t baudrate;
	uint8_t parity;
    MODBUS_FRAMETYPE ModbusType;  //1-RTU 2-ASCII
    uint8_t* psendbuff;
    uint16_t sendsize;
    uint8_t* pparsebuff;
    uint16_t parsesize;
    ModbusSendDataFunc senddata;
	ModbusRecvDataFunc recvdata;
    //ModbusParseFunc pparse;
}S_MODBUS_RUNPARA, *PS_MODBUS_RUNPARA;

//modbus���ܺ�
#define MDB_READ_HOLD_REG       		        0x03
#define MDB_READ_INPUT_REG         		        0x04
#define MDB_WRITE_SINGLE_REG     		        0x06
#define MDB_WRITE_MULTIPLE_REG     		        0x10

//important define
#define MB_ADDRESS_BROADCAST                            0x00//�㲥��ַ
#define MB_ADDRESS_MIN                                  0x01//��С�ӻ���ַ
#define MB_ADDRESS_MAX                                  247 //���ӻ���ַ
#define MB_FUNC_NONE                                    0
#define MB_FUNC_READ_COILS                              1 
#define MB_FUNC_READ_DISCRETE_INPUTS                    2
#define MB_FUNC_READ_HOLDING_REGISTER                   3
#define MB_FUNC_READ_INPUT_REGISTER                     4
#define MB_FUNC_WRITE_SINGLE_COIL                       5
#define MB_FUNC_WRITE_REGISTER                          6
#define MB_FUNC_DIAG_READ_EXCEPTION                     7
#define MB_FUNC_DIAG_DIAGNOSTIC                         8
#define MB_FUNC_DIAG_GET_COM_EVENT_CNT                  11
#define MB_FUNC_DIAG_GET_COM_EVENT_LOG                  12
#define MB_FUNC_WRITE_MULTIPLE_COILS                    15
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS                16
#define MB_FUNC_OTHER_REPORT_SLAVEID                    17
#define MB_FUNC_READWRITE_MULTIPLE_REGISTERS            23 
#define MB_FUNC_ERROR                                   128

//modbus error���ش�����
#define MDB_NO_ERROR       				0x0//�޴���
#define MDB_FNO_INVALID                                 0x1//��֧�ֵĹ���
#define MDB_REG_ADDR_INVALID                            0x2//�Ĵ�����ַ����ȷ
#define MDB_REG_VALUE_OVERRIDE                          0x3//�Ĵ���ֵ������Χ
#define MDB_OP_FAIL                                     0x4//����ʧ��

#define MB_EX_NONE 					(0x00)//Modbus�޴���
#define MB_EX_ILLEGAL_FUNCTION 		                (0x01)//Modbus�쳣������
#define MB_EX_ILLEGAL_DATA_ADDRESS	                (0x02)//Modbus��Ч���ݵ�ַ
#define MB_EX_ILLEGAL_DATA_VALUE	                (0x03)//Modbus��Ч��ֵ
#define MB_EX_SLAVE_DEVICE_FAILURE	                (0x04)//Modbus�����豸ʧ��
#define MB_EX_ACKNOWLEDGE			        (0x05)//Modbus�쳣Ӧ��
#define MB_EX_SLAVE_BUSY			        (0x06)//Modbus���豸æ
#define MB_EX_MEMORY_PARITY_ERROR	                (0x08)//ModbusУ�����
#define MB_EX_GATEWAY_PATH_FAILED	                (0x0A)
#define MB_EX_GATEWAY_TGT_FAILED	                (0x0B)

#define ASCII_HOLDING_REG_REGION1_BGEIN			0x30//���ּĴ�����1��ʼ��ַ
#define ASCII_HOLDING_REG_REGION1_END			0x54//���ּĴ�����1������ַ
#define ASCII_HOLDING_REG_REGION2_BGEIN			0x60//���ּĴ�����2��ʼ��ַ
#define ASCII_HOLDING_REG_REGION2_END			0x60//���ּĴ�����2������ַ
#define ASCII_HOLDING_REG_REGION3_BGEIN			0x80//���ּĴ�����3��ʼ��ַ
#define ASCII_HOLDING_REG_REGION3_END		        0x80//���ּĴ�����3������ַ
//#define ASCII_HOLDING_REG_EXTERN0_BEGIN                 0x70
//#define ASCII_HOLDING_REG_EXTERN0_END                   0x73

#define ASCII_INPUT_REG_REGION1_BGEIN        		0x00//����Ĵ�����1��ʼ��ַ
#define ASCII_INPUT_REG_REGION1_END          		0x1A//����Ĵ�����1������ַ
#define ASCII_INPUT_REG_REGION2_BGEIN         		0x80//����Ĵ�����2��ʼ��ַ
#define ASCII_INPUT_REG_REGION2_END           		0x84//����Ĵ�����2������ַ

#define ASCII_SINGLE_COIL_ADDR_BGEIN          		0x50//�����Ĵ�����ʼ��ַ
#define ASCII_SINGLE_COIL_ADDR_END            		0x53//�����Ĵ���������ַ

#define ASCII_SPECICAL_COIL_REGION1_BGEIN    		0x00//����Ĵ�����1��ʼ��ַ
#define ASCII_SPECICAL_COIL_REGION1_END      		0x03//����Ĵ�����1������ַ
#define ASCII_SPECICAL_COIL_REGION2_BGEIN    		0x40//����Ĵ�����2��ʼ��ַ
#define ASCII_SPECICAL_COIL_REGION2_END      		0x40//����Ĵ�����2������ַ

#define ASCII_MUL_REG_REGION1_BGEIN                     ASCII_HOLDING_REG_REGION1_BGEIN //����Ĵ�����1��ʼ��ַ
#define ASCII_MUL_REG_REGION1_END            	        ASCII_HOLDING_REG_REGION1_END   //����Ĵ�����1������ַ
#define ASCII_MUL_REG_REGION2_BGEIN                     ASCII_HOLDING_REG_REGION2_BGEIN //����Ĵ�����2��ʼ��ַ
#define ASCII_MUL_REG_REGION2_END            	        ASCII_HOLDING_REG_REGION2_END   //����Ĵ�����2������ַ
#define ASCII_MUL_REG_REGION3_BGEIN                     ASCII_HOLDING_REG_REGION3_BGEIN //����Ĵ�����3��ʼ��ַ
#define ASCII_MUL_REG_REGION3_END            	        ASCII_HOLDING_REG_REGION3_END   //����Ĵ�����3������ַ
#define ASCII_MUL_REG_EXTERN0_BEGIN                     0x70//����Ĵ�����չ��ʼ��ַ
#define ASCII_MUL_REG_EXTERN0_END                       0x73//����Ĵ�����չ������ַ

#define ASCII_DB_HOLD_REG_REGION1_BEGIN                 0x00//˫���Ĵ�����1��ʼ��ַ
#define ASCII_DB_HOLD_REG_REGION1_END                   0x74//˫���Ĵ�����1������ַ
#define ASCII_DB_HOLD_REG_REGION2_BEGIN                 0x7E//˫���Ĵ�����2��ʼ��ַ
#define ASCII_DB_HOLD_REG_REGION2_END                   0x9A//˫���Ĵ�����2������ַ
#define ASCII_DB_HOLD_REG_REGION3_BEGIN                 0xB0//˫���Ĵ�����3��ʼ��ַ
#define ASCII_DB_HOLD_REG_REGION3_END                   0xB6//˫���Ĵ�����3������ַ

#define ASCII_PRODUCTION_INF_BEGIN                      0xE0//��Ʒ�汾��Ϣ����ʼ��ַ
#define ASCII_PRODUCTION_INF_END                        0xE5//��Ʒ�汾��Ϣ�ν�����ַ

#define UART_WAIT_TIME	                                100 //��Ϣ֡��Ӧ��ʱʱ��
#define DATAPACKET_LEN                                  80  //���ݰ���󳤶�

void MBASCII_SendMsg(uint8_t *u8Msg, uint32_t u8MsgLen);
void MBASCII_SendErr(uint8_t ErrCode);
void Read_CALIBAndHeighTable(void);
void MBASCII_Fun03(void);
void MBASCII_Fun04(void);
void MBASCII_Fun05(void);
void Write_CALIBAndHeighTable(void);
void MBASCII_Fun10(void);
void MBASCII_Fun25(void);
void MBASCII_Fun26(void);
void MBASCII_Fun27(void);
void MBASCII_Fun2A(void);
void MBASCII_Fun2B(void);
void MBASCII_Fun41(void);
void MBASCII_Function(void);
bool F3_CommunicateInit( PS_MODBUS_RUNPARA pMdbsPara );
uint32_t Get_TankOil(uint32_t cyLiquidHeightAD);
void MBASCII_AutoUpLoadFrame(void);





#define ASCII_HEAD_STATE  0     //ͷ
#define ASCII_END_STATE   1     //����
#define ASCII_IDLE_STATE  2     //����

#define ASCII_HEAD_DATA   ':'           //��ͷ��һ���ֽ�

void MODBUS_ASCII_HexToAscii(uint8_t cyHexData, uint8_t *pCyAsciiBuf);
uint8_t MODBUS_ASCII_AsciiToHex(uint8_t *pCyAsciiBuf);
uint8_t MODBUS_ASCII_GetLrc(uint8_t *pCyAsciiBuf, uint8_t cyLen);
uint8_t MODBUS_ASCII_AsciiPacketToRtuPacket(uint8_t *pCyAsciiBuf, uint8_t cyAsciiLen, uint8_t *pCyRtuBuf);
uint8_t MODBUS_ASCII_RtuPacketToAsciiPacket(uint8_t *pCyRtuBuf, uint8_t cyRtuLen, uint8_t *pCyAsciiBuf);
void MODBUS_ASCII_HandlRevData(uint8_t cyRevData);
uint8_t MODBUS_ASCII_CheckAscii(uint8_t *pCyAsciiBuf, uint8_t cyLen);
uint8_t MODBUS_ASCII_RecvData(uint8_t* cyRecvBuff, uint8_t *pCyLen);


#endif

