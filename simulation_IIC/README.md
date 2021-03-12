#                        模拟IIC

####                                                                  -----代码维护人：马灿林




## 基于STM32G070kb开发板IIC协议读写at24c08 E2PROM
说明：Doc文件夹存放的是说明文档，EXP文件夹存的是例程，IIC_Lin文件夹存放IIC模块.c和.h文件。

 IIC的时钟线和数据线可以通过修改以下的宏定义
SCL_RCC_GPIOX_CLK 			//开启IIC SCL管脚GPIOx时钟
SDA_RCC_GPIOX_CLK			//开启IIC SDA管脚GPIOx时钟
IIC_SCL_GPIOX				//选择SCL管脚的GPIOx端口
IIC_SDA_GPIOX				//选择SDA管脚的GPIOx端口
SCL_PINX					//选择SCL管脚
SDA_PINX				//选择SDA管脚

延时函数使用的频率，HclkFrequency默认设置为64M根据使用频率来修改	12M/24M/32M/48M/64M/72M/168M
#define HclkFrequency 	64	

IIC总线速度默认设置为100k,根据使用情况来修改为 100或者400
#define IIC_SPEED		100

在主函数引用 #include "iic.h"既可以使用以下接口函数。
void iic_init(void);				//iic初始化
void iic_start(void);				//起始信号
void iic_stop(void);				//停止信号
uint8_t iic_wait_ack(void);			//等待ACK 1-无效 0-有效
void iic_ack(uint8_t ack);			//产生有效/无效应答 1-无效 0-有效
void iic_send_byte(uint8_t txd);		//发送一个字节
uint8_t iic_read_byte(void);			//接收一个字节

