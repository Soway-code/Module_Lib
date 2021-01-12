/**@file        iic.c
* @brief        iic模块的应用
* @details      iic的通信协议
* @author       马灿林
* @date         2020-08-02
* @version      V1.0.0
* @copyright    2020-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/08/02  <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/

#include "iic.h"

	
#define	SDA_IN HAL_GPIO_ReadPin(IIC_SDA_GPIOX,SDA_PINX) 

/**@brief   延时函数
* - 根据频率宏定义和iic速度来选择一个合适的延时
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

/**@brief   初始化iic
*/
void iic_init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
    SCL_RCC_GPIOX_CLK;									//开启GPIO时钟
	SDA_RCC_GPIOX_CLK;
	
    GPIO_Initure.Pin=SCL_PINX;     		
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  			//推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          			//上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     	//高速
    HAL_GPIO_Init(IIC_SCL_GPIOX,&GPIO_Initure);    		//初始化
	
	GPIO_Initure.Pin=SDA_PINX;     		
    HAL_GPIO_Init(IIC_SDA_GPIOX,&GPIO_Initure);     	//初始化

}

/**
* @brief 配置sda管脚为推挽输出模式
*/
void iic_sda_out()
{
	GPIO_InitTypeDef GPIO_Initure;
	
	GPIO_Initure.Pin=SDA_PINX;     		
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  			//推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          			//上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     	//高速
    HAL_GPIO_Init(IIC_SCL_GPIOX,&GPIO_Initure);     	//初始化
}

/**
* @brief 配置sda管脚为输入模式
*/
void iic_sda_in()
{
	GPIO_InitTypeDef GPIO_Initure;
	
	GPIO_Initure.Pin=SDA_PINX;     		
    GPIO_Initure.Mode=GPIO_MODE_INPUT;  				//输入
    GPIO_Initure.Pull=GPIO_PULLUP;          			//上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     	//高速
    HAL_GPIO_Init(IIC_SCL_GPIOX,&GPIO_Initure);     	//初始化
}



/**
* @brief iic起始信号
*/
void iic_start(void)
{
	//数据线设置为输出
	iic_sda_out();
	
	//总线空闲
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
	HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_SET);
	Delay_iic_us();
	
	//拉低数据线
	HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	//SCL拉低钳住总线
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
}

/**
* @brief iic停止信号
*/
void iic_stop(void)
{
	iic_sda_out();			//数据线设置为输出
	
	//SCL拉低钳住总线
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	//拉高时钟线
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
	Delay_iic_us();
	
	//数据线由低向高跳变
	HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_SET);
}

/**@brief       等待ACK 
* @return       函数执行结果
* - 返回结果    : 1-ACK无效 0-ACK有效
*/
uint8_t iic_wait_ack(void)
{
	uint8_t ack = 0;
	
	iic_sda_in();				//数据线设置为输入
	
	//拉高时钟线,让从设备可以控制数据线
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
	Delay_iic_us();
	
								//获取数据线的电平
	if(SDA_IN){					//无效应答
		ack = 1;
		iic_stop();
	}
	else{
		ack = 0;
	}
	
								//SCL拉低钳住总线
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	return ack;
}

/**@brief       产生有效/无效应答
* @param[in]    ack : 1-无效 0-有效
*/
void iic_ack(uint8_t ack)
{
	iic_sda_out();				//数据线设置为输出
	
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	//发送ACK/NACK
	if(ack)
		HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_SET);//无效应答
	else
		HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_RESET);;//有效应答
	
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
	
	Delay_iic_us();				//保持数据稳定
	
	//SCL拉低钳住总线
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
}

/**@brief       发送一个字节
* @param[in]    txd : 要写入的字节
*/
void iic_send_byte(uint8_t txd)
{
	uint8_t i;
	
	iic_sda_out();			//数据线设置为输出
	
	HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
	Delay_iic_us();
	
	for(i=0;i<8;i++){
		if(txd&1<<(7-i))
			HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(IIC_SDA_GPIOX, SDA_PINX,GPIO_PIN_RESET);
		
		Delay_iic_us();
		//拉高时钟线
		HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_SET);
		Delay_iic_us();
		
		HAL_GPIO_WritePin(IIC_SCL_GPIOX, SCL_PINX,GPIO_PIN_RESET);
		Delay_iic_us();
	}
}

/**@brief       接收一个字节
* @return       函数执行结果
* - 返回结果    : 返回一个字节uint8_t类型的数据
*/
uint8_t iic_read_byte(void)
{
	uint8_t i,rxd = 0;
	
	iic_sda_in();			//数据线设置为输入
	
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

