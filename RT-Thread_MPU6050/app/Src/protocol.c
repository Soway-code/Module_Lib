/**@file        protocol.c
* @brief        (����)���ز���������_��λ��_Э��
* @details      ���ݰ����ݴ���
* @author      	�����
* @date         2021-1-8
* @version      V1.0.0
* @copyright    2021-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/1/8    <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/
#include "protocol.h"
#include "bsp_usart.h"
#include "adc.h"
#include "Flash_app.h"
#include "mpu6050.h"
#include "DataTreating.h"

extern uint16_t ADC0_filter; //�����ⲿ����
extern double X_Angle,Y_Angle,Z_Angle;		//���ٶȴ���������
uint16_t NullCalibrat;					//����У׼
uint16_t FullCalibrat;					//����У׼
uint8_t ID[3] ={0x30,0x30,0x30};		//�豸ID
uint8_t Threshold=0;					//��������ֵ
uint8_t DampTime= 0;					//����ʱ��	
uint16_t Null_Ad = 0,Reload_Ad = 0,Overload_Ad = 0;		//�������� AD,�������� AD,�������� AD ����ֵ;
uint8_t D01_sign[2]={0,0};			//������־λ-�����ر�־λ
uint8_t TX_buf[200];					//�������ݻ���
uint16_t ton = 0;				//����

/**@brief       ʮ������ת��ASCII
* @param[in]    hex_byte : ��Ҫת����ʮ������;
* @return       ����ִ�н�� ascii
* @note     
*/
char HexToAscii(char hex_byte)
{
    char result;
    if((hex_byte>=0)&&(hex_byte<=9))            //���ascii����
        result = hex_byte + 0x30;
    else if((hex_byte >= 10)&&(hex_byte <= 15)) //���ascii��д��ĸ
        result = hex_byte + 0x37;
    else
        result = 0xff;
    return result;
}

/**@brief       ASCIIת��ʮ����
* @param[in]    hex_byte : ��Ҫת����ASCII;
* @return       ����ִ�н�� ʮ����
* @note     
*/
uint8_t AsciiToDecimalism(char byte)
{
 	if((byte <= 57) && (byte >= 48))
		return (byte - 48);
	else if((byte <= 90) && (byte >= 65))
		return (byte - 55);
	else if((byte <= 122) && (byte >= 97))
		return (byte - 87);
	return 0;
}

/**@brief       �������ݺ�У����֤
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @return       ����ִ�н��
* - 1(У������ȷ)
* - 0(У�������)
* @note     
*/
uint8_t Verification(const char *RX)
{
	uint16_t sum = 0;
	uint16_t i = 0;
	while((RX[i+2] != 0x0D) && (i < 1024))
	{
		sum += RX[i];
		i++;
	}
	sum &= 0x00FF; 
	
	if(((HexToAscii((sum/0x10))) == RX[i++]) && ((HexToAscii((sum%0x10))) == RX[i]))
		return 1;
	return 0;
}

/**@brief       ID������֤
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @return       ����ִ�н��
* - 1(ID����ȷ)
* - 0(ID�Ŵ���)
* @note     
*/
uint8_t IDVerification(const char *RX)
{
	if((ID[0] == RX[7]) && (ID[1] == RX[8]) && (ID[2] == RX[9]))
		return 1;
	return 0;
}

/**@brief       У�������
* @param[in]  	length�����ݰ��ĳ��ȣ�������У����
* @note     	�˺���������õ�У������ӵ�����*TX
*/
void CheckCode(uint8_t length)
{
	uint16_t sum = 0;
	uint8_t i = 0;
	while(length--)
	{
		sum += TX_buf[i];
		i++;
	}
	sum &= 0x00FF;
	TX_buf[i++] = (HexToAscii((sum/0x10)));
	TX_buf[i] = (HexToAscii((sum%0x10)));
}

void Parameter_Init()		//��flash��������
{
	if(Flash_Read_OneByte(FIRST) != 0xa5)	//�״��ϵ�
	{
		//�������Ƕ�ƽ����������ֵ������оƬ�̶���
		Flash_Write_fourByte(X_ACCEL_ADDR,270);
		Flash_Write_fourByte(Y_ACCEL_ADDR,660);
		Flash_Write_fourByte(Z_ACCEL_ADDR,-19422);
		
		Flash_Write_MultiBytes(ID_ADDR,ID,3);	//д��Ĭ��ID=000
		
		Flash_Write_OneByte(VPT_ADDR,15);		//��ֵĬ��15%
		
		Flash_Write_OneByte(DAMPTE,12);			//����ʱ��Ĭ��12*5=60s	
		
		//Ĭ�����ر���ֵ
		Flash_Write_fourByte(NULL_AD_ADDR,100);
		Flash_Write_fourByte(RELOAD_AD_ADDR,3000);
		Flash_Write_fourByte(OVERLOAD_AD_ADDR,3500);
		
		Flash_Write_fourByte(NULL_CAIIBRATION,0);		//Ĭ�Ͽ���У׼0
		Flash_Write_fourByte(FULL_CAIIBRATION,3800);	//Ĭ������У׼3800
		
		Flash_Write_OneByte(SIGN_ADDR,0);		//Ĭ�ϱ궨������Ϊ0
		Flash_Write_OneByte(NOW_ADDR,0);		//Ĭ���Ѿ��궨����Ϊ0	
		
		Flash_Write_OneByte(FIRST,0xa5);		//д��������ó�Ĭ�ϱ�־
	}
	NullCalibrat = Flash_Read_fourByte(NULL_CAIIBRATION);	//��������У׼
	FullCalibrat = Flash_Read_fourByte(FULL_CAIIBRATION);	//��������У׼
	Flash_Read_MultiBytes(ID_ADDR,ID,3);					//����ID
	Threshold = Flash_Read_OneByte(VPT_ADDR);				//��ֵ
	DampTime = Flash_Read_OneByte(DAMPTE);					//����ʱ��
	Null_Ad = Flash_Read_fourByte(NULL_AD_ADDR);			//�������� AD ֵ����ֵ
	Reload_Ad = Flash_Read_fourByte(RELOAD_AD_ADDR);		//�������� ADֵ
	Overload_Ad = Flash_Read_fourByte(OVERLOAD_AD_ADDR);	//�������� ����ADֵ;
}
/**@brief       D01�����
* @note         GPS �豸��ȡ��������:���ն˻���Է����������Ĳ�ѯ����ֵ���� D01
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D01(void)
{
	uint8_t buf[4]={0};
	uint8_t i = 10;
	
	//������־λ 1�ֽ�
	TX_buf[i++] = HexToAscii(D01_sign[0]);	
	
	//�����ر�־λ 1�ֽ�
	TX_buf[i++] = HexToAscii(D01_sign[1]);

	buf[0] = (ADC0_filter & 0xF000)>>12;
	buf[1] = (ADC0_filter & 0x0F00)>>8;
	buf[2] = (ADC0_filter & 0x00F0)>>4;
	buf[3] = (ADC0_filter & 0x000F);
	
	//��ǰ AD ֵ 4�ֽ�
	TX_buf[i++] = HexToAscii(buf[0]);
	TX_buf[i++] = HexToAscii(buf[1]);
	TX_buf[i++] = HexToAscii(buf[2]);
	TX_buf[i++] = HexToAscii(buf[3]);
	
	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;

	memset(TX_buf,0,i+4);/* ���� */
}

/**@brief       D11�����
* @note         ���ն˻���Է����������Ŀ���У׼���� D11
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D11(void)
{
	uint8_t i = 10;

	if(Flash_Write_fourByte(ADC0_filter,NULL_CAIIBRATION) == 1)
	{
		//�ɹ��ظ� OK
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//ʧ�ܻظ� NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
}


/**@brief       D21�����
* @note         ���ն˻���Է���������������У׼���� D21
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D21(void)
{
	uint8_t i = 10;
	if(Flash_Write_fourByte(ADC0_filter,FULL_CAIIBRATION) == 1)
	{
		//�ɹ��ظ� OK
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//ʧ�ܻظ� NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
}

/**@brief       D25�����
* @note         ƽ��Ƕ�ȫ��У׼Ϊ90��
				����; \n
*/
void D25(void)
{
	uint8_t i = 10;
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
	AngleCalibration();			//ƽ��Ƕ�У׼
}

/**@brief       D25�����
* @note         ƽ��Ƕ�ȫ��У׼Ϊ90��
				����; \n
*/
void D26(void)
{
	uint8_t i = 10;
	
	Flash_Write_twoByte(X_45ACCEL,(uint16_t)X_Angle);
	Flash_Write_twoByte(Y_45ACCEL,(uint16_t)Y_Angle);
	Flash_Write_twoByte(Z_45ACCEL,(uint16_t)Z_Angle);
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
	
}

/**@brief       D31�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻���Է������������趨�豸 ID ������ D31
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D31(const char *RX)		
{
	uint8_t i = 10;
	
	if(Flash_Write_MultiBytes(ID_ADDR,(uint8_t*)RX[10],3) == 1)
	{
		//�ɹ��ظ� OK
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//ʧ�ܻظ� NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	ID[0] = RX[10];
	ID[1] = RX[11];
	ID[2] = RX[12];
	
	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
}

/**@brief       D41�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻���Է������������趨������ֵ������ʱ������ D41
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D41(const char *RX)		
{
	uint8_t i = 10;
	uint8_t j=0;	
	
	//�������仯����ֵ
	Threshold = (AsciiToDecimalism(RX[10])*10);
	Threshold += (AsciiToDecimalism(RX[11]));
	if((Threshold >= 0) && (Threshold <= 0xFF))
		j = Flash_Write_OneByte(VPT_ADDR,Threshold);
	
	//����ʱ��
	DampTime = (AsciiToDecimalism(RX[12])*10);
	DampTime += (AsciiToDecimalism(RX[13]));
	if((DampTime >= 0) && (DampTime <= 0xFF))
		j += Flash_Write_OneByte(DAMPTE,DampTime);
	
	if(j == 2)
	{
		//�ɹ��ظ� OK
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//ʧ�ܻظ� NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	
	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
}


/**@brief       D51�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻��ߵ��Է��͸���������У׼ֵ��ѯ���D51
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D51(const char *RX)		 
{
	uint8_t i = 10;
	uint8_t buf[4]={0};
	
	//У׼�յ�
	buf[0] = (NullCalibrat & 0xF000)>>12;
	buf[1] = (NullCalibrat & 0x0F00)>>8;
	buf[2] = (NullCalibrat & 0x00F0)>>4;
	buf[3] = (NullCalibrat & 0x000F);
	TX_buf[i++] = HexToAscii(buf[0]);
	TX_buf[i++] = HexToAscii(buf[1]);
	TX_buf[i++] = HexToAscii(buf[2]);
	TX_buf[i++] = HexToAscii(buf[3]);
	
	//У׼����
	buf[0] = (FullCalibrat & 0xF000)>>12;
	buf[1] = (FullCalibrat & 0x0F00)>>8;
	buf[2] = (FullCalibrat & 0x00F0)>>4;
	buf[3] = (FullCalibrat & 0x000F);
	TX_buf[i++] = HexToAscii(buf[0]);
	TX_buf[i++] = HexToAscii(buf[1]);
	TX_buf[i++] = HexToAscii(buf[2]);
	TX_buf[i++] = HexToAscii(buf[3]);
	
	//��ǰ����
	buf[0] = (ADC0_filter & 0xF000)>>12;
	buf[1] = (ADC0_filter & 0x0F00)>>8;
	buf[2] = (ADC0_filter & 0x00F0)>>4;
	buf[3] = (ADC0_filter & 0x000F);
	TX_buf[i++] = HexToAscii(buf[0]);
	TX_buf[i++] = HexToAscii(buf[1]);
	TX_buf[i++] = HexToAscii(buf[2]);
	TX_buf[i++] = HexToAscii(buf[3]);

	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
	Parameter_Init();
}

/**@brief       D61�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻���Է�����������У׼�������D61
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D61(const char *RX)		//δ���********
{
	uint8_t i = 10;
	
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
}

/**@brief       D71�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻���Է��������������ر���ֵ�������D71
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D71(const char *RX)		//ʹ�õ�falsh 
{
	uint8_t i = 10;
	uint8_t j = 0;
	
	//�������� AD ֵ
	Null_Ad = AsciiToDecimalism(RX[10]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[11]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[12]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[13]);
	if((Null_Ad >= 0) && (Null_Ad <= 0xFFF))
		j = Flash_Write_fourByte(NULL_AD_ADDR,Null_Ad);
	
	//�������� AD ֵ
	Reload_Ad = AsciiToDecimalism(RX[14]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[15]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[16]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[17]);
	if((Reload_Ad >= 0) && (Reload_Ad <= 0xFFF))
		j += Flash_Write_fourByte(RELOAD_AD_ADDR,Reload_Ad);
	
	//�������� AD ֵ	
	Overload_Ad = AsciiToDecimalism(RX[18]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[19]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[20]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[21]);
	if((Overload_Ad >= 0) && (Overload_Ad <= 0xFFF))
		j += Flash_Write_fourByte(OVERLOAD_AD_ADDR,Overload_Ad);
	
	if(j == 3)	//�ɹ��ظ� OK
	{	
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else		//ʧ�ܻظ� NO
	{
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	
	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
}

/**@brief       D81�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻��ߵ��Է����������Ĳ�ѯ����ֵ���D81
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D81(const char *RX)		
{
	uint8_t i = 10;
	
	//������־λ 1�ֽ�
	TX_buf[i++] = HexToAscii(D01_sign[0]);	
	
	//�����ر�־λ 1�ֽ�
	TX_buf[i++] = HexToAscii(D01_sign[1]);

	//16λ���ݶ���
	TX_buf[i++] = HexToAscii((ton & 0xF000)>>12);		
	TX_buf[i++] = HexToAscii((ton & 0x0F00) >> 8);		
	TX_buf[i++] = HexToAscii((ton & 0x00F0) >> 4);		
	TX_buf[i++] = HexToAscii((ton & 0x000F));		
	
	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
}

/**@brief       D91�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻��ߵ��Է���������������ֵ�궨���D91
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D91(const char *RX)	
{
	uint8_t i = 10,j=0,k=12;
	uint8_t error = 0; 					//�����־
	uint8_t count = 0,count1,count2;		//�궨ֵ����
	uint16_t ad[10],weight[10];						//ad��ֵ������
	
	count1 = Flash_Read_OneByte(SIGN_ADDR);		//������Ҫ�ӵڼ���д��
	count2 = Flash_Read_OneByte(NOW_ADDR);		//�����Ѿ��궨����
	//�궨ֵ����
	count = AsciiToDecimalism(RX[i++]);
	count <<= 4;
	count |= AsciiToDecimalism(RX[i++]);
	
	for(;count > 0;count--)
	{
		if(count1 > 19)
			count1 = 0;
		ad[j] = AsciiToDecimalism(RX[k++]);
		ad[j] <<= 4;
		ad[j] |= AsciiToDecimalism(RX[k++]);
		ad[j] <<= 4;
		ad[j] |= AsciiToDecimalism(RX[k++]);
		ad[j] <<= 4;
		ad[j] |= AsciiToDecimalism(RX[k++]);
	
		weight[j] = AsciiToDecimalism(RX[k++]);
		weight[j] <<= 4;
		weight[j] |= AsciiToDecimalism(RX[k++]);
		weight[j] <<= 4;
		weight[j] |= AsciiToDecimalism(RX[k++]);
		weight[j] <<= 4;
		weight[j] |= AsciiToDecimalism(RX[k++]);
		if(ad[j] <= 0x0FFF)							//12λADֵ�ڷ�Χ����0X0FFF
		{
			Flash_Write_twoByte((AD1_ADDR+(count1*0x02)),ad[j]);
			Flash_Write_twoByte((LOAD1_ADDR+(count1*0x02)),weight[j++]);
			count1++;
		}
		else
			error = 1;
	}
	if((count1 > count2) && (error == 0))
	{
		count2 = count1 + count2;
		if(count2 > 20)
			count2 = 20;
		Flash_Write_OneByte(NOW_ADDR,count2);		//�Ѿ��궨����
	}
	if(error == 0)					//���ִ��󲻼�¼����
		Flash_Write_OneByte(SIGN_ADDR,count1);		//�궨������
	i -= 2;
	if(error == 0)
	{
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else
	{
		//ʧ�ܻظ� NO
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	RS485_Tx;
	CheckCode(i);		//���У����
	TX_buf[i+2] = '\r';
	TX_buf[i+3] = '\n';
	Usart_SendArray(USART1,TX_buf,i+4);
	RS485_Rx;
	memset(TX_buf,0,i+4);/* ���� */
	D81_Dispose();	
}

/**@brief       ���ݰ�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @return       ����ִ�н��
* - 1(�ɹ�)
* - 0(ʧ��)
* @note        \n
*/
uint8_t PacketHandlingModule(const char *RX)
{
	uint8_t i=0;
	
	//ID��֤��У������֤ ��֤ʧ��ֱ�ӷ��ز����κδ���
	if((IDVerification(RX) != 1) || (Verification(RX) != 1))		
		return 0;
	
	for(;i<10 ;i++)					//��ȡ������Ҫ�Ĺ̶���������
		TX_buf[i] = RX[i];
	
	if(strncmp(RX+4,"D01",3) == 0)
		D01();
	else if(strncmp(RX+4,"D11",3) == 0)
		D11();
	else if(strncmp(RX+4,"D21",3) == 0)
		D21();
	else if(strncmp(RX+4,"D25",3) == 0)			//����ƽ��Ƕ�У׼
		D25();
	else if(strncmp(RX+4,"D26",3) == 0)			//����ɽ��У׼
		D26();
	else if(strncmp(RX+4,"D31",3) == 0)
		D31(RX);			//*����ID
	else if(strncmp(RX+4,"D41",3) == 0)
		D41(RX);			//*�趨������ֵ������ʱ��
	else if(strncmp(RX+4,"D51",3) == 0)
		D51(RX);			//*У׼ֵ��ѯ
	else if(strncmp(RX+4,"D61",3) == 0)
		D61(RX);			//*У׼ֵ��ѯ	
	else if(strncmp(RX+4,"D71",3) == 0)
		D71(RX);			//*���ر���ֵ����	
	else if(strncmp(RX+4,"D81",3) == 0)
		D81(RX);			//*��ѯ����ֵ
	else if(strncmp(RX+4,"D91",3) == 0)
		D91(RX);			//*��ѯ����ֵ
	return 0;

}

