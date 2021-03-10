#include "BSP.h"

/**
* @brief ���ùܽ�Ϊ�������ģʽ
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
* @brief ���ùܽ�Ϊ��������ģʽ
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

//��ʼ��IIC
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

//����IIC��ʼ�ź�
void IIC_Start(void)
{
    SDA_OUT();      //sda�����
    Set_IIC_SDA;
    Set_IIC_SCL;
    GPIO_SetBits(GPIOC, GPIO_Pin_9);
    Delay_Us(4);
    GPIO_ResetBits(GPIOC, GPIO_Pin_9);
    Reset_IIC_SDA;//START:when CLK is high,DATA change form high to low
    Delay_Us(4);
    Reset_IIC_SCL;//ǯסI2C���ߣ�׼�����ͻ�������� 
}

//����IICֹͣ�ź�
void IIC_Stop(void)
{
    SDA_OUT();//sda�����
    Reset_IIC_SCL;
    Reset_IIC_SDA;//STOP:when CLK is high DATA change form low to high
    Delay_Us(4);
    Set_IIC_SCL;
    Set_IIC_SDA;//����I2C���߽����ź�
    Delay_Us(4);
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN();      //SDA����Ϊ����
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

    Reset_IIC_SCL;//ʱ�����0 

    return 0;
}

//����ACKӦ��
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

//������ACKӦ��
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

//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��   
// 0,???
void IIC_Send_Byte(u8 txd)
{
    u8 t;
    SDA_OUT();
    Reset_IIC_SCL;//����ʱ�ӿ�ʼ���ݴ���

    for (t = 0;t < 8;t++)
    {
        Write_SDA((txd&0x80) >> 7);
        txd <<= 1;
        Delay_Us(2);   //��TEA5767��������ʱ���Ǳ����
        Set_IIC_SCL;
        Delay_Us(2);
        Reset_IIC_SCL;
        Delay_Us(2);
    }
}

//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();//SDA����Ϊ����

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
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK

    return receive;
}



//��AT24CXXָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ
//����ֵ  :����������
u8 I2C1_ReadByte(u16 DriverAddr, u16 ReadAddr)
{
  

    u8 temp = 0;
    IIC_Start();

    if (EE_TYPE > AT24C16)//AT24C16
    {
        IIC_Send_Byte(DriverAddr);	   //����д����
        IIC_Wait_Ack();
        IIC_Send_Byte(ReadAddr >> 8);  //���͸ߵ�ַ
        IIC_Wait_Ack();
    }
    else
	{ 
		IIC_Send_Byte(DriverAddr + ((ReadAddr / 256) << 1));        //����������ַ0XA0,д����
	}
	
    IIC_Wait_Ack();

    IIC_Send_Byte(ReadAddr % 256);     //���͵͵�ַ

    IIC_Wait_Ack();

    IIC_Start();

    IIC_Send_Byte(0XA1);           //�������ģʽ

    IIC_Wait_Ack();

    temp = IIC_Read_Byte(0);

    IIC_Stop();//����һ��ֹͣ����

  

    return temp;
}

//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ
//DataToWrite:Ҫд�������
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
        IIC_Send_Byte(DriverAddr);	    //����д����
        IIC_Wait_Ack();
        IIC_Send_Byte(WriteAddr >> 8);  //���͸ߵ�ַ
    }
    else
    {
        IIC_Send_Byte(DriverAddr + ((WriteAddr / 256) << 1));        //����������ַ0XA0,д����
    }

    IIC_Wait_Ack();

    IIC_Send_Byte(WriteAddr % 256);     //���͵͵�ַ
    IIC_Wait_Ack();
    IIC_Send_Byte(DataToWrite);     //�����ֽ�
    IIC_Wait_Ack();
    IIC_Stop();//����һ��ֹͣ����
    Delay_Ms(2);

  
}


//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void I2C1_ReadNBytes(u16 DriverAddr, u16 ReadAddr, u16 NumToRead, u8 *pBuffer)
{
    while (NumToRead)
    {
        *pBuffer++ = I2C1_ReadByte(DriverAddr, ReadAddr++);
        NumToRead--;
    }
}

//��AT24CXX�����ָ����ַ��ʼ����2����������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
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

//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
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

