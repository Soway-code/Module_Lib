#include "DataTreating.h"
#include "Flash_app.h"
#include "calculate.h"

void TIM3_NVIC_Init (void){ //����TIM3�ж�����
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x3;	//������ռ�������ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_Init(){  //TIM3 ��ʼ�� arr��װ��ֵ pscԤ��Ƶϵ��
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseInitStrue;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//ʹ��TIM3
    TIM3_NVIC_Init (); //����TIM3�ж�����
	      
    TIM_TimeBaseInitStrue.TIM_Period=9999; //�����Զ���װ��ֵ
    TIM_TimeBaseInitStrue.TIM_Prescaler=4799; //Ԥ��Ƶϵ��
    TIM_TimeBaseInitStrue.TIM_CounterMode=TIM_CounterMode_Up; //�������������
    TIM_TimeBaseInitStrue.TIM_ClockDivision=TIM_CKD_DIV1; //ʱ�ӵķ�Ƶ���ӣ�����һ������ʱ���ã�һ����ΪTIM_CKD_DIV1
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStrue); //TIM3��ʼ������
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//ʹ��TIM3�ж�    
    TIM_Cmd(TIM3,ENABLE); //ʹ��TIM3
}
static uint32_t tim_D01 = 0;
void TIM3_IRQHandler(void){ //TIM3�жϴ�����
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){	//�ж��Ƿ���TIM3�ж�
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        //�˴�д���û��Լ��Ĵ������
		tim_D01++;
    }
}

void D01_Dispose(uint16_t ADC0)	//D01�����뱨����־λ�����ر�־λ����
{
	static uint16_t adc_buf=0xFFF5;		//��¼��ǰADC
	static uint16_t adc_buf1;		//��㵽������adֵ����Чֵ
	static uint8_t percent = 0,percent1 = 0;				//�ٷֱ�
	adc_buf1 = FullCalibrat - NullCalibrat;		//��㵽������adֵ

	if(adc_buf == 0xFFF5)
		adc_buf = ADC0;
	if(tim_D01 >= (DampTime*5))
	{
		percent = (float)(ADC0 - NullCalibrat)/(float)adc_buf1*100.0;	//��ǰ�ٷֱ�
		tim_D01 = 0;
		if(percent > percent1)		//�µİٷֱȱ���һ�δ�
		{
			percent1 = percent - percent1;
			if(percent1 >= Threshold)		//������ֵ
				D01_sign[0] = 2;			//����������ͻȻ����
			else
				D01_sign[0] = 0;			//����
		}
		else
		{
			percent1 = percent1 - percent;
			if(percent1 >= Threshold)		//������ֵ
				D01_sign[0] = 1;			//����ͻȻ�½�
			else
				D01_sign[0] = 0;			//����
		}

		percent1 = percent;				//��һ�ΰٷֱ�
	
	}
	if(ADC0 <= Null_Ad) 				//��������
		D01_sign[1] = 1;
	else if((ADC0 >Null_Ad) && (ADC0 < Reload_Ad))
		D01_sign[1] = 0;				//����
	else if((ADC0 >= Reload_Ad) && (ADC0 < Overload_Ad))
		D01_sign[1] = 2;				//����
	else if(ADC0 >= Overload_Ad)
		D01_sign[1] = 3;				//����
}

void D81_Dispose(void)						//D81����������������
{
	static uint16_t adc[22] = {0};   			//����20��21�ǿ���У׼������У׼ֵ
	static uint16_t weight[20] = {0};
	uint8_t i = 0,j = 0,k=0;
	uint8_t count = 0;
	count = Flash_Read_OneByte(NOW_ADDR);		//�����Ѿ��궨����
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

