#include "BSP.h"

/**
* @brief 配置管脚为推挽输出模式
*/
void SDA_OUT()
{
	GPIO_InitTypeDef GPIO_InitStructure;
    //RCC->APB2ENR|=1<<4;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
* @brief 配置管脚为浮空输入模式
*/
void SDA_IN()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Delay_Ms(u32 cnt)
{
    cnt = cnt * 7500;
    //cnt = cnt * 650;

    while (cnt--);
}

void Delay_Us(u32 cnt)
{
    cnt = cnt * 8;
    //cnt = cnt * 2;
    while (cnt--);
}

//初始化IIC
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //RCC->APB2ENR|=1<<4;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
    Set_IIC_SCL;
    Set_IIC_SDA;
}

//产生IIC起始信号
void IIC_Start(void)
{
    SDA_OUT();      //sda线输出
    Set_IIC_SDA;
    Set_IIC_SCL;
    GPIO_SetBits(GPIOC, GPIO_Pin_9);
    Delay_Us(4);
    GPIO_ResetBits(GPIOC, GPIO_Pin_9);
    Reset_IIC_SDA;//START:when CLK is high,DATA change form high to low
    Delay_Us(4);
    Reset_IIC_SCL;//钳住I2C总线，准备发送或接收数据 
}

//产生IIC停止信号
void IIC_Stop(void)
{
    SDA_OUT();//sda线输出
    Reset_IIC_SCL;
    Reset_IIC_SDA;//STOP:when CLK is high DATA change form low to high
    Delay_Us(4);
    Set_IIC_SCL;
    Set_IIC_SDA;//发送I2C总线结束信号
    Delay_Us(4);
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN();      //SDA设置为输入
    Set_IIC_SDA;
    Delay_Us(1);
    Set_IIC_SCL;
    Delay_Us(1);

    while (READ_SDA)
    {
        ucErrTime++;

        if (ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }

    Reset_IIC_SCL;//时钟输出0 

    return 0;
}

//产生ACK应答
void IIC_Ack(void)
{
    Reset_IIC_SCL;
    SDA_OUT();
    Reset_IIC_SDA;
    Delay_Us(2);
    Set_IIC_SCL;
    Delay_Us(2);
    Reset_IIC_SCL;
}

//不产生ACK应答
void IIC_NAck(void)
{
    Reset_IIC_SCL;
    SDA_OUT();
    Set_IIC_SDA;
    Delay_Us(2);
    Set_IIC_SCL;
    Delay_Us(2);
    Reset_IIC_SCL;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答   
// 0,???
void IIC_Send_Byte(u8 txd)
{
    u8 t;
    SDA_OUT();
    Reset_IIC_SCL;//拉低时钟开始数据传输

    for (t = 0;t < 8;t++)
    {
        Write_SDA((txd&0x80) >> 7);
        txd <<= 1;
        Delay_Us(2);   //对TEA5767这三个延时都是必须的
        Set_IIC_SCL;
        Delay_Us(2);
        Reset_IIC_SCL;
        Delay_Us(2);
    }
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();//SDA设置为输入

    for (i = 0;i < 8;i++)
    {
        Reset_IIC_SCL;
        Delay_Us(2);
        Set_IIC_SCL;
        receive <<= 1;

        if (READ_SDA)receive++;

        Delay_Us(1);
    }

    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK

    return receive;
}



//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址
//返回值  :读到的数据
u8 I2C1_ReadByte(u16 DriverAddr, u16 ReadAddr)
{
  

    u8 temp = 0;
    IIC_Start();

    if (EE_TYPE > AT24C16)//AT24C16
    {
        IIC_Send_Byte(DriverAddr);	   //发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(ReadAddr >> 8);  //发送高地址
        IIC_Wait_Ack();
    }
    else
	{ 
		IIC_Send_Byte(DriverAddr + ((ReadAddr / 256) << 1));        //发送器件地址0XA0,写数据
	}
	
    IIC_Wait_Ack();

    IIC_Send_Byte(ReadAddr % 256);     //发送低地址

    IIC_Wait_Ack();

    IIC_Start();

    IIC_Send_Byte(0XA1);           //进入接收模式

    IIC_Wait_Ack();

    temp = IIC_Read_Byte(0);

    IIC_Stop();//产生一个停止条件

  

    return temp;
}

//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址
//DataToWrite:要写入的数据
void I2C1_WriteByte(uint16_t DriverAddr, u16 WriteAddr, u8 DataToWrite)
{
  
    
/*    if(ProductPara.bFlashWEn == FLASH_WR_DISABLE)
    {
        return;
    }
    */
  

    IIC_Start();

    if (EE_TYPE > AT24C16)
    {
        IIC_Send_Byte(DriverAddr);	    //发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(WriteAddr >> 8);  //发送高地址
    }
    else
    {
        IIC_Send_Byte(DriverAddr + ((WriteAddr / 256) << 1));        //发送器件地址0XA0,写数据
    }

    IIC_Wait_Ack();

    IIC_Send_Byte(WriteAddr % 256);     //发送低地址
    IIC_Wait_Ack();
    IIC_Send_Byte(DataToWrite);     //发送字节
    IIC_Wait_Ack();
    IIC_Stop();//产生一个停止条件
    Delay_Ms(2);

  
}


//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void I2C1_ReadNBytes(u16 DriverAddr, u16 ReadAddr, u16 NumToRead, u8 *pBuffer)
{
    while (NumToRead)
    {
        *pBuffer++ = I2C1_ReadByte(DriverAddr, ReadAddr++);
        NumToRead--;
    }
}

//在AT24CXX里面的指定地址开始读出2个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
uint16_t I2C1_Read2Bytes(u16 DriverAddr, u16 ReadAddr)
{
	u16 NumToRead;
	uint8_t buf[2];

    buf[0] = I2C1_ReadByte(DriverAddr, ReadAddr++);
	buf[1] = I2C1_ReadByte(DriverAddr, ReadAddr++);
	NumToRead = buf[1];
	NumToRead =NumToRead<<8;
	NumToRead |= buf[0];
	return NumToRead;
}

//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void I2C1_WriteNBytes(uint16_t DriverAddr, u16 WriteAddr, u16 NumToWrite, const u8 *pBuffer)
{
    if(ProductPara.bFlashWEn == FLASH_WR_DISABLE)
    {
        return;
    }
    
    while (NumToWrite--)
    {
        I2C1_WriteByte(DriverAddr, WriteAddr, *pBuffer);
        WriteAddr++;
        pBuffer++;
    }
}

void I2C1_WNBytesMul3T(uint16_t DriverAddr, u16 WriteAddr, u16 NumToWrite, const u8 *pBuffer)
{
    if(ProductPara.bFlashWEn == FLASH_WR_DISABLE)
    {
        return;
    }
    
    while (NumToWrite--)
    {
        I2C1_WriteByte(DriverAddr, WriteAddr, *pBuffer);
        I2C1_WriteByte(DriverAddr, WriteAddr + 2 * 0x80, *pBuffer);
        I2C1_WriteByte(DriverAddr, WriteAddr + 4 * 0x80, *pBuffer);
        WriteAddr++;
        pBuffer++;
    }
}

