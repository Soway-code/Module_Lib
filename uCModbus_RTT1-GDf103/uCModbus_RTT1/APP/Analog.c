#include "BSP.h"
#include <rtthread.h>
#include <rthw.h>

extern volatile uint16_t MB_DATA[125];
#define AD_GROUP_MAX 40
#define AD_CHANNEL_MAX 1
#define AD_DELETE_NUM  10

#define AD_FILTER_MAX 6
#define AD_FILTER_DELETE_NUM 1

#define ADC1_DR_Address    ((uint32_t)0x4001244C) //ADC1�������ĵ�ַ����ο��ֲ�ó���
extern uint16_t MB_DATA_04[600];
uint16_t  usADC1Value[AD_GROUP_MAX];

typedef struct
{
    unsigned long   FilterIndex;
    unsigned short  usFilterBuf[AD_FILTER_MAX];
}struct_ADCFilter;

struct_ADCFilter  ADC1Filter[AD_CHANNEL_MAX];

unsigned short  usFilterBuf[AD_FILTER_MAX];



static void ADC_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//12M  ���14M ����ADCʱ�ӣ�ADCCLK��
	
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;//ADC
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;	//ģ������
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
 	
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 
	ADC_InitStructure.ADC_ScanConvMode = DISABLE; 
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; 
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 
	ADC_InitStructure.ADC_NbrOfChannel = 1; 
	ADC_Init(ADC1, &ADC_InitStructure);
	
	//����ָ��ADC�Ĺ�����ͨ�����������ǵ�ת��˳��Ͳ���ʱ��
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_239Cycles5);
	
	ADC_Cmd(ADC1,ENABLE);	

	ADC_ResetCalibration(ADC1);//����ָ����ADC��У׼�Ĵ���
	while(ADC_GetResetCalibrationStatus(ADC1));//��ȡADC����У׼�Ĵ�����״̬
	
	ADC_StartCalibration(ADC1);//��ʼָ��ADC��У׼״̬
	while(ADC_GetCalibrationStatus(ADC1));//��ȡָ��ADC��У׼����

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//ʹ�ܻ���ʧ��ָ����ADC�����ת����������
}



void Analog_Init()
{
    ADC_Config();
}


#define ADC_MAX_CNT  300
u16 g_wAdcBuffer[ADC_MAX_CNT];
u16 g_wTempAdcBuffer[ADC_MAX_CNT];
//u16 g_wAdcMaxCnt;
u16 g_wAdcCnt;

extern u32 AD_Buff[5][10];                                                             //ADֵ����
extern u32 Weight_Buff[5][10];                                                         //����ֵ����
extern u16 g_wRADValue; //ʵʱ�ɼ�Adֵ
extern u16 g_wFADValue; //�����˲�Adֵ

//����AD��������Ӧ�����
u32 AD_CalWeight(u16 ADValue, u32 AD_buff[][10], u32 Wei_buff[][10], u32 num)
{
    int Weight, i = 0, j = 0, k = 0;
    u8 m, n, p, q, x, y;
    float tana;
    //u16 tmp=0;
    for(i = 0; i < num; i++)
    {
        m = (u8)(i / 10);
        n = (u8)(i % 10);
        if((*(*(&AD_buff[0] + m) + n) == 0xFFFFFFFF) 
        || (*(*(&Wei_buff[0] + m) + n)== 0xFFFFFFFF))                           //���궨�˶���������
        {
            break;
        }  
    }
    j = k = i;                                                                  //�궨��j������

    if(j == 0 || j == 1)                                                        //û�б궨�����ֻ�궨��һ�飬��������Ϊ0
    {
        return 0;
    }
    
    p = (u8)((k - 1) / 10);
    q = (u8)((k - 1) % 10);
    x = (u8)((k - 2) / 10);
    y = (u8)((k - 2) % 10);
    
       
   if(ADValue >= *(*(&AD_buff[0] + p) + q))                                    //AD�������һ���궨��ADֵ,�����һ��ֱ����
    {
        if(*(*(&AD_buff[0] + p) + q) -  *(*(&AD_buff[0] + x) + y) == 0)
        {
            return *(*(&Wei_buff[0] + x) + y);
        }
        tana =  (*(*(&Wei_buff[0] + p) + q) -  *(*(&Wei_buff[0] + x) + y)) * 1.0f / (*(*(&AD_buff[0] + p) + q) -  *(*(&AD_buff[0] + x) + y));
        
        return *(*(&Wei_buff[0] + x) + y) + (u32)(tana * (ADValue - *(*(&AD_buff[0] + x) + y)) + 0.5f);
    }
    for(i = 0; i < j; i++)                                                      //���ADֵ���ĸ���Χ
    {
        m = (u8)(i / 10);
        n = (u8)(i % 10);
        if(ADValue <= *(*(&AD_buff[0] + m) + n))                                //��⵽AD���������
        {
            break;
        }
    }

    if(i == 0)                                                                  //С�ڵ�һ���궨������
    {
        if(*(*(&AD_buff[0]) + 1) - **(&AD_buff[0]) == 0)                        //��ֹ��0
        {
            return **(&Wei_buff[0]);
        }
        tana =(float)((*(*(&Wei_buff[0]) + 1) - **(&Wei_buff[0])) * 1.0f / (*(*(&AD_buff[0]) + 1) - **(&AD_buff[0])));
        Weight = **(&Wei_buff[0]) - (u32)((**(&AD_buff[0]) - ADValue) * tana + 0.5f);
        if(Weight < 0)
        {
            Weight = 0;
        }
        return Weight;
    }
    else
    {
        p = (u8)((i - 1) / 10);
        q = (u8)((i - 1) % 10); 
        if(*(*(&AD_buff[0] + m) + n) - *(*(&AD_buff[0] + p) + q) == 0)
        {
            return *(*(&AD_buff[0] + p) + q);
        }

        tana =(float)((*(*(&Wei_buff[0] + m) + n) - *(*(&Wei_buff[0] + p) + q)) * 1.0f / (*(*(&AD_buff[0] + m) + n) - *(*(&AD_buff[0] + p) + q)));    
            
        return *(*(&Wei_buff[0] + p) + q) + (u32)((ADValue - *(*(&AD_buff[0] + p) + q)) * tana + 0.5f);
    }
}


u16 HandleADValue(u16 *pWBuffer, u16 wLen)
{
    u16 wTemp;
    u16 i;
    u16 j;
    u32 dwSum;
    
    for (i = 0; i < wLen; i++)                                                  //��ADֵװ������
    {
        g_wTempAdcBuffer[i] = *(pWBuffer + i);
    }

    for (i = 0; i < wLen; i++)                                                  //��С��������
    {
        wTemp = g_wTempAdcBuffer[i];
        for (j = i + 1; j < wLen; j++)
        {
            if (wTemp > g_wTempAdcBuffer[j])
            {
                g_wTempAdcBuffer[i] = g_wTempAdcBuffer[j];
                g_wTempAdcBuffer[j] = wTemp;
                wTemp = g_wTempAdcBuffer[i];
            }
        }
    }
    
    dwSum = 0;                              
    for (i = 10; i < wLen - 10; i++)                                            //ȥ��������С�ĸ�10��ֵ��ʣ�����ֵ                              
    {
        dwSum += g_wTempAdcBuffer[i];
    }
    
    wTemp = dwSum / (wLen - 20);
    return (wTemp);
}







void App_FifoData(u16 *pWBuffer, u16 wData, u16 wLen)                           //�����µ�ֵ�������������(FIFO)
{
    u16 i;
    
    for (i = 0; i < wLen - 1; i++)
    {
        *(pWBuffer + i) = *(pWBuffer + i + 1);
    }
    
    *(pWBuffer + i) = wData;
    
    for (i = 0; i < wLen; i++)                                                 
    {
        *(g_wTempAdcBuffer + i) = *(pWBuffer + i);
    }
}

/*
u16 Get_Unit_Weight(u16 Weight)
{
    u16 uWeight;
    
    switch(RunVar.Weight_Unit)
    {
    case 0:
        uWeight = (u16)Weight * 10;
        break;
    case 1:
        uWeight = (u16)Weight;
        break;
    case 2:
        uWeight = (u16)(Weight * 0.1);
        break;        
    case 3:
        uWeight = (u16)(Weight * 0.01);
        break;
    default:
        break;
    }
    return uWeight;
}
*/
u32 Get_Unit_Weight(u32 Weight)// 3.�޸�Get_Unit_Weight���������������ͺ͵��ò������͸�Ϊ32λ�� 20190712
{
    u32 uWeight;
    
    switch(RunVar.Weight_Unit)
    {
    case 0:
        uWeight = (u32)Weight * 10;
        break;
    case 1:
        uWeight = (u32)Weight;
        break;
    case 2:
        uWeight = (u32)(Weight * 0.1);
        break;        
    case 3:
        uWeight = (u32)(Weight * 0.01);
        break;
    default:
        break;
    }
    return uWeight;
}

//�˲�
uint16_t Get_filter(uint16_t PCap_buf)
{
	uint8_t i=0,j=0;	
	uint32_t buf1 = 0,bufz[12]={0};
	static uint32_t buf_PCap[14]={0},buf2;
	static uint16_t buf3;
	static uint8_t signs = 0,initial = 0;
	
	if(signs >= 14)
		signs = 0;
	
	if(initial < 13)
	{
		buf2 = 0;
		initial++;
		buf_PCap[signs]=PCap_buf;
		for(i = 0;i <= signs;)
		{
			buf2+=buf_PCap[i++];
		}
		signs++;
                return 0;
	}
	
	if(0 != PCap_buf)			
		buf_PCap[signs]=PCap_buf;			//��Чֵ
	
	j = signs++;
	for(i = 0;i<12;i++)
	{
		bufz[i] = buf_PCap[j];
		if(j <= 0)
			j = 14;
		j--;
	}
	
	for(i=0; i<12; i++)
	{
		for(j=0; j<12-i; j++)
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
        if(MB_DATA[53]  == 1)
        {
            buf2+=bufz[2]+bufz[3]+bufz[4]+bufz[5]+bufz[6]+bufz[7]+bufz[8]+bufz[9];
            buf3 = (uint32_t)(buf2 /8.0);
        }
        else if(MB_DATA[53]  == 3)
        {
            buf2+=bufz[4]+bufz[5]+bufz[6]+bufz[7];
            buf3 = (uint32_t)(buf2 /4.0);
        }
        else
        {
            buf2+=bufz[3]+bufz[4]+bufz[5]+bufz[6]+bufz[7]+bufz[8];
            buf3 = (uint32_t)(buf2 /6.0);
        }
	
	return buf3;
}

void App_ADC_Task()
{
	Analog_Init();
	while(1)
	{

		rt_hw_interrupt_disable();	//�ٽ���
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));//ת��������־λ	
		g_wRADValue = ADC_GetConversionValue(ADC1);//�������һ��ADCx�������ת�����
		g_wFADValue = Get_filter(g_wRADValue);
        rt_hw_interrupt_enable(0); //�˳��ٽ��� 
		MB_DATA_04[8+1] = g_wFADValue;
		rt_thread_mdelay(50);/* ��ʱ50��tick */
		RunVar.uiAI[0] = g_wFADValue;
	
	UserParam.CarLoadWeight = AD_CalWeight(g_wFADValue, AD_Buff, Weight_Buff, 50U);
        UserParam.CarLoadWeight = Get_Unit_Weight(UserParam.CarLoadWeight);
        if(UserParam.CarLoadWeight > RunVar.OverloadLimit)
        {
            g_cyWorkState = 3;  //����
        }
        else if(UserParam.CarLoadWeight > RunVar.LoadLimit)     
        {
            g_cyWorkState = 2;  //����
        }                             
        else if(UserParam.CarLoadWeight > RunVar.LightLimit)
        {
            g_cyWorkState = 7;  //����
        }
        else if(UserParam.CarLoadWeight > RunVar.EmptyLimit)
        {
            g_cyWorkState = 6;  //����
        }
        else
        {
            g_cyWorkState = 1;  //����
        }
	}


}
