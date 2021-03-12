#include "ADXL345.h"

unsigned char	ADXL345_BUF[6];                         //�������ݻ�����   

//���������ߵ��������ģʽ
void iic_sda_mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = mode;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;//PB8
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}

//��ʼ��IIC
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

//��ʼ�ź�
void iic_start(void)
{
	//����������Ϊ���
	iic_sda_mode(GPIO_Mode_Out_PP);
	
	//���߿���
	Set_SCL;
	//SCL = 1;
	Set_SDA;
	//SDA_OUT = 1;
	Delay5us();
	
	//����������
	Reset_SDA;
	//SDA_OUT = 0;
	Delay5us();
	
	//SCL����ǯס����
	Reset_SCL;
	//SCL = 0;
}

//ֹͣ�ź�
void iic_stop(void)
{
	//����������Ϊ���
	iic_sda_mode(GPIO_Mode_Out_PP);
	
	//SCL����ǯס����
	Reset_SCL;
	//SCL = 0;
	Reset_SDA;
	//SDA_OUT = 0;
	Delay5us();
	
	//����ʱ����
	Set_SCL;
	//SCL = 1;
	Delay5us();
	
	//�������ɵ��������
	Set_SDA;
	//SDA_OUT = 1;
}

//�ȴ�ACK 1-��Ч 0-��Ч
u8 iic_wait_ack(void)
{
	u8 ack = 0;
	
	//����������Ϊ����
	iic_sda_mode(GPIO_Mode_IPU);
	
	//����ʱ����,�ô��豸���Կ���������
	Set_SCL;
	//SCL = 1;
	Delay5us();
	
	//��ȡ�����ߵĵ�ƽ
	if(DataBit_SDA){//��ЧӦ��
		ack = 1;
		iic_stop();
	}
	else{
		ack = 0;
	}
	
	//SCL����ǯס����
	Reset_SCL;
	//SCL = 0;
	Delay5us();
	
	return ack;
}

//������Ч/��ЧӦ�� 1-��Ч 0-��Ч
void iic_ack(u8 ack)
{
	//����������Ϊ���
	iic_sda_mode(GPIO_Mode_Out_PP);
	Reset_SCL;
	//SCL = 0;
	Delay5us();
	
	//����ACK/NACK
	if(ack)
		Set_SDA;
		//SDA_OUT = 1;//��ЧӦ��
	else
		Reset_SDA;
		//SDA_OUT = 0;//��ЧӦ��
	
	Delay5us();
	Set_SCL;
	//SCL = 1;
	//���������ȶ�
	Delay5us();
	//SCL����ǯס����
	Set_SCL;
	//SCL = 0;
}

//����һ���ֽ�
void iic_send_byte(u8 txd)
{
	u8 i;
	//����������Ϊ���
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
		//����ʱ����
		Set_SCL;
		//SCL = 1;
		Delay5us();
		
		Reset_SCL;
		//SCL = 0;
		Delay5us();
	}
	iic_wait_ack();
}

//����һ���ֽ�
u8 iic_read_byte(void)
{
	 u8 i;
    u8 dat = 0;
	iic_sda_mode(GPIO_Mode_IPU);
    Set_SDA;                  //ʹ���ڲ�����,׼����ȡ����,
    for (i=0; i<8; i++)         //8λ������
    {
        dat <<= 1;
        Set_SCL ;              //����ʱ����
        Delay5us();             //��ʱ
        dat |= DataBit_SDA;             //������               
       Reset_SCL;                //����ʱ����
        Delay5us();             //��ʱ
    }
    return dat;
//	u8 i,rxd = 0;
//	//����������Ϊ����
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

//******���ֽ�д��*******************************************

void Single_Write_ADXL345(uint8_t REG_Address,uint8_t REG_data)
{
    iic_start();                  //��ʼ�ź�
    iic_send_byte(SlaveAddress);   //�����豸��ַ+д�ź�
    iic_send_byte(REG_Address);    //�ڲ��Ĵ�����ַ����ο�����pdf22ҳ 
    iic_send_byte(REG_data);       //�ڲ��Ĵ������ݣ���ο�����pdf22ҳ 
    iic_stop();                   //����ֹͣ�ź�
}

//********���ֽڶ�ȡ*****************************************
uint8_t Single_Read_ADXL345(uint8_t REG_Address)
{  
	uint8_t REG_data;
    iic_start();                           		//��ʼ�ź�
    iic_send_byte(SlaveAddress);           //�����豸��ַ+д�ź�
    iic_send_byte(REG_Address);            //���ʹ洢��Ԫ��ַ����0��ʼ	
    iic_start();                        	  //��ʼ�ź�
    iic_send_byte(SlaveAddress+1);         //�����豸��ַ+���ź�
    REG_data=iic_read_byte();              //�����Ĵ�������
	iic_ack(1);   
	iic_stop();                           //ֹͣ�ź�
    return REG_data; 
}

//*********************************************************
//
//��������ADXL345�ڲ����ٶ����ݣ���ַ��Χ0x32~0x37
//
//*********************************************************
void Multiple_read_ADXL345(void)
{   
	uint8_t i;
    iic_start();                          //��ʼ�ź�
    iic_send_byte(SlaveAddress);           //�����豸��ַ+д�ź�
    iic_send_byte(0x32);                   //���ʹ洢��Ԫ��ַ����0x32��ʼ	
    iic_start();                          //��ʼ�ź�
    iic_send_byte(SlaveAddress+1);         //�����豸��ַ+���ź�
	 for (i=0; i<6; i++)                      //������ȡ6����ַ���ݣ��洢��BUF
    {
        ADXL345_BUF[i] = iic_read_byte();          //BUF[0]�洢0x32��ַ�е�����
		if (i == 5)
        {
           iic_ack(0);                //���һ��������Ҫ��NOACK
        }
        else
        {
          iic_ack(0);                //��ӦACK
       }
		
   }
    iic_stop();                          //ֹͣ�ź�
    Delay5ms();
}

void Multiple_read1_ADXL345(void)
{
    iic_start();                          //��ʼ�ź�
    iic_send_byte(SlaveAddress);           //�����豸��ַ+д�ź�
    iic_send_byte(0x32);                   //���ʹ洢��Ԫ��ַ����0x32��ʼ	
    iic_start();                          	//��ʼ�ź�
    iic_send_byte(SlaveAddress+1);         //�����豸��ַ+���ź�
	ADXL345_BUF[0] = iic_read_byte();       //BUF[0]�洢0x32��ַ�е�����
	iic_ack(1); 
	iic_stop();                          	//ֹͣ�ź�
    Delay5us(); 
	iic_start();                          //��ʼ�ź�
    iic_send_byte(SlaveAddress);           //�����豸��ַ+д�ź�
    iic_send_byte(0x33);                   //���ʹ洢��Ԫ��ַ����0x32��ʼ	
    iic_start();                          	//��ʼ�ź�
    iic_send_byte(SlaveAddress+1);         //�����豸��ַ+���ź�
	ADXL345_BUF[1] = iic_read_byte();       //BUF[0]�洢0x32��ַ�е�����
    ADXL345_BUF[1] &= 0x01;
	iic_ack(1); 
	iic_stop();                          	//ֹͣ�ź�
    Delay5us();
	iic_start();                          //��ʼ�ź�
    iic_send_byte(SlaveAddress);           //�����豸��ַ+д�ź�
    iic_send_byte(0x34);                   //���ʹ洢��Ԫ��ַ����0x32��ʼ	
    iic_start();                          	//��ʼ�ź�
    iic_send_byte(SlaveAddress+1);         //�����豸��ַ+���ź�
	ADXL345_BUF[2] = iic_read_byte();       //BUF[0]�洢0x32��ַ�е�����
	iic_ack(1); 
	iic_stop();                          	//ֹͣ�ź�
    Delay5us(); 
	iic_start();                          //��ʼ�ź�
    iic_send_byte(SlaveAddress);           //�����豸��ַ+д�ź�
    iic_send_byte(0x35);                   //���ʹ洢��Ԫ��ַ����0x32��ʼ	
    iic_start();                          	//��ʼ�ź�
    iic_send_byte(SlaveAddress+1);         //�����豸��ַ+���ź�
	ADXL345_BUF[3] = iic_read_byte();       //BUF[0]�洢0x32��ַ�е�����
    ADXL345_BUF[3] &= 0x01;
	iic_ack(1); 
	iic_stop();                          	//ֹͣ�ź�
    Delay5us(); 
	iic_start();                          //��ʼ�ź�
    iic_send_byte(SlaveAddress);           //�����豸��ַ+д�ź�
    iic_send_byte(0x36);                   //���ʹ洢��Ԫ��ַ����0x32��ʼ	
    iic_start();                          	//��ʼ�ź�
    iic_send_byte(SlaveAddress+1);         //�����豸��ַ+���ź�
	ADXL345_BUF[4] = iic_read_byte();       //BUF[0]�洢0x32��ַ�е�����
	iic_ack(1); 
	iic_stop();                          	//ֹͣ�ź�
    Delay5us(); 
	iic_start();                          //��ʼ�ź�
    iic_send_byte(SlaveAddress);           //�����豸��ַ+д�ź�
    iic_send_byte(0x37);                   //���ʹ洢��Ԫ��ַ����0x32��ʼ	
    iic_start();                          	//��ʼ�ź�
    iic_send_byte(SlaveAddress+1);         //�����豸��ַ+���ź�
    ADXL345_BUF[5] = iic_read_byte();       //BUF[0]�洢0x32��ַ�е�����
   ADXL345_BUF[5] &= 0x01;
	iic_ack(1); 
	iic_stop();                          	//ֹͣ�ź�        	
}
//*****************************************************************

//��ʼ��ADXL345��������Ҫ��ο�pdf�����޸�************************
void Init_ADXL345()
{
   Single_Write_ADXL345(0x31,0x0B);   //������Χ,����16g��13λģʽ
   Single_Write_ADXL345(0x2C,0x08);   //�����趨Ϊ12.5 �ο�pdf13ҳ
   Single_Write_ADXL345(0x2D,0x08);   //ѡ���Դģʽ   �ο�pdf24ҳ
   Single_Write_ADXL345(0x2E,0x80);   //ʹ�� DATA_READY �ж�
   Single_Write_ADXL345(0x1E,0x00);   //X ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
   Single_Write_ADXL345(0x1F,0x00);   //Y ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
   Single_Write_ADXL345(0x20,0x05);   //Z ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
}
