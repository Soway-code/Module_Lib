#include "gd32f10x.h" 
#include <stdio.h>
#include "Flash_app.h"

#define BANK0_WRITE_START_ADDR  ((uint32_t)0x08010000)

uint8_t buf1;
uint8_t buf2[30]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
				0x19,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x30};
uint16_t buf3=0;
uint32_t buf4=0;
/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
	fmc_unlock();
    ob_unlock();
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	fmc_page_erase(BANK0_WRITE_START_ADDR);
	
	//读写一个字节
	Flash_Write_OneByte(BANK0_WRITE_START_ADDR, 0x55);
	buf1=Flash_Read_OneByte(BANK0_WRITE_START_ADDR);
	
	//读写二个字节
	Flash_Write_twoByte(BANK0_WRITE_START_ADDR + 0x1, 0x5AA5);
	buf3 = Flash_Read_twoByte(BANK0_WRITE_START_ADDR + 0x1);

	//读写四个字节
	Flash_Write_fourByte(BANK0_WRITE_START_ADDR + 0x3, 0X87654321);
	buf4 = Flash_Read_fourByte(BANK0_WRITE_START_ADDR + 0x3);
	
	//读写30个字节
	Flash_Write_MultiBytes(BANK0_WRITE_START_ADDR + 0x7,buf2,30);
	Flash_Read_MultiBytes(BANK0_WRITE_START_ADDR + 0x7,buf2,30);
	
	while(1) 
	{}
   
}
