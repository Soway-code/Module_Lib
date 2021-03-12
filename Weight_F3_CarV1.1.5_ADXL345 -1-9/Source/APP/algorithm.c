//#include "bsp.h"
#include "algorithm.h"

void SortArrayExtreme(u16 Array[], const u32 ArraySize,
                      const u32 SortHeadSize, const u32 SortTailSize)
{
    u32 i, j;
    u16 temp;

    for (i = 0; i < SortTailSize; i++)
    {
        for (j = 0; j < ArraySize - i - 1; j++)
        {
            if (Array[j] > Array[j+1])
            {
                temp = Array[j];
                Array[j] = Array[j+1];
                Array[j+1] = temp;
            }
        }
    }

    for (i = 0; i < SortHeadSize; i++)
    {
        for (j = ArraySize - SortTailSize - 1 ; j > i; j--)
        {
            if (Array[j - 1] > Array[j])
            {
                temp = Array[j - 1];
                Array[j - 1] = Array[j];
                Array[j] = temp;
            }
        }
    }
}

u32 GetAverage(u16 Array[], const u32 ArraySize,
               const u32 DelHeadSize, const u32 DelTailSize)
{
    u32 sum = 0;

    if ((DelHeadSize + DelTailSize) >= ArraySize)
    {
        return 0;
    }

    for (long i = DelHeadSize; i < ArraySize - DelTailSize; i++)
    {
        sum += Array[i];
    }

    return(sum / (ArraySize - DelHeadSize - DelTailSize));
}


u32 GetDelExtremeAndAverage(u16 Array[], const u32 ArraySize,
                            const u32 SortHeadSize, const u32 SortTailSize)
{
    SortArrayExtreme(Array, ArraySize, SortHeadSize, SortTailSize);
    return(GetAverage(Array, ArraySize, SortHeadSize, SortTailSize));
}


u32 HexArrayToHex(u8 *pData)//16进制字节数组转32位无符号数
{
    u32 Data;
    Data = pData[0];
    Data <<= 8;
    Data += pData[1];
    Data <<= 8;
    Data += pData[2];
    Data <<= 8;
    Data += pData[3];

    return *(u32*)&Data;
}

float HexToFloat(u8 *pData)//16进制 转浮点数
{
    u32 Data;
    Data = pData[0];
    Data <<= 8;
    Data += pData[1];
    Data <<= 8;
    Data += pData[2];
    Data <<= 8;
    Data += pData[3];

    return *(float*)&Data;
}

u32 HexToUlong(u8 *pData)//16进制字节数组转无符号长整形  32位数
{
    u32 Data;
    Data = pData[0];
    Data <<= 8;
    Data += pData[1];
    Data <<= 8;
    Data += pData[2];
    Data <<= 8;
    Data += pData[3];

    return Data;
}


void floatToHexArray(float fSource, u8 *pu8Temp, long num)//浮点数 转 16进制 
{
    u32 u32Source = *(u32 *)&fSource;
    for(long i = 0; i < num; i++)
    {
        pu8Temp[i] = (u32Source >> (8 * (3 - i))) & 0xff;
    }
}

void UshortToArray(u16 uSource, u8 *pu8Temp)// 16位短整形数 转 16进制数组
{
    u16 data = uSource;
    *pu8Temp = ((u8*)&data)[1];
    *(pu8Temp + 1) = ((u8*)&data)[0];
}


u16 ArrayToUshort(u8 *pu8Temp)//16进制数组 转 16位短整形数
{
    u16 data = 0;
    ((u8*)&data)[0] = *(pu8Temp + 1);
    ((u8*)&data)[1] = *pu8Temp;
    return data;
}


