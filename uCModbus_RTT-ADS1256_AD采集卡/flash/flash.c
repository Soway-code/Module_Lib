/**@file        flash.c
* @brief        读写flash模块
* @details      适用于STM32F10x系列单片机,写入读取Flash函数
* @author       马灿林
* @date         2020-11-20
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/11/20  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#include "flash.h"

static uint16_t Flash_Buf[FLASH_PAGE_SIZE/2];    ///< 临时缓存


/**@brief       向内部Flash指定位置读取一字节数据
* @param[in]    RWAddr : 读取地址
* @return       函数执行结果 - 1个字节数据
* @note         
*/
uint8_t Flash_Read_OneByte(uint32_t RWAddr)
{
  uint8_t RdData;
  RdData = *(__IO uint8_t *)RWAddr;
  return RdData;
}

/**@brief       向内部Flash指定位置写入一字节数据
* @param[in]    RWAddr : 写入地址
* @param[in]    WrData : 写入数据
* @return       函数执行结果  1 成功  0 失败
* @note         
*/
uint8_t Flash_Write_OneByte(uint32_t RWAddr, uint8_t WrData)
{
    uint32_t WrAddr;
    uint16_t i;
    uint8_t *buf = (uint8_t *)Flash_Buf;

    WrAddr = (RWAddr / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
    Flash_Read_MultiBytes(WrAddr, buf, FLASH_PAGE_SIZE);
    buf[RWAddr % FLASH_PAGE_SIZE] = WrData;
    /* 解锁flash程序/擦除控制器 */
	FLASH_Unlock();  //解锁FLASH编程擦除控制器
    FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除标志位
	
    if(buf[RWAddr % FLASH_PAGE_SIZE] != 0xFF)
    {
		FLASH_ErasePage(RWAddr);     //擦除指定地址页
    }
   
    for(i = 0; i < FLASH_PAGE_SIZE;)
    {
		 FLASH_ProgramHalfWord((WrAddr + i), (uint16_t)Flash_Buf[i/2]); //从指定页的addr地址开始写
		i+=2;
	}
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除标志位
    FLASH_Lock();    //锁定FLASH编程擦除控制器
	if( (*(__IO uint8_t *)RWAddr) !=  WrData)
		return 0;
	return 1;
}


/**@brief       向内部Flash指定位置读取二字节数据
* @param[in]    RWAddr : 读取地址
* @return       函数执行结果 - 2个字节数据
* @note         
*/
uint16_t Flash_Read_twoByte(uint32_t RWAddr)
{
  uint16_t RdData;
  RdData = *(__IO uint16_t *)RWAddr;
  return RdData;
}

/**@brief       向内部Flash指定位置写入二字节数据
* @param[in]    RWAddr : 写入地址
* @param[in]    WrData : 写入数据
* @return       函数执行结果  1 成功  0 失败
* @note         
*/
uint8_t Flash_Write_twoByte(uint32_t RWAddr, uint16_t WrData)
{
	uint8_t H,L;
    uint8_t buf[2];
	
	H = (uint8_t)(WrData>>8);
	L = (uint8_t)(WrData);
    buf[0] = L;
	buf[1] = H;
	return Flash_Write_MultiBytes(RWAddr,buf,2);
}


/**@brief       向内部Flash指定位置读取四字节数据
* @param[in]    RWAddr : 读取地址
* @return       函数执行结果 - 4个字节数据
* @note         
*/
uint32_t Flash_Read_fourByte(uint32_t RWAddr)
{
  uint32_t RdData;
  RdData = *(__IO uint32_t *)RWAddr;
  return RdData;
}

/**@brief       向内部Flash指定位置写入四字节数据
* @param[in]    uint32_t RWAddr : 写入地址
* @param[in]    uint32_t WrData : 写入数据
* @return       函数执行结果  1 成功  0 失败
* @note         
*/
uint8_t Flash_Write_fourByte(uint32_t RWAddr, uint32_t WrData)
{
	uint8_t H0,H1,L0,L1;
    uint8_t buf[4];
	
	H0= (uint8_t)(WrData>>24);
	H1= (uint8_t)(WrData>>16);
	L0= (uint8_t)(WrData>>8);
	L1 = (uint8_t)(WrData);

    buf[0] = L1;
	buf[1] = L0;
	buf[2] = H1;
	buf[3] = H0;
	return Flash_Write_MultiBytes(RWAddr,buf,4);
}


/**@brief       从内部Flash指定位置读多个字节
* @param[in]    RWAddr : 读起始地址
* @param[in]    pRdbuf : 数据缓存指针
* @param[in]    Rdlen : 读数据长度
* @return       函数执行结果
* @note        
*/
void Flash_Read_MultiBytes(uint32_t RWAddr, uint8_t *pRdbuf, uint16_t Rdlen)
{
  uint16_t FlashCnt;
  for(FlashCnt = 0; FlashCnt < Rdlen; FlashCnt++)
  {
    *(pRdbuf + FlashCnt) = Flash_Read_OneByte(RWAddr);
    RWAddr++;
  }
}

/**@brief       向内部Flash指定位置写多个字节
* @param[in]    RWAddr : 写起始地址
* @param[in]    pWrbuf : 数据缓存指针
* @param[in]    Wrlen : 写数据长度
* @return       函数执行结果 1 成功  0 失败
* @note  		Wrlen < 2048,单次最大可以写2048个字节，可跨页写
*/
uint8_t Flash_Write_MultiBytes(uint32_t RWAddr, uint8_t const *pWrbuf, uint16_t Wrlen)
{
	uint32_t WrAddr;
    uint16_t i;
    uint8_t *buf = (uint8_t *)Flash_Buf;
	
	uint32_t buf_WrAddr;
	uint32_t RWAddr1 = 0;
	uint16_t Wrlen1=0;
	
	WrAddr = (RWAddr / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;		
	buf_WrAddr = (2048-(RWAddr % FLASH_PAGE_SIZE));
	if(Wrlen > buf_WrAddr)
	{
		RWAddr1 = RWAddr + buf_WrAddr;
		Wrlen1 = (Wrlen - buf_WrAddr);
	}
    Flash_Read_MultiBytes(WrAddr, buf, FLASH_PAGE_SIZE);
    for(i=0;i<(Wrlen-Wrlen1);i++)
	{
		buf[(RWAddr+i) % FLASH_PAGE_SIZE] = pWrbuf[i];
	}
	 FLASH_Unlock();  //解锁FLASH编程擦除控制器
     FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除标志位
     FLASH_ErasePage(RWAddr);     //擦除指定地址页
  
    for(i = 0; i < FLASH_PAGE_SIZE;)
    {
		FLASH_ProgramHalfWord((WrAddr + i), (uint16_t)Flash_Buf[i/2]); //从指定页的addr地址开始写
		i+=2;
	}
	if( (*(__IO uint8_t *)RWAddr) !=  pWrbuf[0])
		return 0;
	if(Wrlen1 != 0)
	{
		Flash_Write_MultiBytes(RWAddr1, pWrbuf+buf_WrAddr,Wrlen1);
		RWAddr1 = 0;
		Wrlen1=0;
	}
	return 1;
}


/**@brief       FLASH写入数据
* @param[in]    add :32位FLASH地址。
* @param[in]    dat :16位数据
* @note      \n
*/
void FLASH_W(u32 add,u16 dat)
{ 
	 uint16_t buf[8],i=0;
	//地址在范围
	if((ADD_I0 == add) || (ADD_I1 == add) || (ADD_I2 == add) || (ADD_I3 == add) ||
		(ADD_I4 == add) || (ADD_I5 == add) || (ADD_I6 == add) || (ADD_I7 == add) ||  (SIGN == add))
	{
		//读出数据
		if(ADD_I0 != add)
			buf[0] = FLASH_R(ADD_I0);
		else
			buf[0] = dat;
		
		if(ADD_I1 != add)
			buf[1] = FLASH_R(ADD_I1);
		else
			buf[1] = dat;
		
		if(ADD_I2 != add)
			buf[2] = FLASH_R(ADD_I2);
		else
			buf[2] = dat;
		
		if(ADD_I3 != add)
			buf[3] = FLASH_R(ADD_I3);
		else
			buf[3] = dat;
		
		if(ADD_I4 != add)
			buf[4] = FLASH_R(ADD_I4);
		else
			buf[4] = dat;
		
		if(ADD_I5 != add)
			buf[5] = FLASH_R(ADD_I5);
		else
			buf[5] = dat;
		
		if(ADD_I6 != add)
			buf[6] = FLASH_R(ADD_I6);
		else
			buf[6] = dat;
		
		if(ADD_I7 != add)
			buf[7] = FLASH_R(ADD_I7);
		else
			buf[7] = dat;
		
		if(add == SIGN)
			for(i=0;i < 8;)
			{
				buf[i] = 32000;			//0值
				i++;
			}
		
		FLASH_Unlock();  //解锁FLASH编程擦除控制器
		FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除标志位
		FLASH_ErasePage(ADD_I0);     //擦除指定地址页
		for(i=0;i < 8;)
		{
			FLASH_ProgramHalfWord((ADD_I0+(i*2)),buf[i]); //开始写
			i++;
		}
		if(add == SIGN)
			FLASH_ProgramHalfWord(SIGN,0X55AA); //写标志
		
		FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除标志位
		FLASH_Lock();    //锁定FLASH编程擦除控制器
	}
}

/**@brief       FLASH读出数据
* @param[in]    add : 32位读出FLASH地址;
* @param[out]   ADC_TemperParam : 温度转换需要的参数结构指针; 
* @return       返回值：16位数据
* @note      \n
*/
u16 FLASH_R(u32 add){ 
	u16 a;
    a = *(u16*)(add);//从指定页的addr地址开始读
return a;
}
