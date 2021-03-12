/**@file        iic.c
* @brief        iicģ���Ӧ��
* @details      iic��ͨ��Э��
* @author       �����
* @date         2020-08-02
* @version      V1.0.0
* @copyright    2020-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/08/02  <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/

#include "iic.h"

	
#define	SDA_IN HAL_GPIO_ReadPin(IIC_SDA_GPIOX,SDA_PINX) 

/**@brief   ��ʱ����
* - ����Ƶ�ʺ궨���iic�ٶ���ѡ��һ�����ʵ���ʱ
*/
void Delay_iic_us()	
{
	
#if	168 == HclkFrequency
	#if 400 == IIC_SPEED
		for(int i = 0; i < 38; i++)  //168M  //1.25us  //400Kz
		{
			__asm("NOP");
		}
	#else
		for(int i = 0; i < 166; i++)  //168M  //5us  //100Kz
		{
			__asm("NOP");
		}
	#endif	
#endif	

#if	72 == HclkFrequency
	#if 400 == IIC_SPEED
		for(int i = 0; i < 14; i++)  //72M  //1.25us  //400Kz
		{
			__asm("NOP");
		}
	#else
		for(int i = 0; i < 68; i++)  //72M  //5us  //100Kz
		{
			__asm("NOP");
		}
	#endif
#endif

#if	64 == HclkFrequency
	#if 400 == IIC_SPEED
		for(int i = 0; i < 10; i++)  //64M  //1.25us  //400Kz
		{
			__asm("NOP");
		}
	#else	
		for(int i = 0; i < 58; i++)  //64M  //5us  //100Kz
		{
			__asm("NOP");
		}
	#endif	
#endif	

#if	48 == HclkFrequency
	#if 400 == IIC_SPEED	
        for(int i = 0; i < 8; i++)  //48M  //1.25us  //400Kz
        {
            __asm("NOP");
        } 
	#else
		for(int i = 0; i < 43; i++)  //48M  //5us  //100Kz
        {
            __asm("NOP");
        }
	#endif
#endif 

#if	32 == HclkFrequency	
	#if 400 == IIC_SPEED
        for(int i = 0; i < 4; i++)  //32M  //1.25us  //400Kz
        {
            __asm("NOP");
        } 
	#else
		for(int i = 0; i < 28; i++)  //32M  //5us  //100Kz
        {
            __asm("NOP");
        }
	#endif
#endif

#if	24 == HclkFrequency
	#if 400 == IIC_SPEED
        for(int i = 0; i < 2; i++)  //24M  //1.25us  //400Kz
        {
            __asm("NOP");
        } 
	#else
		for(int i = 0; i < 21; i++)  //24M  //5us  //100Kz
        {
            __asm("NOP");
        }
	#endif
#endif

#if	12 == HclkFrequency	
	#if 400 == IIC_SPEED
        for(int i = 0; i < 0; i++)  //12M  //1.25us  //400Kz
        {
			
        } 
	#else
		for(int i = 0; i < 9; i++)  //12M  //5us  //100Kz
        {
            __asm("NOP");
        }
	#endif
#endif
}

/**@brief   ��ʼ��iic
*/
void iic_init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
    SCL_RCC_GPIOX_CLK;									//����GPIOʱ��
	SDA_RCC_GPIOX_CLK;
	
    GPIO_Initure.Pin=SCL_PINX;     		
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  			//�������
    GPIO_Initure.Pull=GPIO_PULLUP;          			//����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     	//����
    HAL_GPIO_Init(IIC_SCL_GPIOX,&GPIO_Initure);    		//��ʼ��
	
	GPIO_Initure.Pin=SDA_PINX;     		
    HAL_GPIO_Init(IIC_SDA_GPIOX,&GPIO_Initure);     	//��ʼ��

}

/**
* @brief ����sda�ܽ�Ϊ�������ģʽ
*/
void iic_sda_out()
{
	GPIO_InitTypeDef GPIO_Initure;
	
	GPIO_Initure.Pin=SDA_PINX;     		
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  			//�������
    GPIO_Initure.Pull=GPIO_PULLUP;          			//����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     	//����
    HAL_GPIO_Init(IIC_SCL_GPIOX,&GPIO_Initure);     	//��ʼ��
}

/**
* @brief ����sda�ܽ�Ϊ����ģʽ
*/
void iic_sda_in()
{
	GPIO_InitTypeDef GPIO_Initure;
	
	GPIO_Initure.Pin=SDA_PINX;     		
    GPIO_Initure.Mode=GPIO_MODE_INPUT;  				//����
    GPIO_Initure.Pull=GPIO_PULLUP;          			//����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     	//����
    HAL_GPIO_Init(IIC_SCL_GPIOX,&GPIO_Initure);     	//��ʼ��
}



/**
* @brief iic��ʼ�ź�
*/
void iic_start(void)
{
	//����������Ϊ���
	iic_sda_out();
	
	//���߿���
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
	HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_SET);
	Delay_iic_us();
	
	//����������
	HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	//SCL����ǯס����
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
}

/**
* @brief iicֹͣ�ź�
*/
void iic_stop(void)
{
	iic_sda_out();			//����������Ϊ���
	
	//SCL����ǯס����
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	//����ʱ����
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
	Delay_iic_us();
	
	//�������ɵ��������
	HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_SET);
}

/**@brief       �ȴ�ACK 
* @return       ����ִ�н��
* - ���ؽ��    : 1-ACK��Ч 0-ACK��Ч
*/
uint8_t iic_wait_ack(void)
{
	uint8_t ack = 0;
	
	iic_sda_in();				//����������Ϊ����
	
	//����ʱ����,�ô��豸���Կ���������
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
	Delay_iic_us();
	
								//��ȡ�����ߵĵ�ƽ
	if(SDA_IN){					//��ЧӦ��
		ack = 1;
		iic_stop();
	}
	else{
		ack = 0;
	}
	
								//SCL����ǯס����
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	return ack;
}

/**@brief       ������Ч/��ЧӦ��
* @param[in]    ack : 1-��Ч 0-��Ч
*/
void iic_ack(uint8_t ack)
{
	iic_sda_out();				//����������Ϊ���
	
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	//����ACK/NACK
	if(ack)
		HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_SET);//��ЧӦ��
	else
		HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_RESET);;//��ЧӦ��
	
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
	
	Delay_iic_us();				//���������ȶ�
	
	//SCL����ǯס����
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
}

/**@brief       ����һ���ֽ�
* @param[in]    txd : Ҫд����ֽ�
*/
void iic_send_byte(uint8_t txd)
{
	uint8_t i;
	
	iic_sda_out();			//����������Ϊ���
	
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	for(i=0;i<8;i++){
		if(txd&1<<(7-i))
			HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_RESET);
		
		Delay_iic_us();
		//����ʱ����
		HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
		Delay_iic_us();
		
		HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
		Delay_iic_us();
	}
}

/**@brief       ����һ���ֽ�
* @return       ����ִ�н��
* - ���ؽ��    : ����һ���ֽ�uint8_t���͵�����
*/
uint8_t iic_read_byte(void)
{
	uint8_t i,rxd = 0;
	
	iic_sda_in();			//����������Ϊ����
	
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	for(i=0;i<8;i++){
		HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
		Delay_iic_us();
		
		if(SDA_IN)
			rxd |= 1<<(7-i);
		
		Delay_iic_us();
		HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
		Delay_iic_us();
	}
	
	return rxd;
}

