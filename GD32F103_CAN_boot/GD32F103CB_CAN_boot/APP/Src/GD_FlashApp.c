/**@file        GD_FlashApp.c
* @brief        Flash模块的应用
* @details      适用于GD32F10x系列单片机,写入读取Flash函数
* @author       马灿林
* @date         2021-3-30
* @version      V1.0.0
* @copyright    2021-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/3/30   <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/
#include "GD_FlashApp.h"

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
	return Flash_Write_MultiBytes(RWAddr, &WrData, 1);
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
	buf_WrAddr = (1024-(RWAddr % FLASH_PAGE_SIZE));
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
	fmc_unlock();			//解锁主FMC操作
	fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	fmc_page_erase(WrAddr);	//擦除指定地址页

    for(i = 0; i < FLASH_PAGE_SIZE;)
    {
		fmc_halfword_program((WrAddr + i), (uint16_t)Flash_Buf[i/2]); //从指定页的addr地址开始写
		i+=2;
	}
	if( (*(__IO uint8_t *)RWAddr) !=  pWrbuf[0])		
		return 0;				//写入错误
	if(Wrlen1 != 0)
	{
		Flash_Write_MultiBytes(RWAddr1, pWrbuf+buf_WrAddr,Wrlen1);
		RWAddr1 = 0;
		Wrlen1=0;
	}
	
//	fmc_unlock();
//	fmc_flag_clear(FMC_FLAG_BANK0_END);
//    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
//    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
//    fmc_lock();					//锁定FMC主操作
	return 1;
}

void ErasureFlashPage(uint32_t EWAddr,uint16_t PageCount)
{

	fmc_unlock();			//解锁主FMC操作
	fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	EWAddr = (EWAddr / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;	
	for(;PageCount > 0;PageCount--)
	{
		fmc_page_erase(EWAddr);	//擦除指定地址页
		EWAddr += 1024;
	}
	
}
