#include "DataTreating.h"
#include "Flash_app.h"
#include "calculate.h"

void TIM3_NVIC_Init (void){ //开启TIM3中断向量
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x3;	//设置抢占和子优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_Init(){  //TIM3 初始化 arr重装载值 psc预分频系数
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseInitStrue;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//使能TIM3
    TIM3_NVIC_Init (); //开启TIM3中断向量
	      
    TIM_TimeBaseInitStrue.TIM_Period=9999; //设置自动重装载值
    TIM_TimeBaseInitStrue.TIM_Prescaler=4799; //预分频系数
    TIM_TimeBaseInitStrue.TIM_CounterMode=TIM_CounterMode_Up; //计数器向上溢出
    TIM_TimeBaseInitStrue.TIM_ClockDivision=TIM_CKD_DIV1; //时钟的分频因子，起到了一点点的延时作用，一般设为TIM_CKD_DIV1
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStrue); //TIM3初始化设置
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//使能TIM3中断    
    TIM_Cmd(TIM3,ENABLE); //使能TIM3
}
static uint32_t tim_D01 = 0;
void TIM3_IRQHandler(void){ //TIM3中断处理函数
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){	//判断是否是TIM3中断
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        //此处写入用户自己的处理程序
		tim_D01++;
    }
}

void D01_Dispose(uint16_t ADC0)	//D01功能码报警标志位空重载标志位处理
{
	static uint16_t adc_buf=0xFFF5;		//记录当前ADC
	static uint16_t adc_buf1;		//零点到满量程ad值总有效值
	static uint8_t percent = 0,percent1 = 0;				//百分比
	adc_buf1 = FullCalibrat - NullCalibrat;		//零点到满量程ad值

	if(adc_buf == 0xFFF5)
		adc_buf = ADC0;
	if(tim_D01 >= (DampTime*5))
	{
		percent = (float)(ADC0 - NullCalibrat)/(float)adc_buf1*100.0;	//当前百分比
		tim_D01 = 0;
		if(percent > percent1)		//新的百分比比上一次大
		{
			percent1 = percent - percent1;
			if(percent1 >= Threshold)		//大于阈值
				D01_sign[0] = 2;			//代表重量有突然上升
			else
				D01_sign[0] = 0;			//正常
		}
		else
		{
			percent1 = percent1 - percent;
			if(percent1 >= Threshold)		//大于阈值
				D01_sign[0] = 1;			//重量突然下降
			else
				D01_sign[0] = 0;			//正常
		}

		percent1 = percent;				//上一次百分比
	
	}
	if(ADC0 <= Null_Ad) 				//车辆空载
		D01_sign[1] = 1;
	else if((ADC0 >Null_Ad) && (ADC0 < Reload_Ad))
		D01_sign[1] = 0;				//正常
	else if((ADC0 >= Reload_Ad) && (ADC0 < Overload_Ad))
		D01_sign[1] = 2;				//重载
	else if(ADC0 >= Overload_Ad)
		D01_sign[1] = 3;				//超载
}

void D81_Dispose(void)						//D81功能码重量处理函数
{
	static uint16_t adc[22] = {0};   			//后面20和21是空载校准和满载校准值
	static uint16_t weight[20] = {0};
	uint8_t i = 0,j = 0,k=0;
	uint8_t count = 0;
	count = Flash_Read_OneByte(NOW_ADDR);		//读出已经标定数据
	for(;i<count;i++)
	{
		adc[k] = Flash_Read_twoByte(AD1_ADDR +(j*0x02));
		weight[k++] = Flash_Read_twoByte(LOAD1_ADDR +(j*0x02));	
		j++;
	}
	adc[20] = Flash_Read_twoByte(NULL_CAIIBRATION);
	adc[21] = Flash_Read_twoByte(FULL_CAIIBRATION);
	WeightProcessing_init(adc,weight,count);
	
}

