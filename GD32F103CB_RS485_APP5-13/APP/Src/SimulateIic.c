#include "SimulateIic.h"


 //MPU IIC 延时函数
void MPU_IIC_Delay(void)
{
	uint8_t i=30;
	while(i--) ;
}


//初始化IIC
void MPU_IIC_Init(void)
{					   
#if stm32f103	
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//先使能外设IO PORTB时钟 
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;	 // 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIO 

	GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);						 //PB10,PB11 输出高	
#else
	rcu_periph_clock_enable(RCU_GPIOB);
	
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9);
	gpio_bit_set(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);						 //PB10,PB11 输出高	
  
#endif
}


//产生IIC起始信号
void MPU_IIC_Start(void)
{
#if stm32f103
	MPU_SDA_OUT();     //sda线输出
	MPU_IIC_SDA=1;	  	  
	MPU_IIC_SCL=1;	
	MPU_IIC_Delay();
 	MPU_IIC_SDA=0;//START:when CLK is high,DATA change form high to low 	
	MPU_IIC_Delay();
	MPU_IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 	
	
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


//产生IIC停止信号
void MPU_IIC_Stop(void)
{
#if stm32f103
	MPU_SDA_OUT();//sda线输出
	MPU_IIC_SCL=0;
	MPU_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high	
	MPU_IIC_Delay();
	MPU_IIC_SCL=1; 
	MPU_IIC_SDA=1;//发送I2C总线结束信号	
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


//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t MPU_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
#if stm32f103
	
	MPU_SDA_IN();      //SDA设置为输入 
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
	MPU_IIC_SCL=0;//时钟输出0 

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


//产生ACK应答
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


//不产生ACK应答		    
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


//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void MPU_IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	
#if stm32f103
	MPU_SDA_OUT(); 	
    MPU_IIC_SCL=0;//拉低时钟开始数据传输
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


//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t MPU_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;

#if stm32f103
//	MPU_SDA_IN();//SDA设置为输入
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
        MPU_IIC_NAck();//发送nACK
    else
        MPU_IIC_Ack(); //发送ACK   
    return receive;
	
}


