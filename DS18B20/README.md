#                         DS18B20

####                                                                  -----代码维护人：马灿林




## 基于STM32G070/STM32F103RC/STM32F407ZG开发板ds18b20温度 
说明：Doc文件夹存放的是说明文档，EXP文件夹存的是例程，ds18b20_Lin/DS18B20文件夹存放ds18b20模块.c和.h文件。

通过以下三个宏定义来修改连接DS18B20的DQ管脚
#define RCC_GPIOX_CLK 		__HAL_RCC_GPIOA_CLK_ENABLE();	//开启GPIOx时钟
#define DS18B20_GPIOX 		GPIOA				//选择DS18B20 GPIOx端口
#define DS18B20_GPIO_PINX 		GPIO_PIN_15			//选择DS18B20 连接的管脚

通过修改USE_TIMER_X宏定义来决定使用定时器3或者定时器14
#define USE_TIMER_X     3

需要添加定时器库文件stm32(xx)xx_hal_tim.c和stm32(xx)xx_hal_tim_ex.c
需要在stm32(xx)xx_hal_conf.h文件里把 #define HAL_TIM_MODULE_ENABLED 定时器这个宏定义注释去掉，
在主函数引用 #include "ds18b20.h"既可以使用以下接口函数。

1.DS18B20初始化函数，该函数有三个返回值分别是0x10,1,0,返回0x10定时器频率匹配失败，返回1 DS18B20不存在, 返回0 DS18B20存在初始化成功。
uint8_t DS18B20_Init(void);
    
2.DS18B20获取温度值的函数，该函数返回值是温度值的十倍 (24.5℃时返回245)，
short DS18B20_Get_Temp(void);


3.定时器延时函数，该函数的没有返回值，输入参数是一个32位无符号类型的参数
void delay_tim_ms(uint32_t ms);
void delay_tim_s(uint32_t s);


