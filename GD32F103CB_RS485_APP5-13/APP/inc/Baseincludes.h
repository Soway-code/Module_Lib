/*******************************************************************************
                This is the base header file
                    Edited by xieyuyin
*******************************************************************************/
#ifndef _BASEINCLUDES_H
#define _BASEINCLUDES_H

//#include    <stdbool.h>
//#include    <string.h>
//#include    "stm32f0xx.h"
#include  "includes.h"



//Common functions declare
bool COMMON_Bits8Convert2Bits16( uint16_t *pdest, uint8_t *psrc, uint8_t mode );
bool COMMON_Bits8Convert2Bits32( uint32_t *pdest, uint8_t *psrc, uint8_t mode );
bool COMMON_Bits16Convert2Bits8( uint8_t *pdest, uint16_t src, uint8_t mode );
bool COMMON_Bits32Convert2Bits8( uint8_t *pdest, uint32_t src, uint8_t mode );

bool COMMON_ConverAscii2CommonMode( uint8_t* psrc, uint16_t datlen);
bool COMMON_Convert1Byteto4Bits(uint8_t* pdest, uint8_t src);
uint8_t COMMON_GeneratorLRCCheckCode(uint8_t* psrc, uint16_t len);
bool COMMON_ConvertHex2Ascii( uint8_t* psrc, uint16_t len );
void COMMON_DelayXus( uint32_t dly );

//Generator crc check code
bool COMMON_Generatoruint16_tbitsCRCCheckCode( uint16_t* crcval, uint8_t* psrc, uint32_t srclen);

#endif

