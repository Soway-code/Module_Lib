#include "BSP.h"


#define FREQRECORDMAXNUM    60
typedef struct
{
    unsigned long  Index;
    BitAction bFull;
    u16 usPluse[FREQRECORDMAXNUM];
}PreFREQRecordTypeDef;

static u32 IOStatue[3];

PreFREQRecordTypeDef PluseRecord;

void FREQRec_Init(PreFREQRecordTypeDef *pRec)
{
    pRec->Index = 0;

    for(unsigned long i = 0; i < FREQRECORDMAXNUM; i++)
    {
        pRec->usPluse[i] = 0;
    }

    pRec->bFull = Bit_RESET;
}

void FREQRec_Insert(PreFREQRecordTypeDef *pRec, unsigned short NewFreq)
{
    unsigned long Index = pRec->Index;

    pRec->usPluse[Index] = NewFreq;

    if(++Index >= FREQRECORDMAXNUM)
    {
        Index = 0;
        pRec->bFull = Bit_SET;
    }

    pRec->Index = Index;
}

u16 FREQRec_GetDistand(PreFREQRecordTypeDef *pRec)
{
    unsigned long tailIndex = pRec->Index;
    unsigned long headIndex = tailIndex;
    u16 usTemp = 0;

    if(pRec->bFull == Bit_SET)
    {
        tailIndex = (0 == tailIndex) ? (FREQRECORDMAXNUM - 1) : (tailIndex - 1);
    
        return((pRec->usPluse[tailIndex] - pRec->usPluse[headIndex]) * 2);
    }
    else
    {
        tailIndex = tailIndex - 1;
        headIndex = 0;
        usTemp = pRec->usPluse[tailIndex] - pRec->usPluse[headIndex];

        usTemp *= 2;
        usTemp *= FREQRECORDMAXNUM;
        usTemp /= (tailIndex + 1);
        
        return(usTemp);
    }
}


void TIM3_IRQHandler(void)
{
    OS_ERR Err;
    unsigned short T1cnt;

    if (RESET != TIM_GetITStatus(TIM3, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        T1cnt = TIM_GetCounter(TIM1);

        FREQRec_Insert(&PluseRecord, T1cnt);

        OSTaskSemPost(&AppDITCB, OS_OPT_POST_NONE, &Err);
	}
}


//timer1设置为进油管脉冲计数器
void Timer1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_2);

    TIM_DeInit(TIM1);

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 0x00;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_ETRClockMode2Config(TIM1, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0x0F);

    TIM_SetCounter(TIM1, 0);
    TIM_Cmd(TIM1, ENABLE);
}

//timer2设置为出油管脉冲计数器
void Timer2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_2);

    TIM_DeInit(TIM2);

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 0x00;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);   // Time base configuration

    TIM_ETRClockMode2Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0x0F);

    TIM_SetCounter(TIM2, 0);
    TIM_Cmd(TIM2, ENABLE);
}

void Timer3_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 4999;   //9999;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 4799;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
    NVIC_Init(&NVIC_InitStruct);

    BSP_IntVectSet(BSP_INT_ID_TIM3, TIM3_IRQHandler);

    TIM_SetCounter(TIM3, 0);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void Capture_TimerInit(void)
{
    Timer1_Init();
    Timer2_Init();
    Timer3_Init();
}

void RotationDirInit(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    /* Enable GPIOA clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /* Configure PA15 pins as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void PB5Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    /* Enable GPIOB clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /* Configure PB5 pins as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void PB12Init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    /* Enable GPIOB clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /* Configure PB12 pins as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

BitAction IOStatueFilter(u32 ulStatue, BitAction bLastStatue)
{
    u8 uiOneNum = 0;

    for(long i = 0; i < 10; i++)
    {
        if(0 != (ulStatue & (0x01 << i)))
        {
            uiOneNum += 1;
        }
    }

    if(Bit_SET == bLastStatue)  //如果是闭合状态，就查断开状态
    {
        return (uiOneNum <= 3) ? Bit_RESET : Bit_SET;
    }
    else        //如果是断开状态，就查闭合状态
    {
        return (uiOneNum >= 7) ? Bit_SET : Bit_RESET;
    }
}

void App_DI_Task (void *p_arg)
{
    OS_ERR Err;	
    u32 ulIOStatueBak;

    FREQRec_Init(&PluseRecord);
    
	Capture_TimerInit();

    RotationDirInit();

    PB5Init();

    PB12Init();

    IOStatue[0] = 0x3FF;
    IOStatue[1] = 0x3FF;
    IOStatue[2] = 0x3FF;
    RunVar.bPB5 = Bit_SET;
    RunVar.bPB12 = Bit_SET;
    
    while(1)
    {
        OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, NULL, &Err);

        #if IWDG_EN > 0
            IWDG_ReloadCounter();
        #endif   

        if (Err == OS_ERR_NONE)
        {
            ulIOStatueBak = 0;
        
                /*Read Rotation speed per minute*/
            RunVar.ulRotationSpeed = FREQRec_GetDistand(&PluseRecord);
        
                    /*Read Rotation direction, anti - dithering*/
            if(0 == GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3))
            {
                ulIOStatueBak |= GPIO_Pin_3;
            }
            if(0 == GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5))
            {
                ulIOStatueBak |= GPIO_Pin_5;
            }
            if(0 == GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12))
            {
                ulIOStatueBak |= GPIO_Pin_12;
            }
            
            OSTimeDly(10, OS_OPT_TIME_TIMEOUT, &Err);

            for(long i = 0; i < 3; i++)
            {
                IOStatue[i] <<= 1;
                IOStatue[i] &= 0x3FF;
            }
            
            if((0 == (BitAction)GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3)) 
                && (0 != (ulIOStatueBak & GPIO_Pin_3)))
            {
                IOStatue[0] |= 0x1;
            }
            if((0 == (BitAction)GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5)) 
                && (0 != (ulIOStatueBak & GPIO_Pin_5)))
            {
                IOStatue[1] |= 0x1;
            }
            if((0 == (BitAction)GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) 
                && (0 != (ulIOStatueBak & GPIO_Pin_12)))
            {
                IOStatue[2] |= 0x1;
            }

            RunVar.bRotationDir = IOStatueFilter(IOStatue[0], RunVar.bRotationDir);
            RunVar.bPB5 = IOStatueFilter(IOStatue[1], RunVar.bPB5);
            RunVar.bPB12 = IOStatueFilter(IOStatue[2], RunVar.bPB12);
        }
    }
}

