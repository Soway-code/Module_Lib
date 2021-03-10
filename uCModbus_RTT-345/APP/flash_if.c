//**************************************************************************************************
//  Copyright (C), 2013-2023, The Chinese University of Hong Kong
//  FileName:      flash_if.c
//  Author:        chenjin      
//  Version:       1.0.0.0        
//  Date:          2015/11/17
//  Description:   FLASH操作
//  Function List: 
//  History:         // 历史修改记录
//  	author:       
//      Date:         
//      version:      
//      desc:         
//**************************************************************************************************

#include "stm32f0xx.h"
#include "flash_if.h"

//**************************************************************************************************
// Name         : FLASH_If_Init()
// CreateDate   : 2015-11-17
// Author       : chenjin
// Function     : 初始化FLASH
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

void FLASH_If_Init(void)
{ 
    //Unlock the Program memory 
    FLASH_Unlock();
    //Clear all FLASH flags 
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR | FLASH_FLAG_BSY);   
}

//**************************************************************************************************
// Name         : FLASH_If_Erase()
// CreateDate   : 2015-11-17
// Author       : chenjin
// Function     : 删除FLASH
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

uint32_t FLASH_If_Erase(uint32_t dwStartSector)
{
  	uint32_t dwFlashaddress;
  
  	dwFlashaddress = dwStartSector;
  
  	while (USER_FLASH_LAST_PAGE_ADDRESS > dwFlashaddress)
  	{
	    if (FLASH_COMPLETE == FLASH_ErasePage(dwFlashaddress) )
	    {
	      	dwFlashaddress += FLASH_PAGE_SIZE;
	    }
	    else
	    {
	      	//Error occurred while page erase 
	      	return (1);
	    }
  	}
  	
  	return (0);
}

//**************************************************************************************************
// Name         : FLASH_If_Write()
// CreateDate   : 2015-11-17
// Author       : chenjin
// Function     : 写FLASH
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************
/*
uint32_t FLASH_If_Write(__IO uint32_t* dwFlashAddress, uint32_t *pDwData, uint16_t wDataLength)
{
  	uint32_t i = 0;

  	for (i = 0; (i < wDataLength) && (*dwFlashAddress <= (USER_FLASH_END_ADDRESS - 4) ); i++)
  	{
    	//the operation will be done by word 
    	if (FLASH_COMPLETE == FLASH_ProgramWord(*dwFlashAddress, *(uint32_t*)(pDwData + i) ) )
    	{
     		//Check the written value 
      		if (*(uint32_t*)*dwFlashAddress != *(uint32_t*)(pDwData + i) )
      		{
        		//Flash content doesn't match SRAM content 
        		return(2);
      		}
      		//Increment FLASH destination address 
      		*dwFlashAddress += 4;
    	}
    	else
    	{
      		// Error occurred while writing data in Flash memory 
      		return (1);
    	}
  	}

  	return (0);
}*/

uint32_t FLASH_If_Write(uint32_t *dwFlashAddress, uint16_t *pWData, uint16_t wDataLength)
{
  	uint16_t i;

  	for (i = 0; (i < wDataLength) && (*dwFlashAddress <= (USER_FLASH_END_ADDRESS - 2) ); i++)
  	{
    	//the operation will be done by word 
    	if (FLASH_COMPLETE == FLASH_ProgramHalfWord(*dwFlashAddress, *(pWData + i) ) )
    	{
     		//Check the written value 
      		if (*(vu16*)(*dwFlashAddress) != *(uint16_t*)(pWData + i) )
      		{
        		//Flash content doesn't match SRAM content 
        		return(2);
      		}
      		//Increment FLASH destination address 
      		*dwFlashAddress += 2;
    	}
    	else
    	{
      		// Error occurred while writing data in Flash memory 
      		return (1);
    	}
  	}

  	return (0);
}


//**************************************************************************************************
// Name         : FLASH_If_ReadHalfWord()
// CreateDate   : 2015-11-17
// Author       : chenjin
// Function     : 读半个Word
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

static uint16_t FLASH_If_ReadHalfWord(uint32_t faddr)
{
	return *(vu16*)faddr;
}

//**************************************************************************************************
// Name         : LASH_If_Read()
// CreateDate   : 2015-11-17
// Author       : chenjin
// Function     : 读半个Word
// ParameIn     : 无
// ParameOut    : 无
// Return       : 无
// commments    :
// Modification :
//**************************************************************************************************

void FLASH_If_Read(uint32_t dwAddr, uint16_t *pBuffer, uint16_t wLen)
{
	uint16_t i;

	for (i = 0; i < wLen; i++)
	{
		pBuffer[i] = FLASH_If_ReadHalfWord(dwAddr);//读2个字节
		dwAddr += 2;
	}
}
