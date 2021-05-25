#include  "ADC.h"
#include "gd32f10x.h"
#include "calculate.h"

uint16_t adc_value;
uint16_t adc_value;
uint16_t FilterAdc;
uint16_t ton = 0;				//重量
/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{

    /* enable GPIOA clock */
    rcu_periph_clock_enable(RCU_GPIOA);  
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA0);
    /* enable TIMER0 clock */
    rcu_periph_clock_enable(RCU_TIMER0);
    /* enable ADC0 clock */
    rcu_periph_clock_enable(RCU_ADC0);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV12);
}


/*!
    \brief      configure the GPIO peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/


/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void dma_config(void)
{
    /* ADC_DMA_channel configuration */
    dma_parameter_struct dma_data_parameter;
     gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    /* ADC_DMA_channel deinit */
    dma_deinit(DMA0, DMA_CH0);
    
    /* initialize DMA single data mode */
    dma_data_parameter.periph_addr  = (uint32_t)(&ADC_RDATA(ADC0));
    dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr  = (uint32_t)(&adc_value);
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number       = 1;
    dma_data_parameter.priority     = DMA_PRIORITY_HIGH;  
    dma_init(DMA0, DMA_CH0, &dma_data_parameter);
  
    dma_circulation_enable(DMA0, DMA_CH0);
  
    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH0);  
//}

///*!
//    \brief      configure the ADC peripheral
//    \param[in]  none
//    \param[out] none
//    \retval     none
//*/
//void adc_config(void)
//{
    /* ADC scan mode function enable */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_T0_CH0);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* configure the ADC sync mode */
    adc_mode_config(ADC_DAUL_REGULAL_PARALLEL);  
    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);
  
    /* ADC regular channel config */
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_0, ADC_SAMPLETIME_239POINT5);

  
    /* ADC external trigger enable */
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);

   
    /* enable ADC interface */
    adc_enable(ADC0);
    GD32F103_Systick_Delayxms(1);    
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
    /* enable ADC interface */
    /* ADC DMA function enable */
    adc_dma_mode_enable(ADC0);
}

/*!
    \brief      configure the timer peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void timer_config(void)
{
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    /* TIMER0 configuration */
    timer_initpara.prescaler         = 8399;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 9999;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0, &timer_initpara);

    /* CH0 configuration in PWM mode0 */
    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
    timer_channel_output_config(TIMER0, TIMER_CH_0, &timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, 3999);
    timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

    /* TIMER0 primary output enable */
    timer_primary_output_config(TIMER0, ENABLE);
    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER0); 
    
    /* enable TIMER0 */
    timer_enable(TIMER0);
}


//滤波
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
        return PCap_buf;        
	}
	
			
		buf_PCap[signs]=PCap_buf;			//有效值
	
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

     buf2+=bufz[4]+bufz[5]+bufz[6]+bufz[7];
     buf3 = (uint32_t)(buf2 /4.0);

	return buf3;
}

bool ADC_Filter( void )
{
	uint16_t buf = adc_value;
	if(buf < 0xFFF)
		 FilterAdc = Get_filter(adc_value);
	ton = WeightProcessing(FilterAdc);
	return true;
}
