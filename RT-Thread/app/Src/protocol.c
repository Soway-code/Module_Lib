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

uint8_t ID[3]= {0x30,0x30,0x30};
char TX_buf[200];

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
	uint8_t i = 0;
	while(RX[i+2] != 0x0D)
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

/**@brief       D01命令处理
* @note         GPS 设备读取重量命令:由终端或电脑发往传感器的查询测量值命令 D01
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D01(void)
{
	uint8_t i = 10;
	//报警标志位 1字节
	TX_buf[i++] = 0x30;	
	
	//空重载标志位 1字节
	TX_buf[i++] = 0x30;
	
	//当前 AD 值 4字节
	TX_buf[i++] = 0x30;
	TX_buf[i++] = 0x33;
	TX_buf[i++] = 0x41;
	TX_buf[i++] = 0x42;
	
	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}

/**@brief       D11命令处理
* @note         由终端或电脑发往传感器的空载校准命令 D11
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D11(void)
{
	uint8_t i = 10;
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}


/**@brief       D21命令处理
* @note         由终端或电脑发往传感器的满载校准命令 D21
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D21(void)
{
	uint8_t i = 10;
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}

/**@brief       D31命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或电脑发往传感器的设定设备 ID 号命令 D31
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D31(const char *RX)		//使用到falsh
{
	uint8_t i = 10;
	
	ID[0] = RX[10];
	ID[1] = RX[11];
	ID[2] = RX[12];
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}

/**@brief       D41命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或电脑发往传感器的设定报警阈值和阻尼时间命令 D41
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D41(const char *RX)		//使用到falsh 
{
	uint8_t i = 10;
	static uint8_t a=0,a1 = 0;	
	
	//传感器变化的阈值
	a = (AsciiToDecimalism(RX[10])*10);
	a += (AsciiToDecimalism(RX[11]));
	
	//阻尼时间
	a1 = (AsciiToDecimalism(RX[12])*10);
	a1 += (AsciiToDecimalism(RX[13]));
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}

/**@brief       D51命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或者电脑发送给传感器的校准值查询命令：D51
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D51(const char *RX)		//使用到falsh 
{
	uint8_t i = 10;
	
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}

/**@brief       D61命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或电脑发往传感器的校准设置命令：D61
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D61(const char *RX)		//使用到falsh 
{
	uint8_t i = 10;
	
	//成功回复 OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}

/**@brief       D71命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或电脑发往传感器的载重报警值设置命令：D71
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D71(const char *RX)		//使用到falsh 
{
	uint8_t i = 10;
	
	static uint32_t Null_Ad = 0,Reload_Ad = 0,Overload_Ad = 0;		//车辆空载 AD,车辆重载 AD,车辆超载 AD 值;
	
	//车辆空载 AD 值
	Null_Ad = AsciiToDecimalism(RX[10]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[11]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[12]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[13]);
	
	//车辆重载 AD 值
	Reload_Ad = AsciiToDecimalism(RX[14]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[15]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[16]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[17]);
	
	//车辆重载 AD 值	
	Overload_Ad = AsciiToDecimalism(RX[18]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[19]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[20]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[21]);
	
	if(((Null_Ad != 0x0) && (Reload_Ad != 0x0) && (Overload_Ad != 0x0)) ||
		((Null_Ad != 0xFFFF) && (Reload_Ad != 0xFFFF) && (Overload_Ad != 0xFFFF)))	//成功回复 OK
	{	
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else		//失败回复 NO
	{
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}

/**@brief       D81命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或者电脑发给传感器的查询载重值命令：D81
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D81(const char *RX)		//使用到falsh 
{
	uint8_t i = 10;
	
//加判断
	TX_buf[i++] = '0';		//报警标志位
	
	TX_buf[i++] = '0';		//空重载标志位

	//16位数据吨数
	TX_buf[i++] = '0';		
	TX_buf[i++] = '3';		
	TX_buf[i++] = 'A';		
	TX_buf[i++] = '3';		
	
	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
}

/**@brief       D91命令处理
* @param[in]    *RX : 串口接收到数据包;
* @note         由终端或者电脑发给传感器的载重值标定命令：D91
				数据格式详细说明查看：(定制)载重测量传感器_定位器_平台协议.pdf; \n
*/
void D91(const char *RX)		//使用到falsh 
{
	uint8_t i = 10;
	uint8_t count = 0;		//标定值数量
	uint16_t D91_Ad = 0,D91_load = 0;

	//标定值数量
	count = AsciiToDecimalism(RX[10]);
	count <<= 4;
	count |= AsciiToDecimalism(RX[11]);	
	
	

	CheckCode(i);		//添加校验码
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* 清零 */
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
	return 0;

}

