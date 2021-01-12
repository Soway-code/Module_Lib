#include "BSP.h"
#include "os_cfg_app.h"


#define AD_GROUP_MAX 40
#define AD_CHANNEL_MAX 1
#define AD_DELETE_NUM  10

#define AD_FILTER_MAX 6
#define AD_FILTER_DELETE_NUM 1


vu16  usADC1ConvertedValue[AD_GROUP_MAX][AD_CHANNEL_MAX];
u16  usADC1Value[AD_GROUP_MAX];

typedef struct
{
    unsigned long   FilterIndex;
    unsigned short  usFilterBuf[AD_FILTER_MAX];
}struct_ADCFilter;

struct_ADCFilter  ADC1Filter[AD_CHANNEL_MAX];

unsigned short  usFilterBuf[AD_FILTER_MAX];

static void ADC_Config(void);
static void DMA_Config(void);


static void ADC_Config(void)
{
    ADC_InitTypeDef       ADC_InitStructure;
    GPIO_InitTypeDef      GPIO_InitStructure;

    ADC_DeInit(ADC1);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;// | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_6| GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    //GPIO_Init(GPIOB, &GPIO_InitStructure);

    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_ChannelConfig(ADC1, ADC_Channel_0 , ADC_SampleTime_239_5Cycles);
    //ADC_ChannelConfig(ADC1, ADC_Channel_1 , ADC_SampleTime_239_5Cycles);
    //ADC_ChannelConfig(ADC1, ADC_Channel_2 , ADC_SampleTime_239_5Cycles);
    //ADC_ChannelConfig(ADC1, ADC_Channel_3, ADC_SampleTime_239_5Cycles);
    //ADC_ChannelConfig(ADC1, ADC_Channel_6, ADC_SampleTime_239_5Cycles);
    //ADC_ChannelConfig(ADC1, ADC_Channel_7, ADC_SampleTime_239_5Cycles);
    //ADC_ChannelConfig(ADC1, ADC_Channel_8, ADC_SampleTime_239_5Cycles);
    //ADC_ChannelConfig(ADC1, ADC_Channel_9, ADC_SampleTime_239_5Cycles);
    
    ADC_GetCalibrationFactor(ADC1);
    ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);  //????DMA?-?·?￡ê?
    ADC_Cmd(ADC1, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY));
    ADC_StartOfConversion(ADC1);
}


static void DMA_Config(void)
{
    DMA_InitTypeDef  DMA_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (ADC1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) & usADC1ConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = AD_GROUP_MAX * AD_CHANNEL_MAX;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);
}

void Analog_Init()
{
    ADC_Config();

    DMA_Config();
}



extern u16 g_wRADValue; //实时采集Ad值
extern u16 g_wFADValue; //经过滤波Ad值

#define ADC_MAX_CNT  300
u16 g_wAdcBuffer[ADC_MAX_CNT];
u16 g_wTempAdcBuffer[ADC_MAX_CNT];
//u16 g_wAdcMaxCnt;
u16 g_wAdcCnt;

extern u32 AD_Buff[5][10];                                                             //AD值缓存
extern u32 Weight_Buff[5][10];                                                         //载重值缓存


//根据AD和重量对应表计算
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
        || (*(*(&Wei_buff[0] + m) + n)== 0xFFFFFFFF))                           //检测标定了多少组数据
        {
            break;
        }  
    }
    j = k = i;                                                                  //标定了j组数据

    if(j == 0 || j == 1)                                                        //没有标定数组或只标定了一组，计算重量为0
    {
        return 0;
    }
    
    p = (u8)((k - 1) / 10);
    q = (u8)((k - 1) % 10);
    x = (u8)((k - 2) / 10);
    y = (u8)((k - 2) % 10);
    
       
   if(ADValue >= *(*(&AD_buff[0] + p) + q))                                    //AD大于最后一个标定的AD值,用最后一条直线算
    {
        if(*(*(&AD_buff[0] + p) + q) -  *(*(&AD_buff[0] + x) + y) == 0)
        {
            return *(*(&Wei_buff[0] + x) + y);
        }
        tana =  (*(*(&Wei_buff[0] + p) + q) -  *(*(&Wei_buff[0] + x) + y)) * 1.0f / (*(*(&AD_buff[0] + p) + q) -  *(*(&AD_buff[0] + x) + y));
        
        return *(*(&Wei_buff[0] + x) + y) + (u32)(tana * (ADValue - *(*(&AD_buff[0] + x) + y)) + 0.5f);
    }
    for(i = 0; i < j; i++)                                                      //检测AD值在哪个范围
    {
        m = (u8)(i / 10);
        n = (u8)(i % 10);
        if(ADValue <= *(*(&AD_buff[0] + m) + n))                                //检测到AD所在区间段
        {
            break;
        }
    }

    if(i == 0)                                                                  //小于第一个标定的数据
    {
        if(*(*(&AD_buff[0]) + 1) - **(&AD_buff[0]) == 0)                        //防止除0
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
    
    for (i = 0; i < wLen; i++)                                                  //把AD值装进数组
    {
        g_wTempAdcBuffer[i] = *(pWBuffer + i);
    }

    for (i = 0; i < wLen; i++)                                                  //从小到大排序
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
    for (i = 10; i < wLen - 10; i++)                                            //去掉最大和最小的各10个值，剩下求均值                              
    {
        dwSum += g_wTempAdcBuffer[i];
    }
    
    wTemp = dwSum / (wLen - 20);
    return (wTemp);
}







void App_FifoData(u16 *pWBuffer, u16 wData, u16 wLen)                           //把最新的值放在数组最后面(FIFO)
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
u32 Get_Unit_Weight(u32 Weight)// 3.修改Get_Unit_Weight函数，将返回类型和调用参数类型改为32位。 20190712
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


//extern struct OutData myOutData;
void App_Analog_Task (void *p_arg)
{
    OS_ERR Err;
    
    FlagStatus FillBuffFlag = RESET;
    
    CPU_SR_ALLOC();
        
    Analog_Init();

//    RunVar.ulLoadStatue = 0xffffffff;
    
//    LOAD_GetPar();
	
    while(1)
    {
        #if IWDG_EN > 0
            IWDG_ReloadCounter();
        #endif	

        OSTaskSemPend(10U, OS_OPT_PEND_BLOCKING, NULL, &Err);                   //等待10个节拍1000ms
        
        for(long channel = 0; channel < AD_CHANNEL_MAX; channel++)             //采集的AD值求均值
          //只有一个通道
        {
            CPU_CRITICAL_ENTER();  //禁止CPU中断
            for(long i = 0; i < AD_GROUP_MAX; i++)  //AD_GROUP_MAX = 40
            {
                usADC1Value[i] = usADC1ConvertedValue[i][channel];
            }
            CPU_CRITICAL_EXIT(); //开户CPU中断
       
            RunVar.uiAI[channel] = LOAD_GetAverage(usADC1Value, AD_GROUP_MAX);
        }       
        g_wRADValue = RunVar.uiAI[0];
        //RunVar.AdcMaxCnt = ProductPara.cyFilterLevel * 90;
        if(FillBuffFlag == RESET)
        {
            FillBuffFlag = SET;
            g_wFADValue = g_wRADValue;                                          //上电滤波值先置一个固定值
        }
        if (RunVar.AdcMaxCnt > ADC_MAX_CNT)                                     //ADC最大采集数量300个（滤波系数最大设置为6）
        {
            RunVar.AdcMaxCnt = ADC_MAX_CNT;
        }
        if (ProductPara.cyFilterLevel == 1)                                     //滤波系数设置为1，实时数据
        {
            g_wFADValue = RunVar.uiAI[0]; 
        }     
        else
        {
            if(RunVar.AdcMaxCnt > g_wAdcCnt)                                    //取满g_wAdcMaxCnt个值
            {
                g_wAdcBuffer[g_wAdcCnt++] = RunVar.uiAI[0];                       
            } 
            else                                                               //剩下的按照FIFO方式填满数组
            {                
                App_FifoData(g_wAdcBuffer, RunVar.uiAI[0], RunVar.AdcMaxCnt);   //最新的数据放在队尾             
                //g_wFADValue = HandleADValue(g_wAdcBuffer, g_wAdcMaxCnt);      //排序，去掉最大和最小各10个后取均值
                g_wFADValue = GetDelExtremeAndAverage(g_wTempAdcBuffer, RunVar.AdcMaxCnt, 10U, 10U);               
            }                                                                   
        }
       // g_wFADValue = 0x0006;
        
        UserParam.CarLoadWeight = AD_CalWeight(g_wFADValue, AD_Buff, Weight_Buff, 50U);
        UserParam.CarLoadWeight = Get_Unit_Weight(UserParam.CarLoadWeight);
        if(UserParam.CarLoadWeight > RunVar.OverloadLimit)
        {
            g_cyWorkState = 3;  //超载
        }
        else if(UserParam.CarLoadWeight > RunVar.LoadLimit)     
        {
            g_cyWorkState = 2;  //满载
        }                             
        else if(UserParam.CarLoadWeight > RunVar.LightLimit)
        {
            g_cyWorkState = 7;  //重载
        }
        else if(UserParam.CarLoadWeight > RunVar.EmptyLimit)
        {
            g_cyWorkState = 6;  //轻载
        }
        else
        {
            g_cyWorkState = 1;  //空载
        }
      //LOAD_LoadTask();       
    }
}


