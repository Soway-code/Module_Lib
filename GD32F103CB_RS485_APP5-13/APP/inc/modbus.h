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

//modbus功能号
#define MDB_READ_HOLD_REG       		        0x03
#define MDB_READ_INPUT_REG         		        0x04
#define MDB_WRITE_SINGLE_REG     		        0x06
#define MDB_WRITE_MULTIPLE_REG     		        0x10

//important define
#define MB_ADDRESS_BROADCAST                            0x00//广播地址
#define MB_ADDRESS_MIN                                  0x01//最小从机地址
#define MB_ADDRESS_MAX                                  247 //最大从机地址
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

//modbus error返回错误码
#define MDB_NO_ERROR       				0x0//无错误
#define MDB_FNO_INVALID                                 0x1//非支持的功能
#define MDB_REG_ADDR_INVALID                            0x2//寄存器地址不正确
#define MDB_REG_VALUE_OVERRIDE                          0x3//寄存器值超出范围
#define MDB_OP_FAIL                                     0x4//操作失败

#define MB_EX_NONE 					(0x00)//Modbus无错误
#define MB_EX_ILLEGAL_FUNCTION 		                (0x01)//Modbus异常功能码
#define MB_EX_ILLEGAL_DATA_ADDRESS	                (0x02)//Modbus无效数据地址
#define MB_EX_ILLEGAL_DATA_VALUE	                (0x03)//Modbus无效数值
#define MB_EX_SLAVE_DEVICE_FAILURE	                (0x04)//Modbus访问设备失败
#define MB_EX_ACKNOWLEDGE			        (0x05)//Modbus异常应答
#define MB_EX_SLAVE_BUSY			        (0x06)//Modbus从设备忙
#define MB_EX_MEMORY_PARITY_ERROR	                (0x08)//Modbus校验错误
#define MB_EX_GATEWAY_PATH_FAILED	                (0x0A)
#define MB_EX_GATEWAY_TGT_FAILED	                (0x0B)

#define ASCII_HOLDING_REG_REGION1_BGEIN			0x30//保持寄存器段1起始地址
#define ASCII_HOLDING_REG_REGION1_END			0x54//保持寄存器段1结束地址
#define ASCII_HOLDING_REG_REGION2_BGEIN			0x60//保持寄存器段2开始地址
#define ASCII_HOLDING_REG_REGION2_END			0x60//保持寄存器段2结束地址
#define ASCII_HOLDING_REG_REGION3_BGEIN			0x80//保持寄存器段3起始地址
#define ASCII_HOLDING_REG_REGION3_END		        0x80//保持寄存器段3结束地址
//#define ASCII_HOLDING_REG_EXTERN0_BEGIN                 0x70
//#define ASCII_HOLDING_REG_EXTERN0_END                   0x73

#define ASCII_INPUT_REG_REGION1_BGEIN        		0x00//输入寄存器段1起始地址
#define ASCII_INPUT_REG_REGION1_END          		0x1A//输入寄存器段1结束地址
#define ASCII_INPUT_REG_REGION2_BGEIN         		0x80//输入寄存器段2起始地址
#define ASCII_INPUT_REG_REGION2_END           		0x84//输入寄存器段2结束地址

#define ASCII_SINGLE_COIL_ADDR_BGEIN          		0x50//单个寄存器起始地址
#define ASCII_SINGLE_COIL_ADDR_END            		0x53//单个寄存器结束地址

#define ASCII_SPECICAL_COIL_REGION1_BGEIN    		0x00//特殊寄存器段1起始地址
#define ASCII_SPECICAL_COIL_REGION1_END      		0x03//特殊寄存器段1结束地址
#define ASCII_SPECICAL_COIL_REGION2_BGEIN    		0x40//特殊寄存器段2起始地址
#define ASCII_SPECICAL_COIL_REGION2_END      		0x40//特殊寄存器段2结束地址

#define ASCII_MUL_REG_REGION1_BGEIN                     ASCII_HOLDING_REG_REGION1_BGEIN //多个寄存器段1起始地址
#define ASCII_MUL_REG_REGION1_END            	        ASCII_HOLDING_REG_REGION1_END   //多个寄存器段1结束地址
#define ASCII_MUL_REG_REGION2_BGEIN                     ASCII_HOLDING_REG_REGION2_BGEIN //多个寄存器段2起始地址
#define ASCII_MUL_REG_REGION2_END            	        ASCII_HOLDING_REG_REGION2_END   //多个寄存器段2结束地址
#define ASCII_MUL_REG_REGION3_BGEIN                     ASCII_HOLDING_REG_REGION3_BGEIN //多个寄存器段3起始地址
#define ASCII_MUL_REG_REGION3_END            	        ASCII_HOLDING_REG_REGION3_END   //多个寄存器段3结束地址
#define ASCII_MUL_REG_EXTERN0_BEGIN                     0x70//多个寄存器扩展起始地址
#define ASCII_MUL_REG_EXTERN0_END                       0x73//多个寄存器扩展结束地址

#define ASCII_DB_HOLD_REG_REGION1_BEGIN                 0x00//双个寄存器段1起始地址
#define ASCII_DB_HOLD_REG_REGION1_END                   0x74//双个寄存器段1结束地址
#define ASCII_DB_HOLD_REG_REGION2_BEGIN                 0x7E//双个寄存器段2起始地址
#define ASCII_DB_HOLD_REG_REGION2_END                   0x9A//双个寄存器段2结束地址
#define ASCII_DB_HOLD_REG_REGION3_BEGIN                 0xB0//双个寄存器段3起始地址
#define ASCII_DB_HOLD_REG_REGION3_END                   0xB6//双个寄存器段3结束地址

#define ASCII_PRODUCTION_INF_BEGIN                      0xE0//产品版本信息段起始地址
#define ASCII_PRODUCTION_INF_END                        0xE5//产品版本信息段结束地址

#define UART_WAIT_TIME	                                100 //消息帧响应超时时间
#define DATAPACKET_LEN                                  80  //数据包最大长度

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





#define ASCII_HEAD_STATE  0     //头
#define ASCII_END_STATE   1     //结束
#define ASCII_IDLE_STATE  2     //空闲

#define ASCII_HEAD_DATA   ':'           //包头第一个字节

void MODBUS_ASCII_HexToAscii(uint8_t cyHexData, uint8_t *pCyAsciiBuf);
uint8_t MODBUS_ASCII_AsciiToHex(uint8_t *pCyAsciiBuf);
uint8_t MODBUS_ASCII_GetLrc(uint8_t *pCyAsciiBuf, uint8_t cyLen);
uint8_t MODBUS_ASCII_AsciiPacketToRtuPacket(uint8_t *pCyAsciiBuf, uint8_t cyAsciiLen, uint8_t *pCyRtuBuf);
uint8_t MODBUS_ASCII_RtuPacketToAsciiPacket(uint8_t *pCyRtuBuf, uint8_t cyRtuLen, uint8_t *pCyAsciiBuf);
void MODBUS_ASCII_HandlRevData(uint8_t cyRevData);
uint8_t MODBUS_ASCII_CheckAscii(uint8_t *pCyAsciiBuf, uint8_t cyLen);
uint8_t MODBUS_ASCII_RecvData(uint8_t* cyRecvBuff, uint8_t *pCyLen);


#endif

