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

uint8_t ID[3]= {0x30,0x30,0x30};
char TX_buf[200];

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
	uint8_t i = 0;
	while(RX[i+2] != 0x0D)
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

/**@brief       D01�����
* @note         GPS �豸��ȡ��������:���ն˻���Է����������Ĳ�ѯ����ֵ���� D01
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D01(void)
{
	uint8_t i = 10;
	//������־λ 1�ֽ�
	TX_buf[i++] = 0x30;	
	
	//�����ر�־λ 1�ֽ�
	TX_buf[i++] = 0x30;
	
	//��ǰ AD ֵ 4�ֽ�
	TX_buf[i++] = 0x30;
	TX_buf[i++] = 0x33;
	TX_buf[i++] = 0x41;
	TX_buf[i++] = 0x42;
	
	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}

/**@brief       D11�����
* @note         ���ն˻���Է����������Ŀ���У׼���� D11
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D11(void)
{
	uint8_t i = 10;
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}


/**@brief       D21�����
* @note         ���ն˻���Է���������������У׼���� D21
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D21(void)
{
	uint8_t i = 10;
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}

/**@brief       D31�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻���Է������������趨�豸 ID ������ D31
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D31(const char *RX)		//ʹ�õ�falsh
{
	uint8_t i = 10;
	
	ID[0] = RX[10];
	ID[1] = RX[11];
	ID[2] = RX[12];
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}

/**@brief       D41�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻���Է������������趨������ֵ������ʱ������ D41
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D41(const char *RX)		//ʹ�õ�falsh 
{
	uint8_t i = 10;
	static uint8_t a=0,a1 = 0;	
	
	//�������仯����ֵ
	a = (AsciiToDecimalism(RX[10])*10);
	a += (AsciiToDecimalism(RX[11]));
	
	//����ʱ��
	a1 = (AsciiToDecimalism(RX[12])*10);
	a1 += (AsciiToDecimalism(RX[13]));
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}

/**@brief       D51�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻��ߵ��Է��͸���������У׼ֵ��ѯ���D51
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D51(const char *RX)		//ʹ�õ�falsh 
{
	uint8_t i = 10;
	
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}

/**@brief       D61�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻���Է�����������У׼�������D61
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D61(const char *RX)		//ʹ�õ�falsh 
{
	uint8_t i = 10;
	
	//�ɹ��ظ� OK
	TX_buf[i++] = 'O';	
	TX_buf[i++] = 'K';

	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}

/**@brief       D71�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻���Է��������������ر���ֵ�������D71
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D71(const char *RX)		//ʹ�õ�falsh 
{
	uint8_t i = 10;
	
	static uint32_t Null_Ad = 0,Reload_Ad = 0,Overload_Ad = 0;		//�������� AD,�������� AD,�������� AD ֵ;
	
	//�������� AD ֵ
	Null_Ad = AsciiToDecimalism(RX[10]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[11]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[12]);
	Null_Ad <<= 4;
	Null_Ad |= AsciiToDecimalism(RX[13]);
	
	//�������� AD ֵ
	Reload_Ad = AsciiToDecimalism(RX[14]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[15]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[16]);
	Reload_Ad <<= 4;
	Reload_Ad += AsciiToDecimalism(RX[17]);
	
	//�������� AD ֵ	
	Overload_Ad = AsciiToDecimalism(RX[18]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[19]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[20]);
	Overload_Ad <<= 4;
	Overload_Ad += AsciiToDecimalism(RX[21]);
	
	if(((Null_Ad != 0x0) && (Reload_Ad != 0x0) && (Overload_Ad != 0x0)) ||
		((Null_Ad != 0xFFFF) && (Reload_Ad != 0xFFFF) && (Overload_Ad != 0xFFFF)))	//�ɹ��ظ� OK
	{	
		TX_buf[i++] = 'O';	
		TX_buf[i++] = 'K';
	}
	else		//ʧ�ܻظ� NO
	{
		TX_buf[i++] = 'N';	
		TX_buf[i++] = 'O';
	}
	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}

/**@brief       D81�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻��ߵ��Է����������Ĳ�ѯ����ֵ���D81
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D81(const char *RX)		//ʹ�õ�falsh 
{
	uint8_t i = 10;
	
//���ж�
	TX_buf[i++] = '0';		//������־λ
	
	TX_buf[i++] = '0';		//�����ر�־λ

	//16λ���ݶ���
	TX_buf[i++] = '0';		
	TX_buf[i++] = '3';		
	TX_buf[i++] = 'A';		
	TX_buf[i++] = '3';		
	
	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
}

/**@brief       D91�����
* @param[in]    *RX : ���ڽ��յ����ݰ�;
* @note         ���ն˻��ߵ��Է���������������ֵ�궨���D91
				���ݸ�ʽ��ϸ˵���鿴��(����)���ز���������_��λ��_ƽ̨Э��.pdf; \n
*/
void D91(const char *RX)		//ʹ�õ�falsh 
{
	uint8_t i = 10;
	uint8_t count = 0;		//�궨ֵ����
	uint16_t D91_Ad = 0,D91_load = 0;

	//�궨ֵ����
	count = AsciiToDecimalism(RX[10]);
	count <<= 4;
	count |= AsciiToDecimalism(RX[11]);	
	
	

	CheckCode(i);		//���У����
	rt_kprintf("%s\r\n",TX_buf);
	memset(TX_buf,0,i+2);/* ���� */
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
	return 0;

}

