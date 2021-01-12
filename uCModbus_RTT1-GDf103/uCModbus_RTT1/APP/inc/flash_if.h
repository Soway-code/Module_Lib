//**************************************************************************************************
//  Copyright (C), 2013-2023, The Chinese University of Hong Kong
//  FileName:      flash_if.h
//  Author:        chenjin      
//  Version:       1.0.0.0        
//  Date:          2015/11/17
//  Description:   FLASH头文件 
//  Function List: 
//  History:         // 历史修改记录
//  	author:       
//      Date:         
//      version:      
//      desc:         
//**************************************************************************************************


// Define to prevent recursive inclusion -------------------------------------

#ifndef __FLASH_IF_H
#define __FLASH_IF_H

#define USER_FLASH_LAST_PAGE_ADDRESS  0x08004000
#define USER_FLASH_END_ADDRESS        0x08010000  //64 KBytes 
#define FLASH_PAGE_SIZE               0x400       //1 Kbytes 

#define APP_UPDATE_FLAG_ADDRESS       0x08003800

// define the address from where user application will be loaded,
//   the application address should be a start sector address 
//#define APPLICATION_ADDRESS     (uint32_t)0x08004000

// Exported macro ------------------------------------------------------------
// Exported functions ------------------------------------------------------- 

void FLASH_If_Init(void);
uint32_t FLASH_If_Erase(uint32_t StartSector);
//uint32_t FLASH_If_Write(__IO uint32_t* FlashAddress, uint32_t* Data, uint16_t DataLength);
uint32_t FLASH_If_Write(uint32_t *dwFlashAddress, uint16_t *pWData, uint16_t wDataLength);
void FLASH_If_Read(uint32_t dwAddr, uint16_t *pBuffer, uint16_t wLen);

#endif  // __FLASH_IF_H 


