/**@file        main.c
* @details      配置中断，端口等应用的API函数
* @author       马灿林
* @date         2021-03-09
* @version      V1.0.0
* @copyright    2021-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/03/09  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/
#include "gd32e23x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32e230c_eval.h"
uint32_t sign = 0,sign1 = 0,sign2 = 0,sign3 = 20,sign4 = 20,sign5=0;
uint8_t key1=0,key2 = 0;
uint16_t adc_value[4];
void dma_config(void);
void adc_config(void);

/**@brief       led初始化 
* @param[in]    无
* @return       无
* @note         
*/
void LED_Init(void)
{
	/* enable the GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
	/* configure PA2(ADC channel2) as analog input */
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_1 | GPIO_PIN_0);
	
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1 | GPIO_PIN_0);
	
	gpio_bit_reset(GPIOA,GPIO_PIN_1);
	
	gpio_bit_set(GPIOA,GPIO_PIN_0);
}

/**@brief       led状态设置函数 
* @param[in]    led状态 1或者1
* @return       无
* @note         1 绿色  0红色
*/
void Led_Control(uint8_t color)  
{       
	if(color)  //1 绿色
	{
		gpio_bit_reset(GPIOA,GPIO_PIN_1);
		gpio_bit_set(GPIOA,GPIO_PIN_0);

	}
	else      // 0  红色
	{     
		gpio_bit_reset(GPIOA,GPIO_PIN_0);
		gpio_bit_set(GPIOA,GPIO_PIN_1);

	}
}

/**@brief       管脚输出配置
* @param[in]    
* @param[in]    
* @return       无
* @note        
*/
void signal_output_Init(void)
{
		/* enable the GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);
	/* configure PA2(ADC channel2) as analog input */
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_7 | GPIO_PIN_0);
	
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_0);
	
	gpio_bit_set(GPIOB, GPIO_PIN_0);
	gpio_bit_set(GPIOB, GPIO_PIN_7);

}

/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable GPIOC clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC);
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);
}

/*!
    \brief      initialize the key GPIO
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ZF_gpio_init(void)
{
    /* enable the TAMPER key gpio clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* configure button pin as input */
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_8 | GPIO_PIN_9);

}

/*!
    \brief      initialize the EXTI configuration of the key
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ZF_exti_init(void)
{
	ZF_gpio_init();
    /* enable the CFGCMP clock */
    rcu_periph_clock_enable(RCU_CFGCMP);
    /* enable and set key EXTI interrupt to the specified priority */
    nvic_irq_enable(EXTI4_15_IRQn, 0U);

    /* connect key EXTI line to key GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN8 | EXTI_SOURCE_PIN9);

    /* configure key EXTI line */
    exti_init(EXTI_8 | EXTI_9, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_interrupt_flag_clear(EXTI_8 | EXTI_9);
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

int main(void)
{
    /* configure systick */
    rcu_config();
	systick_config();
	
    /* initilize the LEDs, USART and key */
	LED_Init();
	signal_output_Init();
	ZF_exti_init();

	while(1)
	{
		
		if((sign>14) && (sign1>14))
		{	
			sign=0;
			sign1=0;
		}
		if((sign == 1) &&(sign1 == 1))
		{
			key2 = gpio_input_bit_get(GPIOA,GPIO_PIN_8);
			key1 = gpio_input_bit_get(GPIOA,GPIO_PIN_9);
			if((key1==1) && (key2 == 1))
			{
				sign = 0;
				sign1 = 0;
				sign2 = 0;
			}	
		}	
		
		if((sign2 == 9) && (sign4 >= 20))
		{
			if((sign == 1) && (sign1 == 1))
			{
				Led_Control(0);  // 1 绿色  0红色
				gpio_bit_reset(GPIOB, GPIO_PIN_7);
				gpio_bit_reset(GPIOB, GPIO_PIN_0);
			}
			else if((sign == 2) && (sign1 == 2))
			{
				sign = 0;
				sign1 = 0;
				gpio_bit_set(GPIOB, GPIO_PIN_0);
			}

		}
		if((sign2 == 8)&& (sign3 >= 20))
		{
			if((sign == 1) && (sign1 == 1))
			{
				Led_Control(1);  // 1 绿色  0红色
				gpio_bit_set(GPIOB, GPIO_PIN_7);	//高
				gpio_bit_reset(GPIOB, GPIO_PIN_0);
			}
			else if((sign == 2) && (sign1 == 2))
			{
				sign = 0;
				sign1 = 0;
				gpio_bit_set(GPIOB, GPIO_PIN_0);
			}
		}
		if((sign != 1) && (sign1 != 1))
			gpio_bit_set(GPIOB, GPIO_PIN_0);
		if(sign2 == 8)
		{
			if(sign5 == 0)
			{	
				sign3++;
				sign5=1;
			}
			sign4 = 0;
		}
		if(sign2 == 9)
		{
			if(sign5 == 0)
			{
				sign4++;
				sign5=1;
			}
			sign3 = 0;			
		}

		delay_1ms(1);	
    }
}


/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM, (uint8_t)ch);
    while(RESET == usart_flag_get(EVAL_COM, USART_FLAG_TBE));

    return ch;
}

void EXTI4_15_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_8)) 
	{
        sign++;
		if(sign > sign1)
			sign2 = 8;
		exti_interrupt_flag_clear(EXTI_8);
		
    }
	
	 if(RESET != exti_interrupt_flag_get(EXTI_9)) 
	{
        sign1++;
		if(sign < sign1)
			sign2 = 9;
		exti_interrupt_flag_clear(EXTI_9);
    }

	sign5 = 0;
}
