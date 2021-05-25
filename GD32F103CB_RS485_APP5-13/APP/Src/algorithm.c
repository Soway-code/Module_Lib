//#include "bsp.h"
#include "algorithm.h"

void SortArrayExtreme(int32_t Array[], const uint32_t ArraySize,
                      const uint32_t SortHeadSize, const uint32_t SortTailSize)
{
    uint32_t i, j;
    int32_t temp;

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

uint32_t GetAverage(int32_t Array[], const uint32_t ArraySize,
               const uint32_t DelHeadSize, const uint32_t DelTailSize)
{
    int64_t sum = 0;

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


uint32_t GetDelExtremeAndAverage(int32_t Array[], const uint32_t ArraySize,
                            const uint32_t SortHeadSize, const uint32_t SortTailSize)
{
    SortArrayExtreme(Array, ArraySize, SortHeadSize, SortTailSize);
    return(GetAverage(Array, ArraySize, SortHeadSize, SortTailSize));
}




//获得数组最大或最小值元素
uint32_t Get_Min_Max(uint32_t *dBuf, uint8_t len, uint8_t mode)
{
    uint8_t i;
    uint32_t temp;
       
    if(mode)                                                                    //返回最大值
    {
        for(i = 0;i < len - 1;i++)                                              //找到最大的值
        {
            if(dBuf[i] > dBuf[i + 1])
            {
                temp = dBuf[i];
                dBuf[i] = dBuf[i + 1];
                dBuf[i + 1] = temp;
            }
        }
        return dBuf[len - 1];
    }
    else
    {
        for(i = len - 1;i > 0 ;i--)                                             //找到数组最小的值
        {
            if(dBuf[i - 1] > dBuf[i])
            {
                temp = dBuf[i - 1];
                dBuf[i - 1] = dBuf[i];
                dBuf[i] = temp;
            }
        }
        return dBuf[0];                                                         //返回最小值
    }
}




uint32_t ArrToHex(uint8_t *pData)
{
    uint32_t Data;
    Data = pData[0];
    Data <<= 8;
    Data += pData[1];
    Data <<= 8;
    Data += pData[2];
    Data <<= 8;
    Data += pData[3];

    return *(uint32_t*)&Data;
}

float HexToFloat(uint8_t *pData)
{
    uint32_t Data;
    Data = pData[0];
    Data <<= 8;
    Data += pData[1];
    Data <<= 8;
    Data += pData[2];
    Data <<= 8;
    Data += pData[3];

    return *(float*)&Data;
}

uint32_t HexToUlong(uint8_t *pData)
{
    uint32_t Data;
    Data = pData[0];
    Data <<= 8;
    Data += pData[1];
    Data <<= 8;
    Data += pData[2];
    Data <<= 8;
    Data += pData[3];

    return Data;
}


void floatToHexArray(float fSource, uint8_t *puint8_tTemp, long num)
{
    uint32_t uint32_tSource = *(uint32_t *)&fSource;
    for(long i = 0; i < num; i++)
    {
        puint8_tTemp[i] = (uint32_tSource >> (8 * (3 - i))) & 0xff;
    }
}



int sum(int *buf, uint8_t len)
{
    uint8_t i;
    int sum = 0;
    
    for(i = 0;i < len;i++)
    {
        sum += *(buf + i);
    }
    return sum;
}


uint32_t square_sum(int *bufx, uint8_t len)
{
    uint8_t i;
    uint32_t sum = 0;
    
    for(i = 0;i < len;i++)
    {
        sum += (*(bufx + i)) * (*(bufx + i));
    }
    return sum;    
}


int multi_sum(int *bufx, int *bufy, uint8_t len)
{
    uint8_t i;
    int sum = 0;
    
    for(i = 0;i < len;i++)
    {
        sum += (*(bufx + i)) * (*(bufy + i));
    }
    return sum; 
}


int get_divisor(int *bufx, uint8_t len)
{
    int sumx;
    int divisor;
    uint32_t square_sumx;

    sumx = sum(bufx, len);
    square_sumx = square_sum(bufx, len);
    divisor = sumx * sumx - square_sumx * len;
    return divisor;
}


int get_slop(int *bufx, int *bufy, uint8_t len)
{
    int k;
    int divisor;
    int sdivisor;
 
    int sumx;
    int sumy;
    int multi_sumxy;
    
    sumx = sum(bufx, len);
    sumy = sum(bufy, len);
    multi_sumxy = multi_sum(bufx, bufy, len);
    sdivisor = sumx * sumy - multi_sumxy * len;
    divisor = get_divisor(bufx, len);
    if(divisor != 0)                         
    {  
        k = (int)(sdivisor *100.0f / divisor);
    }

    else
    {
        k = 0;
    }
    return k;
}

