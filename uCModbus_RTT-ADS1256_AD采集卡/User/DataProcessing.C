#include "DataProcessing.h"

//每个通道标定的电压值
uint32_t *ch0;
uint32_t *ch1;
uint32_t *ch2;
uint32_t *ch3;
uint32_t *ch4;
uint32_t *ch5;
uint32_t *ch6;
uint32_t *ch7;

//每一段对应的0.00001伏电压值
float ch0_f[10];
float ch1_f[10];
float ch2_f[10];
float ch3_f[10];
float ch4_f[10];
float ch5_f[10];
float ch6_f[10];
float ch7_f[10];

/**@brief       标定的参数初始
* @param[in]    *CalibrationValue : 数数组方式传入8*11个点的电压值;uint32_t CalibrationValue[80]={xxxx,xxxx};
* @return       函数执行结果
* - 1(成功)
* - 0(失败)
* @note  \n
*/
uint8_t parameter_init(uint32_t *CalibrationValue)
{
	float Cardinality = 500000.0;
	uint8_t i = 2;
	ch0 = &CalibrationValue[0];
	ch1 = &CalibrationValue[11];
	ch2 = &CalibrationValue[22];
	ch3 = &CalibrationValue[33];
	ch4 = &CalibrationValue[44];
	ch5 = &CalibrationValue[55];
	ch6 = &CalibrationValue[66];
	ch7 = &CalibrationValue[77];

////0点赋值	
//	ch0_f[0] = (Cardinality / (float)ch0[0]);
//	ch1_f[0] = ch1[0];
//	ch2_f[0] = ch2[0];
//	ch3_f[0] = ch3[0];
//	ch4_f[0] = ch4[0];
//	ch5_f[0] = ch5[0];
//	ch6_f[0] = ch6[0];
//	ch7_f[0] = ch7[0];
	
////1点赋值
//	ch0_f[1] = ((float)ch0[1] / Cardinality);
//	ch1_f[1] = ((float)ch1[1] / Cardinality);
//	ch2_f[1] = ((float)ch2[1] / Cardinality);
//	ch3_f[1] = ((float)ch3[1] / Cardinality);
//	ch4_f[1] = ((float)ch4[1] / Cardinality);
//	ch5_f[1] = ((float)ch5[1] / Cardinality);
//	ch6_f[1] = ((float)ch6[1] / Cardinality);
//	ch7_f[1] = ((float)ch7[1] / Cardinality);
	
	for(i = 1;i <= 10;)
	{
		ch0_f[i-1] = (Cardinality/((float)(ch0[i] - ch0[i-1])));
		ch1_f[i-1] = (Cardinality/((float)(ch1[i] - ch1[i-1])));
		ch2_f[i-1] = (Cardinality/((float)(ch2[i] - ch2[i-1])));
		ch3_f[i-1] = (Cardinality/((float)(ch3[i] - ch3[i-1])));
		ch4_f[i-1] = (Cardinality/((float)(ch4[i] - ch4[i-1])));
		ch5_f[i-1] = (Cardinality/((float)(ch5[i] - ch5[i-1])));
		ch6_f[i-1] = (Cardinality/((float)(ch6[i] - ch6[i-1])));
		ch7_f[i-1] = (Cardinality/((float)(ch7[i] - ch7[i-1])));
		i++;
	}

	if((ch0 != NULL) && (ch7 != NULL))
		return 1;
	else return 0;
}

//通道0滤波
uint32_t CH0_filter(uint32_t voltage)
{
	uint8_t i=0,j=0;
	uint32_t buf1 = 0,bufz[7]={0};
	static uint8_t signs = 0;
	static uint32_t vol[8]={0},buf2=0;
	if(signs >= 8)
		signs = 0;
	vol[signs] = voltage;
	j = signs++;
	for(i = 0;i<7;i++)
	{
		bufz[i] = vol[j];
		if(j <= 0)
			j = 8;
		j--;
	}
	for(i=0; i<7; i++)
	{
		for(j=0; j<7-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}
	buf2 = 0;
	buf2+=bufz[2]+bufz[3]+bufz[4];
	buf2 = (uint32_t)(buf2 /3.0);
	return buf2;
}


//通道1滤波
uint32_t CH1_filter(uint32_t voltage)
{
	uint8_t i=0,j=0;
	uint32_t buf1 = 0,bufz[7]={0};
	static uint8_t signs = 0;
	static uint32_t vol[8]={0},buf2=0;
	if(signs >= 8)
		signs = 0;
	vol[signs] = voltage;
	j = signs++;
	for(i = 0;i<7;i++)
	{
		bufz[i] = vol[j];
		if(j <= 0)
			j = 8;
		j--;
	}
	for(i=0; i<7; i++)
	{
		for(j=0; j<7-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}
	buf2 = 0;
	buf2+=bufz[2]+bufz[3]+bufz[4];
	buf2 = (uint32_t)(buf2 /3.0);
	return buf2;
}


//通道2滤波
uint32_t CH2_filter(uint32_t voltage)
{
	uint8_t i=0,j=0;
	uint32_t buf1 = 0,bufz[7]={0};
	static uint8_t signs = 0;
	static uint32_t vol[8]={0},buf2=0;
	if(signs >= 8)
		signs = 0;
	vol[signs] = voltage;
	j = signs++;
	for(i = 0;i<7;i++)
	{
		bufz[i] = vol[j];
		if(j <= 0)
			j = 8;
		j--;
	}
	for(i=0; i<7; i++)
	{
		for(j=0; j<7-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}
	buf2 = 0;
	buf2+=bufz[2]+bufz[3]+bufz[4];
	buf2 = (uint32_t)(buf2 /3.0);
	return buf2;
}

//通道3滤波
uint32_t CH3_filter(uint32_t voltage)
{
	uint8_t i=0,j=0;
	uint32_t buf1 = 0,bufz[7]={0};
	static uint8_t signs = 0;
	static uint32_t vol[8]={0},buf2=0;
	if(signs >= 8)
		signs = 0;
	vol[signs] = voltage;
	j = signs++;
	for(i = 0;i<7;i++)
	{
		bufz[i] = vol[j];
		if(j <= 0)
			j = 8;
		j--;
	}
	for(i=0; i<7; i++)
	{
		for(j=0; j<7-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}
	buf2 = 0;
	buf2+=bufz[2]+bufz[3]+bufz[4];
	buf2 = (uint32_t)(buf2 /3.0);
	return buf2;
}

//通道4滤波
uint32_t CH4_filter(uint32_t voltage)
{
	uint8_t i=0,j=0;
	uint32_t buf1 = 0,bufz[7]={0};
	static uint8_t signs = 0;
	static uint32_t vol[8]={0},buf2=0;
	if(signs >= 8)
		signs = 0;
	vol[signs] = voltage;
	j = signs++;
	for(i = 0;i<7;i++)
	{
		bufz[i] = vol[j];
		if(j <= 0)
			j = 8;
		j--;
	}
	for(i=0; i<7; i++)
	{
		for(j=0; j<7-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}
	buf2 = 0;
	buf2+=bufz[2]+bufz[3]+bufz[4];
	buf2 = (uint32_t)(buf2 /3.0);
	return buf2;
}

//通道5滤波
uint32_t CH5_filter(uint32_t voltage)
{
	uint8_t i=0,j=0;
	uint32_t buf1 = 0,bufz[7]={0};
	static uint8_t signs = 0;
	static uint32_t vol[8]={0},buf2=0;
	if(signs >= 8)
		signs = 0;
	vol[signs] = voltage;
	j = signs++;
	for(i = 0;i<7;i++)
	{
		bufz[i] = vol[j];
		if(j <= 0)
			j = 8;
		j--;
	}
	for(i=0; i<7; i++)
	{
		for(j=0; j<7-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}
	buf2 = 0;
	buf2+=bufz[2]+bufz[3]+bufz[4];
	buf2 = (uint32_t)(buf2 /3.0);
	return buf2;
}

//通道6滤波
uint32_t CH6_filter(uint32_t voltage)
{
	uint8_t i=0,j=0;
	uint32_t buf1 = 0,bufz[7]={0};
	static uint8_t signs = 0;
	static uint32_t vol[8]={0},buf2=0;
	if(signs >= 8)
		signs = 0;
	vol[signs] = voltage;
	j = signs++;
	for(i = 0;i<7;i++)
	{
		bufz[i] = vol[j];
		if(j <= 0)
			j = 8;
		j--;
	}
	for(i=0; i<7; i++)
	{
		for(j=0; j<7-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}
	buf2 = 0;
	buf2+=bufz[2]+bufz[3]+bufz[4];
	buf2 = (uint32_t)(buf2 /3.0);
	return buf2;
}

//通道7滤波
uint32_t CH7_filter(uint32_t voltage)
{
	uint8_t i=0,j=0;
	uint32_t buf1 = 0,bufz[7]={0};
	static uint8_t signs = 0;
	static uint32_t vol[8]={0},buf2=0;
	if(signs >= 8)
		signs = 0;
	vol[signs] = voltage;
	j = signs++;
	for(i = 0;i<7;i++)
	{
		bufz[i] = vol[j];
		if(j <= 0)
			j = 8;
		j--;
	}
	for(i=0; i<7; i++)
	{
		for(j=0; j<7-i; j++)
		{
			if(bufz[j] > bufz[j+1])
			{
				buf1 = bufz[j];
				bufz[j] = bufz[j+1];
				bufz[j+1] = buf1;
			}	
		}
	}
	buf2 = 0;
	buf2+=bufz[2]+bufz[3]+bufz[4];
	buf2 = (uint32_t)(buf2 /3.0);
	return buf2;
}

/**@brief       通道0处理
* @param[in]    V_IN: ads1256芯片读出的电压值
* @param[out]	V_out: 滤波后面的当前电压
* @return       分段处理后的电压值
* @note  \n
*/

uint32_t Channel_0(uint32_t V_IN,uint32_t *V_OUT)
{
	uint32_t Volutag = 0;
	Volutag = CH0_filter(V_IN);
	*V_OUT = Volutag;
	if(Volutag <= ch0[0])		//0V
		return 0;
	else if((Volutag > ch0[0]) && (Volutag <= ch0[1]))	//0-0.5v
		return (ch0_f[0] * Volutag);
	else if((Volutag > ch0[1]) && (Volutag <= ch0[2]))	//0.5-1v
		return ((ch0_f[1] * (Volutag - ch0[1])) + 500000);
	else if((Volutag > ch0[2]) && (Volutag <= ch0[3]))	//1-1.5v
		return ((ch0_f[2] * (Volutag - ch0[2])) + 1000000);
	else if((Volutag > ch0[3]) && (Volutag <= ch0[4]))	//1.5-2v
		return ((ch0_f[3] * (Volutag - ch0[3])) + 1500000);	
	else if((Volutag > ch0[4]) && (Volutag <= ch0[5]))	//2-2.5v
		return ((ch0_f[4] * (Volutag - ch0[4])) + 2000000);	
	else if((Volutag > ch0[5]) && (Volutag <= ch0[6]))	//2.5-3v
		return ((ch0_f[5] * (Volutag - ch0[5])) + 2500000);
	else if((Volutag > ch0[6]) && (Volutag <= ch0[7]))	//3-3.5v
		return ((ch0_f[6] * (Volutag - ch0[6])) + 3000000);
	else if((Volutag > ch0[7]) && (Volutag <= ch0[8]))	//3.5-4v
		return ((ch0_f[7] * (Volutag - ch0[7])) + 3500000);
	else if((Volutag > ch0[8]) && (Volutag <= ch0[9]))	//4-4.5v
		return ((ch0_f[8] * (Volutag - ch0[8])) + 4000000);
	else if(Volutag > ch0[9])	//4.5-5v
		return ((ch0_f[9] * (Volutag - ch0[9])) + 4500000);
	return 0;
}

/**@brief       通道1处理
* @param[in]    V_IN: ads1256芯片读出的电压值
* @param[out]	V_out: 滤波后面的当前电压
* @return       分段处理后的电压值
* @note  \n
*/

uint32_t Channel_1(uint32_t V_IN,uint32_t *V_OUT)
{
	uint32_t Volutag = 0;
	Volutag = CH1_filter(V_IN);
	*V_OUT = Volutag;
	if(Volutag <= ch1[0])		//0V
		return 0;
	else if((Volutag > ch1[0]) && (Volutag <= ch1[1]))	//0-0.5v
		return (ch1_f[0] * Volutag);
	else if((Volutag > ch1[1]) && (Volutag <= ch1[2]))	//0.5-1v
		return ((ch1_f[1] * (Volutag - ch1[1])) + 500000);
	else if((Volutag > ch1[2]) && (Volutag <= ch1[3]))	//1-1.5v
		return ((ch1_f[2] * (Volutag - ch1[2])) + 1000000);
	else if((Volutag > ch1[3]) && (Volutag <= ch1[4]))	//1.5-2v
		return ((ch1_f[3] * (Volutag - ch1[3])) + 1500000);	
	else if((Volutag > ch1[4]) && (Volutag <= ch1[5]))	//2-2.5v
		return ((ch1_f[4] * (Volutag - ch1[4])) + 2000000);	
	else if((Volutag > ch1[5]) && (Volutag <= ch1[6]))	//2.5-3v
		return ((ch1_f[5] * (Volutag - ch1[5])) + 2500000);
	else if((Volutag > ch1[6]) && (Volutag <= ch1[7]))	//3-3.5v
		return ((ch1_f[6] * (Volutag - ch1[6])) + 3000000);
	else if((Volutag > ch1[7]) && (Volutag <= ch1[8]))	//3.5-4v
		return ((ch1_f[7] * (Volutag - ch1[7])) + 3500000);
	else if((Volutag > ch1[8]) && (Volutag <= ch1[9]))	//4-4.5v
		return ((ch1_f[8] * (Volutag - ch1[8])) + 4000000);
	else if(Volutag > ch1[9])	//4.5-5v
		return ((ch1_f[9] * (Volutag - ch1[9])) + 4500000);
	return 0;
}

/**@brief       通道2处理
* @param[in]    V_IN: ads1256芯片读出的电压值
* @param[out]	V_out: 滤波后面的当前电压
* @return       分段处理后的电压值
* @note  \n
*/

uint32_t Channel_2(uint32_t V_IN,uint32_t *V_OUT)
{
	uint32_t Volutag = 0;
	Volutag = CH2_filter(V_IN);
	*V_OUT = Volutag;
	if(Volutag <= ch2[0])		//0V
		return 0;
	else if((Volutag > ch2[0]) && (Volutag <= ch2[1]))	//0-0.5v
		return (ch2_f[0] * Volutag);
	else if((Volutag > ch2[1]) && (Volutag <= ch2[2]))	//0.5-1v
		return ((ch2_f[1] * (Volutag - ch2[1])) + 500000);
	else if((Volutag > ch2[2]) && (Volutag <= ch2[3]))	//1-1.5v
		return ((ch2_f[2] * (Volutag - ch2[2])) + 1000000);
	else if((Volutag > ch2[3]) && (Volutag <= ch2[4]))	//1.5-2v
		return ((ch2_f[3] * (Volutag - ch2[3])) + 1500000);	
	else if((Volutag > ch2[4]) && (Volutag <= ch2[5]))	//2-2.5v
		return ((ch2_f[4] * (Volutag - ch2[4])) + 2000000);	
	else if((Volutag > ch2[5]) && (Volutag <= ch2[6]))	//2.5-3v
		return ((ch2_f[5] * (Volutag - ch2[5])) + 2500000);
	else if((Volutag > ch2[6]) && (Volutag <= ch2[7]))	//3-3.5v
		return ((ch2_f[6] * (Volutag - ch2[6])) + 3000000);
	else if((Volutag > ch2[7]) && (Volutag <= ch2[8]))	//3.5-4v
		return ((ch2_f[7] * (Volutag - ch2[7])) + 3500000);
	else if((Volutag > ch2[8]) && (Volutag <= ch2[9]))	//4-4.5v
		return ((ch2_f[8] * (Volutag - ch2[8])) + 4000000);
	else if(Volutag > ch2[9])	//4.5-5v
		return ((ch2_f[9] * (Volutag - ch2[9])) + 4500000);
	return 0;
}

/**@brief       通道3处理
* @param[in]    V_IN: ads1256芯片读出的电压值
* @param[out]	V_out: 滤波后面的当前电压
* @return       分段处理后的电压值
* @note  \n
*/

uint32_t Channel_3(uint32_t V_IN,uint32_t *V_OUT)
{
	uint32_t Volutag = 0;
	Volutag = CH3_filter(V_IN);
	*V_OUT = Volutag;
	if(Volutag <= ch3[0])		//0V
		return 0;
	else if((Volutag > ch3[0]) && (Volutag <= ch3[1]))	//0-0.5v
		return (ch3_f[0] * Volutag);
	else if((Volutag > ch3[1]) && (Volutag <= ch3[2]))	//0.5-1v
		return ((ch3_f[1] * (Volutag - ch3[1])) + 500000);
	else if((Volutag > ch3[2]) && (Volutag <= ch3[3]))	//1-1.5v
		return ((ch3_f[2] * (Volutag - ch3[2])) + 1000000);
	else if((Volutag > ch3[3]) && (Volutag <= ch3[4]))	//1.5-2v
		return ((ch3_f[3] * (Volutag - ch3[3])) + 1500000);	
	else if((Volutag > ch3[4]) && (Volutag <= ch3[5]))	//2-2.5v
		return ((ch3_f[4] * (Volutag - ch3[4])) + 2000000);	
	else if((Volutag > ch3[5]) && (Volutag <= ch3[6]))	//2.5-3v
		return ((ch3_f[5] * (Volutag - ch3[5])) + 2500000);
	else if((Volutag > ch3[6]) && (Volutag <= ch3[7]))	//3-3.5v
		return ((ch3_f[6] * (Volutag - ch3[6])) + 3000000);
	else if((Volutag > ch3[7]) && (Volutag <= ch3[8]))	//3.5-4v
		return ((ch3_f[7] * (Volutag - ch3[7])) + 3500000);
	else if((Volutag > ch3[8]) && (Volutag <= ch3[9]))	//4-4.5v
		return ((ch3_f[8] * (Volutag - ch3[8])) + 4000000);
	else if(Volutag > ch3[9])	//4.5-5v
		return ((ch3_f[9] * (Volutag - ch3[9])) + 4500000);
	return 0;
}

/**@brief       通道4处理
* @param[in]    V_IN: ads1256芯片读出的电压值
* @param[out]	V_out: 滤波后面的当前电压
* @return       分段处理后的电压值
* @note  \n
*/

uint32_t Channel_4(uint32_t V_IN,uint32_t *V_OUT)
{
	uint32_t Volutag = 0;
	Volutag = CH4_filter(V_IN);
	*V_OUT = Volutag;
	if(Volutag <= ch4[0])		//0V
		return 0;
	else if((Volutag > ch4[0]) && (Volutag <= ch4[1]))	//0-0.5v
		return (ch4_f[0] * Volutag);
	else if((Volutag > ch4[1]) && (Volutag <= ch4[2]))	//0.5-1v
		return ((ch4_f[1] * (Volutag - ch4[1])) + 500000);
	else if((Volutag > ch4[2]) && (Volutag <= ch4[3]))	//1-1.5v
		return ((ch4_f[2] * (Volutag - ch4[2])) + 1000000);
	else if((Volutag > ch4[3]) && (Volutag <= ch4[4]))	//1.5-2v
		return ((ch4_f[3] * (Volutag - ch4[3])) + 1500000);	
	else if((Volutag > ch4[4]) && (Volutag <= ch4[5]))	//2-2.5v
		return ((ch4_f[4] * (Volutag - ch4[4])) + 2000000);	
	else if((Volutag > ch4[5]) && (Volutag <= ch4[6]))	//2.5-3v
		return ((ch4_f[5] * (Volutag - ch4[5])) + 2500000);
	else if((Volutag > ch4[6]) && (Volutag <= ch4[7]))	//3-3.5v
		return ((ch4_f[6] * (Volutag - ch4[6])) + 3000000);
	else if((Volutag > ch4[7]) && (Volutag <= ch4[8]))	//3.5-4v
		return ((ch4_f[7] * (Volutag - ch4[7])) + 3500000);
	else if((Volutag > ch4[8]) && (Volutag <= ch4[9]))	//4-4.5v
		return ((ch4_f[8] * (Volutag - ch4[8])) + 4000000);
	else if(Volutag > ch4[9])	//4.5-5v
		return ((ch4_f[9] * (Volutag - ch4[9])) + 4500000);
	return 0;
}

/**@brief       通道5处理
* @param[in]    V_IN: ads1256芯片读出的电压值
* @param[out]	V_out: 滤波后面的当前电压
* @return       分段处理后的电压值
* @note  \n
*/

uint32_t Channel_5(uint32_t V_IN,uint32_t *V_OUT)
{
	uint32_t Volutag = 0;
	Volutag = CH5_filter(V_IN);
	*V_OUT = Volutag;
	if(Volutag <= ch5[0])		//0V
		return 0;
	else if((Volutag > ch5[0]) && (Volutag <= ch5[1]))	//0-0.5v
		return (ch5_f[0] * Volutag);
	else if((Volutag > ch5[1]) && (Volutag <= ch5[2]))	//0.5-1v
		return ((ch5_f[1] * (Volutag - ch5[1])) + 500000);
	else if((Volutag > ch5[2]) && (Volutag <= ch5[3]))	//1-1.5v
		return ((ch5_f[2] * (Volutag - ch5[2])) + 1000000);
	else if((Volutag > ch5[3]) && (Volutag <= ch5[4]))	//1.5-2v
		return ((ch5_f[3] * (Volutag - ch5[3])) + 1500000);	
	else if((Volutag > ch5[4]) && (Volutag <= ch5[5]))	//2-2.5v
		return ((ch5_f[4] * (Volutag - ch5[4])) + 2000000);	
	else if((Volutag > ch5[5]) && (Volutag <= ch5[6]))	//2.5-3v
		return ((ch5_f[5] * (Volutag - ch5[5])) + 2500000);
	else if((Volutag > ch5[6]) && (Volutag <= ch5[7]))	//3-3.5v
		return ((ch5_f[6] * (Volutag - ch5[6])) + 3000000);
	else if((Volutag > ch5[7]) && (Volutag <= ch5[8]))	//3.5-4v
		return ((ch5_f[7] * (Volutag - ch5[7])) + 3500000);
	else if((Volutag > ch5[8]) && (Volutag <= ch5[9]))	//4-4.5v
		return ((ch5_f[8] * (Volutag - ch5[8])) + 4000000);
	else if(Volutag > ch5[9])	//4.5-5v
		return ((ch5_f[9] * (Volutag - ch5[9])) + 4500000);
	return 0;
}

/**@brief       通道6处理
* @param[in]    V_IN: ads1256芯片读出的电压值
* @param[out]	V_out: 滤波后面的当前电压
* @return       分段处理后的电压值
* @note  \n
*/

uint32_t Channel_6(uint32_t V_IN,uint32_t *V_OUT)
{
	uint32_t Volutag = 0;
	Volutag = CH6_filter(V_IN);
	*V_OUT = Volutag;
	if(Volutag <= ch6[0])		//0V
		return 0;
	else if((Volutag > ch6[0]) && (Volutag <= ch6[1]))	//0-0.5v
		return (ch6_f[0] * Volutag);
	else if((Volutag > ch6[1]) && (Volutag <= ch6[2]))	//0.5-1v
		return ((ch6_f[1] * (Volutag - ch6[1])) + 500000);
	else if((Volutag > ch6[2]) && (Volutag <= ch6[3]))	//1-1.5v
		return ((ch6_f[2] * (Volutag - ch6[2])) + 1000000);
	else if((Volutag > ch6[3]) && (Volutag <= ch6[4]))	//1.5-2v
		return ((ch6_f[3] * (Volutag - ch6[3])) + 1500000);	
	else if((Volutag > ch6[4]) && (Volutag <= ch6[5]))	//2-2.5v
		return ((ch6_f[4] * (Volutag - ch6[4])) + 2000000);	
	else if((Volutag > ch6[5]) && (Volutag <= ch6[6]))	//2.5-3v
		return ((ch6_f[5] * (Volutag - ch6[5])) + 2500000);
	else if((Volutag > ch6[6]) && (Volutag <= ch6[7]))	//3-3.5v
		return ((ch6_f[6] * (Volutag - ch6[6])) + 3000000);
	else if((Volutag > ch6[7]) && (Volutag <= ch6[8]))	//3.5-4v
		return ((ch6_f[7] * (Volutag - ch6[7])) + 3500000);
	else if((Volutag > ch6[8]) && (Volutag <= ch6[9]))	//4-4.5v
		return ((ch6_f[8] * (Volutag - ch6[8])) + 4000000);
	else if(Volutag > ch6[9])	//4.5-5v
		return ((ch6_f[9] * (Volutag - ch6[9])) + 4500000);
	return 0;
}

/**@brief       通道7处理
* @param[in]    V_IN: ads1256芯片读出的电压值
* @param[out]	V_out: 滤波后面的当前电压
* @return       分段处理后的电压值
* @note  \n
*/

uint32_t Channel_7(uint32_t V_IN,uint32_t *V_OUT)
{
	uint32_t Volutag = 0;
	Volutag = CH7_filter(V_IN);
	*V_OUT = Volutag;
	if(Volutag <= ch7[0])		//0V
		return 0;
	else if((Volutag > ch7[0]) && (Volutag <= ch7[1]))	//0-0.5v
		return (ch7_f[0] * Volutag);
	else if((Volutag > ch7[1]) && (Volutag <= ch7[2]))	//0.5-1v
		return ((ch7_f[1] * (Volutag - ch7[1])) + 500000);
	else if((Volutag > ch7[2]) && (Volutag <= ch7[3]))	//1-1.5v
		return ((ch7_f[2] * (Volutag - ch7[2])) + 1000000);
	else if((Volutag > ch7[3]) && (Volutag <= ch7[4]))	//1.5-2v
		return ((ch7_f[3] * (Volutag - ch7[3])) + 1500000);	
	else if((Volutag > ch7[4]) && (Volutag <= ch7[5]))	//2-2.5v
		return ((ch7_f[4] * (Volutag - ch7[4])) + 2000000);	
	else if((Volutag > ch7[5]) && (Volutag <= ch7[6]))	//2.5-3v
		return ((ch7_f[5] * (Volutag - ch7[5])) + 2500000);
	else if((Volutag > ch7[6]) && (Volutag <= ch7[7]))	//3-3.5v
		return ((ch7_f[6] * (Volutag - ch7[6])) + 3000000);
	else if((Volutag > ch7[7]) && (Volutag <= ch7[8]))	//3.5-4v
		return ((ch7_f[7] * (Volutag - ch7[7])) + 3500000);
	else if((Volutag > ch7[8]) && (Volutag <= ch7[9]))	//4-4.5v
		return ((ch7_f[8] * (Volutag - ch7[8])) + 4000000);
	else if(Volutag > ch7[9])	//4.5-5v
		return ((ch7_f[9] * (Volutag - ch7[9])) + 4500000);
	return 0;
}
