#include "ADXL345.h"

unsigned char	ADXL345_BUF[6];                         //接收数据缓存区   

//设置数据线的输入输出模式
void iic_sda_mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = mode;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;//速度
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;//PB8
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}

//初始化IIC
void ADXL345_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //RCC->APB2ENR|=1<<4;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    Set_SCL;
    Set_SDA;
}


void Delay5us()
{
     uint8_t cnt = 5 * 8;
    //cnt = cnt * 2;
    while (cnt--);
}


void Delay5ms()
{
	uint32_t cnt = 5* 7500;
    //cnt = cnt * 650;

    while (cnt--);
}

//起始信号
void iic_start(void)
{
	//数据线设置为输出
	iic_sda_mode(GPIO_Mode_Out_PP);
	
	//总线空闲
	Set_SCL;
	//SCL = 1;
	Set_SDA;
	//SDA_OUT = 1;
	Delay5us();
	
	//拉低数据线
	Reset_SDA;
	//SDA_OUT = 0;
	Delay5us();
	
	//SCL拉低钳住总线
	Reset_SCL;
	//SCL = 0;
}

//停止信号
void iic_stop(void)
{
	//数据线设置为输出
	iic_sda_mode(GPIO_Mode_Out_PP);
	
	//SCL拉低钳住总线
	Reset_SCL;
	//SCL = 0;
	Reset_SDA;
	//SDA_OUT = 0;
	Delay5us();
	
	//拉高时钟线
	Set_SCL;
	//SCL = 1;
	Delay5us();
	
	//数据线由低向高跳变
	Set_SDA;
	//SDA_OUT = 1;
}

//等待ACK 1-无效 0-有效
u8 iic_wait_ack(void)
{
	u8 ack = 0;
	
	//数据线设置为输入
	iic_sda_mode(GPIO_Mode_IPU);
	
	//拉高时钟线,让从设备可以控制数据线
	Set_SCL;
	//SCL = 1;
	Delay5us();
	
	//获取数据线的电平
	if(DataBit_SDA){//无效应答
		ack = 1;
		iic_stop();
	}
	else{
		ack = 0;
	}
	
	//SCL拉低钳住总线
	Reset_SCL;
	//SCL = 0;
	Delay5us();
	
	return ack;
}

//产生有效/无效应答 1-无效 0-有效
void iic_ack(u8 ack)
{
	//数据线设置为输出
	iic_sda_mode(GPIO_Mode_Out_PP);
	Reset_SCL;
	//SCL = 0;
	Delay5us();
	
	//发送ACK/NACK
	if(ack)
		Set_SDA;
		//SDA_OUT = 1;//无效应答
	else
		Reset_SDA;
		//SDA_OUT = 0;//有效应答
	
	Delay5us();
	Set_SCL;
	//SCL = 1;
	//保持数据稳定
	Delay5us();
	//SCL拉低钳住总线
	Set_SCL;
	//SCL = 0;
}

//发送一个字节
void iic_send_byte(u8 txd)
{
	u8 i;
	//数据线设置为输出
	iic_sda_mode(GPIO_Mode_Out_PP);
	
	Reset_SCL;
	//SCL = 0;
	Delay5us();
	
	for(i=0;i<8;i++){
		if(txd&1<<(7-i))
			Set_SDA;
		//SDA_OUT = 1;
		else
			Reset_SDA;
			//SDA_OUT = 0;
		
		Delay5us();
		//拉高时钟线
		Set_SCL;
		//SCL = 1;
		Delay5us();
		
		Reset_SCL;
		//SCL = 0;
		Delay5us();
	}
	iic_wait_ack();
}

//接收一个字节
u8 iic_read_byte(void)
{
	 u8 i;
    u8 dat = 0;
	iic_sda_mode(GPIO_Mode_IPU);
    Set_SDA;                  //使能内部上拉,准备读取数据,
    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;
        Set_SCL ;              //拉高时钟线
        Delay5us();             //延时
        dat |= DataBit_SDA;             //读数据               
       Reset_SCL;                //拉低时钟线
        Delay5us();             //延时
    }
    return dat;
//	u8 i,rxd = 0;
//	//数据线设置为输入
//	iic_sda_mode(GPIO_Mode_IPU);
//	
//	Reset_SCL;
//	//SCL = 0;
//	Delay5us();
//	
//	for(i=0;i<8;i++){
//		Set_SCL;
//		//SCL = 1;
//		Delay5us();
//		
//		if(DataBit_SDA)
//			rxd |= 1<<(7-i);

//		Delay5us();
//		Reset_SCL;
//		//SCL = 0;
//		Delay5us();
//	}
//	
//	return rxd;
}

//******单字节写入*******************************************

void Single_Write_ADXL345(uint8_t REG_Address,uint8_t REG_data)
{
    iic_start();                  //起始信号
    iic_send_byte(SlaveAddress);   //发送设备地址+写信号
    iic_send_byte(REG_Address);    //内部寄存器地址，请参考中文pdf22页 
    iic_send_byte(REG_data);       //内部寄存器数据，请参考中文pdf22页 
    iic_stop();                   //发送停止信号
}

//********单字节读取*****************************************
uint8_t Single_Read_ADXL345(uint8_t REG_Address)
{  
	uint8_t REG_data;
    iic_start();                           		//起始信号
    iic_send_byte(SlaveAddress);           //发送设备地址+写信号
    iic_send_byte(REG_Address);            //发送存储单元地址，从0开始	
    iic_start();                        	  //起始信号
    iic_send_byte(SlaveAddress+1);         //发送设备地址+读信号
    REG_data=iic_read_byte();              //读出寄存器数据
	iic_ack(1);   
	iic_stop();                           //停止信号
    return REG_data; 
}

//*********************************************************
//
//连续读出ADXL345内部加速度数据，地址范围0x32~0x37
//
//*********************************************************
void Multiple_read_ADXL345(void)
{   
	uint8_t i;
    iic_start();                          //起始信号
    iic_send_byte(SlaveAddress);           //发送设备地址+写信号
    iic_send_byte(0x32);                   //发送存储单元地址，从0x32开始	
    iic_start();                          //起始信号
    iic_send_byte(SlaveAddress+1);         //发送设备地址+读信号
	 for (i=0; i<6; i++)                      //连续读取6个地址数据，存储中BUF
    {
        ADXL345_BUF[i] = iic_read_byte();          //BUF[0]存储0x32地址中的数据
		if (i == 5)
        {
           iic_ack(0);                //最后一个数据需要回NOACK
        }
        else
        {
          iic_ack(0);                //回应ACK
       }
		
   }
    iic_stop();                          //停止信号
    Delay5ms();
}

void Multiple_read1_ADXL345(void)
{
    iic_start();                          //起始信号
    iic_send_byte(SlaveAddress);           //发送设备地址+写信号
    iic_send_byte(0x32);                   //发送存储单元地址，从0x32开始	
    iic_start();                          	//起始信号
    iic_send_byte(SlaveAddress+1);         //发送设备地址+读信号
	ADXL345_BUF[0] = iic_read_byte();       //BUF[0]存储0x32地址中的数据
	iic_ack(1); 
	iic_stop();                          	//停止信号
    Delay5us(); 
	iic_start();                          //起始信号
    iic_send_byte(SlaveAddress);           //发送设备地址+写信号
    iic_send_byte(0x33);                   //发送存储单元地址，从0x32开始	
    iic_start();                          	//起始信号
    iic_send_byte(SlaveAddress+1);         //发送设备地址+读信号
	ADXL345_BUF[1] = iic_read_byte();       //BUF[0]存储0x32地址中的数据
    ADXL345_BUF[1] &= 0x01;
	iic_ack(1); 
	iic_stop();                          	//停止信号
    Delay5us();
	iic_start();                          //起始信号
    iic_send_byte(SlaveAddress);           //发送设备地址+写信号
    iic_send_byte(0x34);                   //发送存储单元地址，从0x32开始	
    iic_start();                          	//起始信号
    iic_send_byte(SlaveAddress+1);         //发送设备地址+读信号
	ADXL345_BUF[2] = iic_read_byte();       //BUF[0]存储0x32地址中的数据
	iic_ack(1); 
	iic_stop();                          	//停止信号
    Delay5us(); 
	iic_start();                          //起始信号
    iic_send_byte(SlaveAddress);           //发送设备地址+写信号
    iic_send_byte(0x35);                   //发送存储单元地址，从0x32开始	
    iic_start();                          	//起始信号
    iic_send_byte(SlaveAddress+1);         //发送设备地址+读信号
	ADXL345_BUF[3] = iic_read_byte();       //BUF[0]存储0x32地址中的数据
    ADXL345_BUF[3] &= 0x01;
	iic_ack(1); 
	iic_stop();                          	//停止信号
    Delay5us(); 
	iic_start();                          //起始信号
    iic_send_byte(SlaveAddress);           //发送设备地址+写信号
    iic_send_byte(0x36);                   //发送存储单元地址，从0x32开始	
    iic_start();                          	//起始信号
    iic_send_byte(SlaveAddress+1);         //发送设备地址+读信号
	ADXL345_BUF[4] = iic_read_byte();       //BUF[0]存储0x32地址中的数据
	iic_ack(1); 
	iic_stop();                          	//停止信号
    Delay5us(); 
	iic_start();                          //起始信号
    iic_send_byte(SlaveAddress);           //发送设备地址+写信号
    iic_send_byte(0x37);                   //发送存储单元地址，从0x32开始	
    iic_start();                          	//起始信号
    iic_send_byte(SlaveAddress+1);         //发送设备地址+读信号
    ADXL345_BUF[5] = iic_read_byte();       //BUF[0]存储0x32地址中的数据
   ADXL345_BUF[5] &= 0x01;
	iic_ack(1); 
	iic_stop();                          	//停止信号        	
}
//*****************************************************************

//初始化ADXL345，根据需要请参考pdf进行修改************************
void Init_ADXL345()
{
   Single_Write_ADXL345(0x31,0x0B);   //测量范围,正负16g，13位模式
   Single_Write_ADXL345(0x2C,0x08);   //速率设定为12.5 参考pdf13页
   Single_Write_ADXL345(0x2D,0x08);   //选择电源模式   参考pdf24页
   Single_Write_ADXL345(0x2E,0x80);   //使能 DATA_READY 中断
   Single_Write_ADXL345(0x1E,0x00);   //X 偏移量 根据测试传感器的状态写入pdf29页
   Single_Write_ADXL345(0x1F,0x00);   //Y 偏移量 根据测试传感器的状态写入pdf29页
   Single_Write_ADXL345(0x20,0x05);   //Z 偏移量 根据测试传感器的状态写入pdf29页
}
