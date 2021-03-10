/**@file        protocol.c
* @brief        (定制)载重测量传感器_定位器_协议
* @details      数据包数据处理
* @author      	马灿林
* @date         2021-1-8
* @version      V1.0.0
* @copyright    2021-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/1/8    <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/
#include "protocol.h"
#include "bsp_usart.h"
#include "adc.h"
#include "Flash_app.h"
#include "mpu6050.h"
#include "DataTreating.h"

extern uint16_t ADC0_filter; //声明外部变量
extern double X_Angle,Y_Angle,Z_Angle;		//加速度传感器数据
uint16_t NullCalibrat;					//空载校准
uint16_t FullCalibrat;					//满载校准
uint8_t ID[3] ={0x30,0x30,0x30};		//设备ID
uint8_t Threshold=0;					//传感器阈值
uint8_t DampTime= 0;					//阻尼时间	
uint16_t Null_Ad = 0,Reload_Ad = 0,Overload_Ad = 0;		//车辆空载 AD,车辆重载 AD,车辆超载 AD 报警值;
uint8_t D01_sign[2]={0,0};			//报警标志位-空重载标志位
uint8_t TX_buf[200];					//发送数据缓存
uint16_t ton = 0;				//重量

/**@brief       十六进制转换ASCII
* @param[in]    hex_byte : 需要转换的十六进制;
* @return       函数执行结果 ascii
* @note     
*/
char HexToAscii(char hex_byte)
{
    char result;
    if((hex_byte>=0)&&(hex_byte<=9))            //变成ascii数字
        result = hex_byte + 0x30;
    else if((hex_byte >= 10)&&(hex_byte <= 15)) //变成ascii大写字母
        result = hex_byte + 0x37;
    else
        result = 0xff;
    return result;
}

/**@brief       ASCII转换十进制
* @param[in]    hex_byte : 需要转换的ASCII;
* @return       函数执行结果 十进制
* @note     
*/
uint8_t AsciiToDecimalism(char byte)
{
 	if((byte <= 57) && (byte >= 48))
		return (byte - 48);
	else if((byte <= 90) && (byte >= 65))
		return (byte - 55);
	else if((byte <= 122) && (byte >= 97))
		return (byte - 87);
	return 0;
}

/**@brief       接收数据和校验验证
* @param[in]    *RX : 串口接收到数据包;
* @return       函数执行结果
* - 1(校验码正确)
* - 0(校验码错误)
* @note     
*/
uint8_t Verification(const char *RX)
{
	uint16_t sum = 0;
	uint16_t i = 0;
	while((RX[i+2] != 0x0D) && (i < 1024))
	{
		sum += RX[i];
		i++;
	}
	sum &= 0x00FF; 
	
	if(((HexToAscii((sum/0x10))) == RX[i++]) && ((HexToAscii((sum%0x10))) == RX[i]))
		return 1;
	return 0;
}

/**@brief       ID号验验证
* @param[in]    *RX : 串口接收到数据包;
* @return       函数执行结果
* - 1(ID号正确)
* - 0(ID号错误)
* @note     
*/
uint8_t IDVerification(const char *RX)
{
	if((ID[0] == RX[7]) && (ID[1] == RX[8]) && (ID[2] == RX[9]))
		return 1;
	return 0;
}

/**@brief       校验和生成
* @param[in]  	length：数据包的长度，不包括校验码
* @note     	此函数将计算好的校验码添加到后面*TX
*/
void CheckCode(uint8_t length)
{
	uint16_t sum = 0;
	uint8_t i = 0;
	while(length--)
	{
		sum += TX_buf[i];
		i++;
	}
	sum &= 0x00FF;
	TX_buf[i++] = (HexToAscii((sum/0x10)));
	TX_buf[i] = (HexToAscii((sum%0x10)));
}

void Parameter_Init()		//从flash读出参数
{
	if(Flash_Read_OneByte(FIRST) != 0xa5)	//首次上电
	{
		//传感器角度平面数据修正值（消除芯片固定误差）
		Flash_Write_fourByte(X_ACCEL_ADDR,270);
		Flash_Write_fourByte(Y_ACCEL_ADDR,660);
		Flash_Write_fourByte(Z_ACCEL_ADDR,-19422);
		
		Flash_Write_MultiBytes(ID_ADDR,ID,3);	//写入默认ID=000
		
		Flash_Write_OneByte(VPT_ADDR,15);		//阈值默认15%
		
		Flash_Write_OneByte(DAMPTE,12);			//阻尼时间默认12*5=60s	
		
		//默认载重报警值
		Flash_Write_fourByte(NULL_AD_ADDR,100);
		Flash_Write_fourByte(RELOAD_AD_ADDR,3000);
		Flash_Write_fourByte(OVERLOAD_AD_ADDR,3500);
		
		Flash_Write_fourByte(NULL_CAIIBRATION,0);		//默认空载校准0
		Flash_Write_fourByte(FULL_CAIIBRATION,3800);	//默认满载校准3800
		
		Flash_Write_OneByte(SIGN_ADDR,0);		//默认标定组数据为0
		Flash_Write_OneByte(NOW_ADDR,0);		//默认已经标定数据为0	
		
		Flash_Write_OneByte(FIRST,0xa5);		//写入参数设置成默认标志
	}
	NullCalibrat = Flash_Read_fourByte(NULL_CAIIBRATION);	//读出空载校准
	FullCalibrat = Flash_Read_fourByte(FULL_CAIIBRATION);	//读出满载校准
	Flash_Read_MultiBytes(ID_ADDR,ID,3);					//读从ID
	Threshold = Flash_Read_OneByte(VPT_ADDR);				//阈值
	DampTime = Flash_Read_OneByte(DAMPTE);					//阻尼时间
	Null_Ad = Flash_Read_fourByte(NULL_AD_ADDR);			//车辆空载 AD 值报警值
	Reload_Ad = Flash_Read_fourByte(RELOAD_AD_ADDR);		//车辆重载 AD值
	Overload_Ad = Flash_Read_fourByte(OVERLOAD_AD_ADDR);	//车辆超载 报警AD值;
}
/**@brief       D01命令处理
* @note         GPS 设备读取重量命令:由终端或电脑发往传感器的查询测量值命令 D01
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D01(void)
{
	uint8_t buf[4]={0};
	uint8_t i = 10;
	
	//报警标志位 1字节
	TX_buf[i++] = HexToAscii(D01_sign[0]);	
	
	//空重载标志位 1字节
	TX_buf[i++] = HexToAscii(D01_sign[1]);

	buf[0] = (ADC0_filter & 0xF000)>>12;
	buf[1] = (ADC0_filter & 0x0F00)>>8;
	buf[2] = (ADC0_filter & 0x00F0)>>4;
	buf[3] = (ADC0_filter & 0x000F);
	
	//当前 AD 值 4字节
	TX_buf[i++] = HexToAscii(buf[0]);
	TX_buf[i++] = HexToAscii(buf[1]);
	TX_buf[i++] = HexToAscii(buf[2]);
	TX_buf[i++] = HexToAscii(buf[3]);
	
	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;

	memset(TX_buf,0,i+4);/* 清零 */
}

/**@brief       D11命令处理
* @note         由终端或电脑发往传感器的空载校准命令 D11
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D11(void)
{
	uint8_t i = 10;

	if(Flash_Write_fourByte(ADC0_filter,NULL_CAIIBRATION) == 1)
	{
		//成功回复 OK
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//失败回复 NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
}


/**@brief       D21命令处理
* @note         由终端或电脑发往传感器的满载校准命令 D21
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D21(void)
{
	uint8_t i = 10;
	if(Flash_Write_fourByte(ADC0_filter,FULL_CAIIBRATION) == 1)
	{
		//成功回复 OK
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//失败回复 NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
}

/**@brief       D25命令处理
* @note         平面角度全部校准为90度
				新增; \n
*/
void D25(void)
{
	uint8_t i = 10;
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
	AngleCalibration();			//平面角度校准
}

/**@brief       D25命令处理
* @note         平面角度全部校准为90度
				新增; \n
*/
void D26(void)
{
	uint8_t i = 10;
	
	Flash_Write_twoByte(X_45ACCEL,(uint16_t)X_Angle);
	Flash_Write_twoByte(Y_45ACCEL,(uint16_t)Y_Angle);
	Flash_Write_twoByte(Z_45ACCEL,(uint16_t)Z_Angle);
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
	
}

/**@brief       D31命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或电脑发往传感器的设定设备 ID 号命令 D31
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D31(const char *RX)		
{
	uint8_t i = 10;
	
	if(Flash_Write_MultiBytes(ID_ADDR,(uint8_t*)RX[10],3) == 1)
	{
		//成功回复 OK
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//失败回复 NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	ID[0] = RX[10];
	ID[1] = RX[11];
	ID[2] = RX[12];
	
	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
}

/**@brief       D41命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或电脑发往传感器的设定报警阈值和阻尼时间命令 D41
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D41(const char *RX)		
{
	uint8_t i = 10;
	uint8_t j=0;	
	
	//传感器变化的阈值
	Threshold = (AsciiToDecimalism(RX[10])*10);
	Threshold += (AsciiToDecimalism(RX[11]));
	if((Threshold >= 0) && (Threshold <= 0xFF))
		j = Flash_Write_OneByte(VPT_ADDR,Threshold);
	
	//阻尼时间
	DampTime = (AsciiToDecimalism(RX[12])*10);
	DampTime += (AsciiToDecimalism(RX[13]));
	if((DampTime >= 0) && (DampTime <= 0xFF))
		j += Flash_Write_OneByte(DAMPTE,DampTime);
	
	if(j == 2)
	{
		//成功回复 OK
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//失败回复 NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	
	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
}


/**@brief       D51命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或者电脑发送给传感器的校准值查询命令：D51
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D51(const char *RX)		 
{
	uint8_t i = 10;
	uint8_t buf[4]={0};
	
	//校准空点
	buf[0] = (NullCalibrat & 0xF000)>>12;
	buf[1] = (NullCalibrat & 0x0F00)>>8;
	buf[2] = (NullCalibrat & 0x00F0)>>4;
	buf[3] = (NullCalibrat & 0x000F);
	TX_buf[i++] = HexToAscii(buf[0]);
	TX_buf[i++] = HexToAscii(buf[1]);
	TX_buf[i++] = HexToAscii(buf[2]);
	TX_buf[i++] = HexToAscii(buf[3]);
	
	//校准满点
	buf[0] = (FullCalibrat & 0xF000)>>12;
	buf[1] = (FullCalibrat & 0x0F00)>>8;
	buf[2] = (FullCalibrat & 0x00F0)>>4;
	buf[3] = (FullCalibrat & 0x000F);
	TX_buf[i++] = HexToAscii(buf[0]);
	TX_buf[i++] = HexToAscii(buf[1]);
	TX_buf[i++] = HexToAscii(buf[2]);
	TX_buf[i++] = HexToAscii(buf[3]);
	
	//当前数据
	buf[0] = (ADC0_filter & 0xF000)>>12;
	buf[1] = (ADC0_filter & 0x0F00)>>8;
	buf[2] = (ADC0_filter & 0x00F0)>>4;
	buf[3] = (ADC0_filter & 0x000F);
	TX_buf[i++] = HexToAscii(buf[0]);
	TX_buf[i++] = HexToAscii(buf[1]);
	TX_buf[i++] = HexToAscii(buf[2]);
	TX_buf[i++] = HexToAscii(buf[3]);

	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
	Parameter_Init();
}

/**@brief       D61命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或电脑发往传感器的校准设置命令：D61
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D61(const char *RX)		//未完成********
{
	uint8_t i = 10;
	
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
}

/**@brief       D71命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或电脑发往传感器的载重报警值设置命令：D71
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D71(const char *RX)		//使用到falsh 
{
	uint8_t i = 10;
	uint8_t j = 0;
	
	//车辆空载 AD 值
	Null_Ad = AsciiToDecimalism(RX[10]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[11]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[12]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[13]);
	if((Null_Ad >= 0) && (Null_Ad <= 0xFFF))
		j = Flash_Write_fourByte(NULL_AD_ADDR,Null_Ad);
	
	//车辆重载 AD 值
	Reload_Ad = AsciiToDecimalism(RX[14]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[15]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[16]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[17]);
	if((Reload_Ad >= 0) && (Reload_Ad <= 0xFFF))
		j += Flash_Write_fourByte(RELOAD_AD_ADDR,Reload_Ad);
	
	//车辆重载 AD 值	
	Overload_Ad = AsciiToDecimalism(RX[18]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[19]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[20]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[21]);
	if((Overload_Ad >= 0) && (Overload_Ad <= 0xFFF))
		j += Flash_Write_fourByte(OVERLOAD_AD_ADDR,Overload_Ad);
	
	if(j == 3)	//成功回复 OK
	{	
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else		//失败回复 NO
	{
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	
	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
}

/**@brief       D81命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或者电脑发给传感器的查询载重值命令：D81
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D81(const char *RX)		
{
	uint8_t i = 10;
	
	//报警标志位 1字节
	TX_buf[i++] = HexToAscii(D01_sign[0]);	
	
	//空重载标志位 1字节
	TX_buf[i++] = HexToAscii(D01_sign[1]);

	//16位数据吨数
	TX_buf[i++] = HexToAscii((ton & 0xF000)>>12);		
	TX_buf[i++] = HexToAscii((ton & 0x0F00) >> 8);		
	TX_buf[i++] = HexToAscii((ton & 0x00F0) >> 4);		
	TX_buf[i++] = HexToAscii((ton & 0x000F));		
	
	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
}

/**@brief       D91命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或者电脑发给传感器的载重值标定命令：D91
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D91(const char *RX)	
{
	uint8_t i = 10,j=0,k=12;
	uint8_t error = 0; 					//错误标志
	uint8_t count = 0,count1,count2;		//标定值数量
	uint16_t ad[10],weight[10];						//ad数值和重量
	
	count1 = Flash_Read_OneByte(SIGN_ADDR);		//读出需要从第几组写入
	count2 = Flash_Read_OneByte(NOW_ADDR);		//读出已经标定数据
	//标定值数量
	count = AsciiToDecimalism(RX[i++]);
	count <<= 4;
	count |= AsciiToDecimalism(RX[i++]);
	
	for(;count > 0;count--)
	{
		if(count1 > 19)
			count1 = 0;
		ad[j] = AsciiToDecimalism(RX[k++]);
		ad[j] <<= 4;
		ad[j] |= AsciiToDecimalism(RX[k++]);
		ad[j] <<= 4;
		ad[j] |= AsciiToDecimalism(RX[k++]);
		ad[j] <<= 4;
		ad[j] |= AsciiToDecimalism(RX[k++]);
	
		weight[j] = AsciiToDecimalism(RX[k++]);
		weight[j] <<= 4;
		weight[j] |= AsciiToDecimalism(RX[k++]);
		weight[j] <<= 4;
		weight[j] |= AsciiToDecimalism(RX[k++]);
		weight[j] <<= 4;
		weight[j] |= AsciiToDecimalism(RX[k++]);
		if(ad[j] <= 0x0FFF)							//12位AD值在范围内是0X0FFF
		{
			Flash_Write_twoByte((AD1_ADDR+(count1*0x02)),ad[j]);
			Flash_Write_twoByte((LOAD1_ADDR+(count1*0x02)),weight[j++]);
			count1++;
		}
		else
			error = 1;
	}
	if((count1 > count2) && (error == 0))
	{
		count2 = count1 + count2;
		if(count2 > 20)
			count2 = 20;
		Flash_Write_OneByte(NOW_ADDR,count2);		//已经标定数据
	}
	if(error == 0)					//出现错误不记录次数
		Flash_Write_OneByte(SIGN_ADDR,count1);		//标定组数据
	i -= 2;
	if(error == 0)
	{
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//失败回复 NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	RS485_Tx;
	CheckCode(i);		//添加校验码
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* 清零 */
	D81_Dispose();	
}

/**@brief       数据包处理
* @param[in]    *RX : 串口接收到数据包;
* @return       函数执行结果
* - 1(成功)
* - 0(失败)
* @note        \n
*/
uint8_t PacketHandlingModule(const char *RX)
{
	uint8_t i=0;
	
	//ID验证和校验码验证 验证失败直接返回不做任何处理
	if((IDVerification(RX) != 1) || (Verification(RX) != 1))		
		return 0;
	
	for(;i<10 ;i++)					//获取发送需要的固定部分数据
		TX_buf[i] = RX[i];
	
	if(strncmp(RX+4,"D01",3) == 0)
		D01();
	else if(strncmp(RX+4,"D11",3) == 0)
		D11();
	else if(strncmp(RX+4,"D21",3) == 0)
		D21();
	else if(strncmp(RX+4,"D25",3) == 0)			//新增平面角度校准
		D25();
	else if(strncmp(RX+4,"D26",3) == 0)			//新增山坡校准
		D26();
	else if(strncmp(RX+4,"D31",3) == 0)
		D31(RX);			//*设置ID
	else if(strncmp(RX+4,"D41",3) == 0)
		D41(RX);			//*设定报警阈值和阻尼时间
	else if(strncmp(RX+4,"D51",3) == 0)
		D51(RX);			//*校准值查询
	else if(strncmp(RX+4,"D61",3) == 0)
		D61(RX);			//*校准值查询	
	else if(strncmp(RX+4,"D71",3) == 0)
		D71(RX);			//*载重报警值设置	
	else if(strncmp(RX+4,"D81",3) == 0)
		D81(RX);			//*查询载重值
	else if(strncmp(RX+4,"D91",3) == 0)
		D91(RX);			//*查询载重值
	return 0;

}

