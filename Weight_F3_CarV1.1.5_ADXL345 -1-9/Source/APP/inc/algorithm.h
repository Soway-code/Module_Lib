#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include "stm32f0xx.h"

void ArrayMerge(u8 A1[], u8 A2[], u8 A[], u8 Size1, u8 Size2);
void SortArrayExtreme(u16 Array[], const u32 ArraySize,
                      const u32 SortHeadSize, const u32 SortTailSize);
u32 GetDelExtremeAndAverage(u16 Array[], const u32 ArraySize,
                            const u32 SortHeadSize, const u32 SortTailSize);

u32 HexArrayToHex(u8 *pData);
float HexToFloat(u8 *pData);
void floatToHexArray(float fSource, u8 *pu8Temp, long num);
u32 HexToUlong(u8 *pData);

void UshortToArray(u16 uSource, u8 *pu8Temp);
u16 ArrayToUshort(u8 *pu8Temp);

#endif
