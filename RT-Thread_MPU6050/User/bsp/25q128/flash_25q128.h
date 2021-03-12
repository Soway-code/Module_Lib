#ifndef __FLASH_25Q128_H
#define __FLASH_25Q128_H			    
#include "sys.h" 
 
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
 

extern u16 SPI_FLASH_TYPE;		//��������ʹ�õ�flashоƬ�ͺ�		

#define	SPI_FLASH_CS PBout(12)  //ѡ��FLASH					 

extern u8 SPI_FLASH_BUF[4096];
//W25X16��д
#define FLASH_ID 0XEF14
//ָ���
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 


#define SPI_Div_2  	0
#define SPI_Div_4  	1
#define SPI_Div_8  	2
#define SPI_Div_16 	3
#define SPI_Div_32	4
#define SPI_Div_64	5
#define SPI_Div_128	6
#define SPI_Div_256	7

void SPI1_Init(void);
void SPI1_SetSpeed(u8 SpeedSet);

//��ʼ��SPI����
void SPIx_Configuration(SPI_TypeDef *SPIx);

//SPI����BRR��Ƶ����
void SPIx_BRR_Div(SPI_TypeDef *SPIx, uint8_t SPI_Div);

//SPI����һ�ֽ�ͨ��
uint8_t SPIx_WRByte(SPI_TypeDef *SPIx, uint8_t Txdata);
u8 SPI1_ReadWriteByte(u8 TxData);

void SPI_Flash_Init(void);
u16  SPI_Flash_ReadID(void);  	    //��ȡFLASH ID
u8	 SPI_Flash_ReadSR(void);        //��ȡ״̬�Ĵ��� 
void SPI_FLASH_Write_SR(u8 sr);  	//д״̬�Ĵ���
void SPI_FLASH_Write_Enable(void);  //дʹ�� 
void SPI_FLASH_Write_Disable(void);	//д����
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //��ȡflash
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//д��flash
void SPI_Flash_Erase_Chip(void);    	  //��Ƭ����
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//��������
void SPI_Flash_Wait_Busy(void);           //�ȴ�����
void SPI_Flash_PowerDown(void);           //�������ģʽ
void SPI_Flash_WAKEUP(void);			  //����
#endif
