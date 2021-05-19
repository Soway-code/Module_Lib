#include "SimulateIic.h"


 //MPU IIC ��ʱ����
void MPU_IIC_Delay(void)
{
	uint8_t i=30;
	while(i--) ;
}


//��ʼ��IIC
void MPU_IIC_Init(void)
{					   
#if stm32f103	
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//��ʹ������IO PORTBʱ�� 
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;	 // �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIO 

	GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);						 //PB10,PB11 �����	
#else
	rcu_periph_clock_enable(RCU_GPIOB);
	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9);
	gpio_bit_set(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);						 //PB10,PB11 �����	
  
#endif
}


//����IIC��ʼ�ź�
void MPU_IIC_Start(void)
{
#if stm32f103
	MPU_SDA_OUT();     //sda�����
	MPU_IIC_SDA=1;	  	  
	MPU_IIC_SCL=1;	
	MPU_IIC_Delay();
 	MPU_IIC_SDA=0;//START:when CLK is high,DATA change form high to low 	
	MPU_IIC_Delay();
	MPU_IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 	
	
#else
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

	gpio_bit_write(GPIOB, GPIO_PIN_8, SET);
	gpio_bit_write(GPIOB, GPIO_PIN_9, SET);
	MPU_IIC_Delay();

	gpio_bit_write(GPIOB, GPIO_PIN_8, RESET);
	MPU_IIC_Delay();

	gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
	
#endif
}	


//����IICֹͣ�ź�
void MPU_IIC_Stop(void)
{
#if stm32f103
	MPU_SDA_OUT();//sda�����
	MPU_IIC_SCL=0;
	MPU_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high	
	MPU_IIC_Delay();
	MPU_IIC_SCL=1; 
	MPU_IIC_SDA=1;//����I2C���߽����ź�	
	MPU_IIC_Delay();
#else
	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

	gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
	gpio_bit_write(GPIOB, GPIO_PIN_8, RESET);
 	MPU_IIC_Delay();

	gpio_bit_write(GPIOB, GPIO_PIN_9, SET);
	gpio_bit_write(GPIOB, GPIO_PIN_8, SET);
	MPU_IIC_Delay();			
#endif
	
}


//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t MPU_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
#if stm32f103
	
	MPU_SDA_IN();      //SDA����Ϊ���� 
	MPU_IIC_SDA=1;
	MPU_IIC_Delay();
	MPU_IIC_SCL=1;	
	MPU_IIC_Delay();
#else	
	gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

	gpio_bit_write(GPIOB, GPIO_PIN_8, SET);
	MPU_IIC_Delay();	   

	gpio_bit_write(GPIOB, GPIO_PIN_9, SET);
	MPU_IIC_Delay();	

#endif
	
#if stm32f103	
	while(MPU_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			MPU_IIC_Stop();
			return 1;
		}
	}
	MPU_IIC_SCL=0;//ʱ�����0 

#else
	while(gpio_input_bit_get(GPIOB, GPIO_PIN_8))
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			MPU_IIC_Stop();
			return 1;
		}
	} 
	gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
#endif	
	return 0;  

	
} 


//����ACKӦ��
void MPU_IIC_Ack(void)
{
	
#if stm32f103
	MPU_IIC_SCL=0;
	MPU_SDA_OUT();
	MPU_IIC_SDA=0;
	MPU_IIC_Delay();
	MPU_IIC_SCL=1;
	MPU_IIC_Delay();
	MPU_IIC_SCL=0;	
#else
	gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	gpio_bit_write(GPIOB, GPIO_PIN_8, RESET);
	MPU_IIC_Delay();
	gpio_bit_write(GPIOB, GPIO_PIN_9, SET);
	MPU_IIC_Delay();
	gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
#endif
	
}


//������ACKӦ��		    
void MPU_IIC_NAck(void)
{
#if stm32f103
	MPU_IIC_SCL=0;
	MPU_SDA_OUT();
	MPU_IIC_SDA=1;	
	MPU_IIC_Delay();
	MPU_IIC_SCL=1;	
	MPU_IIC_Delay();
	MPU_IIC_SCL=0;	
#else
	gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

	gpio_bit_write(GPIOB, GPIO_PIN_8, SET);
	MPU_IIC_Delay();

	gpio_bit_write(GPIOB, GPIO_PIN_9, SET);
	MPU_IIC_Delay();

	gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
#endif
	
}


//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void MPU_IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	
#if stm32f103
	MPU_SDA_OUT(); 	
    MPU_IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
#else    
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
#endif
	
    for(t=0;t<8;t++)
    {         
#if stm32f013		
		
        MPU_IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 
		MPU_IIC_SCL=1;	
		MPU_IIC_Delay(); 
		MPU_IIC_SCL=0;		
		MPU_IIC_Delay();
#else		
		if(txd&1<<(7-t))
			gpio_bit_write(GPIOB, GPIO_PIN_8, SET);
		else
			gpio_bit_write(GPIOB, GPIO_PIN_8, RESET);
		
		MPU_IIC_Delay(); 

		gpio_bit_write(GPIOB, GPIO_PIN_9, SET);
		MPU_IIC_Delay(); 

		gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
		MPU_IIC_Delay();
#endif			
    }	

} 


//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t MPU_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;

#if stm32f103
//	MPU_SDA_IN();//SDA����Ϊ����
#else
	gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
#endif
    for(i=0;i<8;i++ )
	{
#if stm32f013
        MPU_IIC_SCL=0; 
		MPU_IIC_Delay();
		MPU_IIC_SCL=1;	
		receive<<=1;
		
#else
		gpio_bit_write(GPIOB, GPIO_PIN_9, RESET);
        MPU_IIC_Delay();

		gpio_bit_write(GPIOB, GPIO_PIN_9, SET);
        receive<<=1;
#endif
	
#if smt32f103
		if(MPU_READ_SDA)receive++;  
		MPU_IIC_Delay(); 
#else
        if(gpio_input_bit_get(GPIOB, GPIO_PIN_8))receive++; 
		
		MPU_IIC_Delay(); 
#endif			
    }		

    if (!ack)
        MPU_IIC_NAck();//����nACK
    else
        MPU_IIC_Ack(); //����ACK   
    return receive;
	
}


