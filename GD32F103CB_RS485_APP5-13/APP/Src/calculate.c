#include "calculate.h"

static uint16_t adc_buf[22],buf1;
float ratio[20];					//分段比例
static uint16_t adc_buf1[20],weight_buf[20];	//筛选后的ad值和对应的重量
static 	uint8_t count_buf = 0;				//当前有不重复的组数

uint8_t WeightProcessing_init(const uint16_t *adc_in,const uint16_t *weight_in,const uint8_t count)
{
	uint8_t i=0,j = 0;
	count_buf = 0;
	
	for(;i < count;i++)		//赋值	
	{
		adc_buf[i] = adc_in[i];
	}
	adc_buf[20] = adc_in[20];
	adc_buf[21] = adc_in[21];
	
	for(i=0;i<count;i++)
		for(j=0;j<i;j++)
			if(adc_buf[j]==adc_buf[i])
				adc_buf[i]=0xFAF5;
	
//排除相同的			
	for(i=0;i<count;i++)		
	{
		if((adc_buf[i] != 0xFAF5) && (adc_buf[i] <= 0x0FFF))		//限制0x0fff
		{
			adc_buf1[count_buf++] = adc_buf[i];
		}	
	}
	
//排大小
	for(i=0; i<count_buf; i++)
	{
		for(j=0; j<(count_buf-i-1); j++)
		{
			if(adc_buf1[j] > adc_buf1[j+1])
			{
				buf1 = adc_buf1[j];
				adc_buf1[j] = adc_buf1[j+1];
				adc_buf1[j+1] = buf1;
			}	
		}
	}
	
//查找对应的重量	
	for(i=0; i<count_buf; i++)		
	{
		for(j=0;j<count;j++)
		{
			buf1 = j;
			if(adc_buf1[i] == adc_in[j])
				j = count+1;
		}
		weight_buf[i] = weight_in[buf1];
	}	
	if(adc_buf[20] >= adc_buf[21])			//空值
	{	
		adc_buf[20] = 0;
		adc_buf[21] = 3800;
	}
	
//比例分配
	if(weight_buf[0] == 0)
		ratio[0] = 0;
	else
		ratio[0] =  (float)weight_buf[0] / (float)(adc_buf1[0] - adc_buf[20]) ;	//减去空值
	ratio[1] = (float)(weight_buf[1] - weight_buf[0]) / (float)(adc_buf1[1] - adc_buf1[0]);	
	ratio[2] = (float)(weight_buf[2] - weight_buf[1]) / (float)(adc_buf1[2] - adc_buf1[1]);
	ratio[3] = (float)(weight_buf[3] - weight_buf[2]) / (float)(adc_buf1[3] - adc_buf1[2]);
	ratio[4] = (float)(weight_buf[4] - weight_buf[3]) / (float)(adc_buf1[4] - adc_buf1[3]);
	ratio[5] = (float)(weight_buf[5] - weight_buf[4]) / (float)(adc_buf1[5] - adc_buf1[4]);
	ratio[6] = (float)(weight_buf[6] - weight_buf[5]) / (float)(adc_buf1[6] - adc_buf1[5]);
	ratio[7] = (float)(weight_buf[7] - weight_buf[6]) / (float)(adc_buf1[7] - adc_buf1[6]);
	ratio[8] = (float)(weight_buf[8] - weight_buf[7]) / (float)(adc_buf1[8] - adc_buf1[7]);
	ratio[9] = (float)(weight_buf[9] - weight_buf[8]) / (float)(adc_buf1[9] - adc_buf1[8]);
	ratio[10] = (float)(weight_buf[10] - weight_buf[9]) / (float)(adc_buf1[10] - adc_buf1[9]);
	ratio[11] = (float)(weight_buf[11] - weight_buf[10]) / (float)(adc_buf1[11] - adc_buf1[10]);
	ratio[12] = (float)(weight_buf[12] - weight_buf[11]) / (float)(adc_buf1[12] - adc_buf1[11]);
	ratio[13] = (float)(weight_buf[13] - weight_buf[12]) / (float)(adc_buf1[13] - adc_buf1[12]);
	ratio[14] = (float)(weight_buf[14] - weight_buf[13]) / (float)(adc_buf1[14] - adc_buf1[13]);
	ratio[15] = (float)(weight_buf[15] - weight_buf[14]) / (float)(adc_buf1[15] - adc_buf1[14]);
	ratio[16] = (float)(weight_buf[16] - weight_buf[15]) / (float)(adc_buf1[16] - adc_buf1[15]);
	ratio[17] = (float)(weight_buf[17] - weight_buf[16]) / (float)(adc_buf1[17] - adc_buf1[16]);
	ratio[18] = (float)(weight_buf[18] - weight_buf[17]) / (float)(adc_buf1[18] - adc_buf1[17]);
	ratio[19] = (float)(weight_buf[19] - weight_buf[18]) / (float)(adc_buf1[19] - adc_buf1[18]);
	
	return count_buf;
}

uint16_t WeightProcessing(uint16_t adc_in)
{
	uint16_t weight = 0;
	if(count_buf == 0)							//标定数量为0直接返回0重量
		weight = 0;
	else if(count_buf == 1)
	{
		weight = ((adc_in - adc_buf[20]) * ratio[0]);
	}
	else if(count_buf == 2)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if(adc_in > adc_buf1[1])
			weight = (ratio[1] * (adc_in - adc_buf1[1])) + weight_buf[1]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 3)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if(adc_in > adc_buf1[2])
			weight = (ratio[2] * (adc_in - adc_buf1[2])) + weight_buf[2]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}	
	else if(count_buf == 4)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if(adc_in > adc_buf1[3])
			weight = (ratio[3] * (adc_in - adc_buf1[3])) + weight_buf[3]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 5)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if(adc_in > adc_buf1[4])
			weight = (ratio[4] * (adc_in - adc_buf1[4])) + weight_buf[4]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 6)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if(adc_in > adc_buf1[5])
			weight = (ratio[5] * (adc_in - adc_buf1[5])) + weight_buf[5]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 7)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if(adc_in > adc_buf1[6])
			weight = (ratio[6] * (adc_in - adc_buf1[6])) + weight_buf[6]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 8)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if(adc_in > adc_buf1[7])
			weight = (ratio[7] * (adc_in - adc_buf1[7])) + weight_buf[7]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 9)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if(adc_in > adc_buf1[8])
			weight = (ratio[8] * (adc_in - adc_buf1[8])) + weight_buf[8]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 10)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if(adc_in > adc_buf1[9])
			weight = (ratio[9] * (adc_in - adc_buf1[9])) + weight_buf[9]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 11)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if(adc_in > adc_buf1[10])
			weight = (ratio[10] * (adc_in - adc_buf1[10])) + weight_buf[10]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 12)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if(adc_in > adc_buf1[11])
			weight = (ratio[11] * (adc_in - adc_buf1[11])) + weight_buf[11]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 13)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if((adc_in <= adc_buf1[12]) && (adc_in > adc_buf1[11]))
		{
			weight = (ratio[12] * (adc_in - adc_buf1[11])) + weight_buf[11];
		}
		else if(adc_in > adc_buf1[12])
			weight = (ratio[12] * (adc_in - adc_buf1[12])) + weight_buf[12]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 14)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if((adc_in <= adc_buf1[12]) && (adc_in > adc_buf1[11]))
		{
			weight = (ratio[12] * (adc_in - adc_buf1[11])) + weight_buf[11];
		}
		else if((adc_in <= adc_buf1[13]) && (adc_in > adc_buf1[12]))
		{
			weight = (ratio[13] * (adc_in - adc_buf1[12])) + weight_buf[12];
		}
		else if(adc_in > adc_buf1[13])
			weight = (ratio[13] * (adc_in - adc_buf1[13])) + weight_buf[13]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 15)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if((adc_in <= adc_buf1[12]) && (adc_in > adc_buf1[11]))
		{
			weight = (ratio[12] * (adc_in - adc_buf1[11])) + weight_buf[11];
		}
		else if((adc_in <= adc_buf1[13]) && (adc_in > adc_buf1[12]))
		{
			weight = (ratio[13] * (adc_in - adc_buf1[12])) + weight_buf[12];
		}
		else if((adc_in <= adc_buf1[14]) && (adc_in > adc_buf1[13]))
		{
			weight = (ratio[14] * (adc_in - adc_buf1[13])) + weight_buf[13];
		}
		else if(adc_in > adc_buf1[14])
			weight = (ratio[14] * (adc_in - adc_buf1[14])) + weight_buf[14]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 16)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if((adc_in <= adc_buf1[12]) && (adc_in > adc_buf1[11]))
		{
			weight = (ratio[12] * (adc_in - adc_buf1[11])) + weight_buf[11];
		}
		else if((adc_in <= adc_buf1[13]) && (adc_in > adc_buf1[12]))
		{
			weight = (ratio[13] * (adc_in - adc_buf1[12])) + weight_buf[12];
		}
		else if((adc_in <= adc_buf1[14]) && (adc_in > adc_buf1[13]))
		{
			weight = (ratio[14] * (adc_in - adc_buf1[13])) + weight_buf[13];
		}
		else if((adc_in <= adc_buf1[15]) && (adc_in > adc_buf1[14]))
		{
			weight = (ratio[15] * (adc_in - adc_buf1[14])) + weight_buf[14];
		}
		else if(adc_in > adc_buf1[15])
			weight = (ratio[15] * (adc_in - adc_buf1[15])) + weight_buf[15]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 17)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if((adc_in <= adc_buf1[12]) && (adc_in > adc_buf1[11]))
		{
			weight = (ratio[12] * (adc_in - adc_buf1[11])) + weight_buf[11];
		}
		else if((adc_in <= adc_buf1[13]) && (adc_in > adc_buf1[12]))
		{
			weight = (ratio[13] * (adc_in - adc_buf1[12])) + weight_buf[12];
		}
		else if((adc_in <= adc_buf1[14]) && (adc_in > adc_buf1[13]))
		{
			weight = (ratio[14] * (adc_in - adc_buf1[13])) + weight_buf[13];
		}
		else if((adc_in <= adc_buf1[15]) && (adc_in > adc_buf1[14]))
		{
			weight = (ratio[15] * (adc_in - adc_buf1[14])) + weight_buf[14];
		}
		else if((adc_in <= adc_buf1[16]) && (adc_in > adc_buf1[15]))
		{
			weight = (ratio[16] * (adc_in - adc_buf1[15])) + weight_buf[15];
		}
		else if(adc_in > adc_buf1[16])
			weight = (ratio[16] * (adc_in - adc_buf1[16])) + weight_buf[16]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 18)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if((adc_in <= adc_buf1[12]) && (adc_in > adc_buf1[11]))
		{
			weight = (ratio[12] * (adc_in - adc_buf1[11])) + weight_buf[11];
		}
		else if((adc_in <= adc_buf1[13]) && (adc_in > adc_buf1[12]))
		{
			weight = (ratio[13] * (adc_in - adc_buf1[12])) + weight_buf[12];
		}
		else if((adc_in <= adc_buf1[14]) && (adc_in > adc_buf1[13]))
		{
			weight = (ratio[14] * (adc_in - adc_buf1[13])) + weight_buf[13];
		}
		else if((adc_in <= adc_buf1[15]) && (adc_in > adc_buf1[14]))
		{
			weight = (ratio[15] * (adc_in - adc_buf1[14])) + weight_buf[14];
		}
		else if((adc_in <= adc_buf1[16]) && (adc_in > adc_buf1[15]))
		{
			weight = (ratio[16] * (adc_in - adc_buf1[15])) + weight_buf[15];
		}
		else if((adc_in <= adc_buf1[17]) && (adc_in > adc_buf1[16]))
		{
			weight = (ratio[17] * (adc_in - adc_buf1[16])) + weight_buf[16];
		}
		else if(adc_in > adc_buf1[17])
			weight = (ratio[17] * (adc_in - adc_buf1[17])) + weight_buf[17]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 19)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if((adc_in <= adc_buf1[12]) && (adc_in > adc_buf1[11]))
		{
			weight = (ratio[12] * (adc_in - adc_buf1[11])) + weight_buf[11];
		}
		else if((adc_in <= adc_buf1[13]) && (adc_in > adc_buf1[12]))
		{
			weight = (ratio[13] * (adc_in - adc_buf1[12])) + weight_buf[12];
		}
		else if((adc_in <= adc_buf1[14]) && (adc_in > adc_buf1[13]))
		{
			weight = (ratio[14] * (adc_in - adc_buf1[13])) + weight_buf[13];
		}
		else if((adc_in <= adc_buf1[15]) && (adc_in > adc_buf1[14]))
		{
			weight = (ratio[15] * (adc_in - adc_buf1[14])) + weight_buf[14];
		}
		else if((adc_in <= adc_buf1[16]) && (adc_in > adc_buf1[15]))
		{
			weight = (ratio[16] * (adc_in - adc_buf1[15])) + weight_buf[15];
		}
		else if((adc_in <= adc_buf1[17]) && (adc_in > adc_buf1[16]))
		{
			weight = (ratio[17] * (adc_in - adc_buf1[16])) + weight_buf[16];
		}
		else if((adc_in <= adc_buf1[18]) && (adc_in > adc_buf1[17]))
		{
			weight = (ratio[18] * (adc_in - adc_buf1[17])) + weight_buf[17];
		}
		else if(adc_in > adc_buf1[18])
			weight = (ratio[18] * (adc_in - adc_buf1[18])) + weight_buf[18]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	else if(count_buf == 20)
	{
		if((adc_in <= adc_buf1[1]) && (adc_in > adc_buf1[0]))
		{
			weight = (ratio[1] * (adc_in - adc_buf1[0])) + weight_buf[0];
		}
		else if((adc_in <= adc_buf1[2]) && (adc_in > adc_buf1[1]))
		{
			weight = (ratio[2] * (adc_in - adc_buf1[1])) + weight_buf[1];
		}
		else if((adc_in <= adc_buf1[3]) && (adc_in > adc_buf1[2]))
		{
			weight = (ratio[3] * (adc_in - adc_buf1[2])) + weight_buf[2];
		}
		else if((adc_in <= adc_buf1[4]) && (adc_in > adc_buf1[3]))
		{
			weight = (ratio[4] * (adc_in - adc_buf1[3])) + weight_buf[3];
		}
		else if((adc_in <= adc_buf1[5]) && (adc_in > adc_buf1[4]))
		{
			weight = (ratio[5] * (adc_in - adc_buf1[4])) + weight_buf[4];
		}
		else if((adc_in <= adc_buf1[6]) && (adc_in > adc_buf1[5]))
		{
			weight = (ratio[6] * (adc_in - adc_buf1[5])) + weight_buf[5];
		}
		else if((adc_in <= adc_buf1[7]) && (adc_in > adc_buf1[6]))
		{
			weight = (ratio[7] * (adc_in - adc_buf1[6])) + weight_buf[6];
		}
		else if((adc_in <= adc_buf1[8]) && (adc_in > adc_buf1[7]))
		{
			weight = (ratio[8] * (adc_in - adc_buf1[7])) + weight_buf[7];
		}
		else if((adc_in <= adc_buf1[9]) && (adc_in > adc_buf1[8]))
		{
			weight = (ratio[9] * (adc_in - adc_buf1[8])) + weight_buf[8];
		}
		else if((adc_in <= adc_buf1[10]) && (adc_in > adc_buf1[9]))
		{
			weight = (ratio[10] * (adc_in - adc_buf1[9])) + weight_buf[9];
		}
		else if((adc_in <= adc_buf1[11]) && (adc_in > adc_buf1[10]))
		{
			weight = (ratio[11] * (adc_in - adc_buf1[10])) + weight_buf[10];
		}
		else if((adc_in <= adc_buf1[12]) && (adc_in > adc_buf1[11]))
		{
			weight = (ratio[12] * (adc_in - adc_buf1[11])) + weight_buf[11];
		}
		else if((adc_in <= adc_buf1[13]) && (adc_in > adc_buf1[12]))
		{
			weight = (ratio[13] * (adc_in - adc_buf1[12])) + weight_buf[12];
		}
		else if((adc_in <= adc_buf1[14]) && (adc_in > adc_buf1[13]))
		{
			weight = (ratio[14] * (adc_in - adc_buf1[13])) + weight_buf[13];
		}
		else if((adc_in <= adc_buf1[15]) && (adc_in > adc_buf1[14]))
		{
			weight = (ratio[15] * (adc_in - adc_buf1[14])) + weight_buf[14];
		}
		else if((adc_in <= adc_buf1[16]) && (adc_in > adc_buf1[15]))
		{
			weight = (ratio[16] * (adc_in - adc_buf1[15])) + weight_buf[15];
		}
		else if((adc_in <= adc_buf1[17]) && (adc_in > adc_buf1[16]))
		{
			weight = (ratio[17] * (adc_in - adc_buf1[16])) + weight_buf[16];
		}
		else if((adc_in <= adc_buf1[18]) && (adc_in > adc_buf1[17]))
		{
			weight = (ratio[18] * (adc_in - adc_buf1[17])) + weight_buf[17];
		}
		else if((adc_in <= adc_buf1[19]) && (adc_in > adc_buf1[18]))
		{
			weight = (ratio[19] * (adc_in - adc_buf1[18])) + weight_buf[18];
		}
		else if(adc_in > adc_buf1[19])
			weight = (ratio[19] * (adc_in - adc_buf1[19])) + weight_buf[19]; 
		else if(adc_in <= adc_buf1[0])
			weight = ratio[0] * adc_in;
	}
	return weight;
}