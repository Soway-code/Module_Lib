#include "gd32f10x.h"
//#include <stdio.h>
#include "GD_FlashApp.h"
#include "CAN_BSP.h"
#include "string.h"
#include "systick.h"

typedef  void (*pFunction)(void);
can_receive_message_struct receive_message;		//CAN �������ݽṹ��

#define GD32F103_FEEDWATCHDOG()		fwdgt_counter_reload()

//boot ʹ���ڴ�0x8000000 - 0x08001490
//	0x8003000  1���ֽڴ��������־λ  0x00�Ѿ�������app,   
//  0x8003001- 0x8003FFF   �������APP��������
#define SOWAYAPP_FLASH_PAGES        112				//128 - 16 = 112	//����flashҳ��

#define CANID_ADDR					0x0800305B		//*APP��CANid��ŵ�ַ,��Ҫ����app�޸�
/* BAUDRATE APP�Ĳ����� 1Mbps 		= 9
				500Kbps		= 8
				125Kbps 	= 6
				100Kbps 	= 5
				50Kbps 		= 4
				20Kbps 		= 3
				10Kbps 		= 2
				5Kbps 		= 1
				250Kbps 	= ������ֵ 
*/
#define	BAUDRATE					0x0800305F		//*APP�����ʴ�ŵ�ַ,��Ҫ����app�޸�

#define SOWAYAPP_START_ADDR         0x08004000 		//APP��ʼ��ַ
#define CHIPFLASH_MAXIMUM_ADDR      0x0801FFFF	
#define UPGRADE_FALG    0xFF
uint8_t UpgradeSign;				//������־
uint32_t CANID;
uint32_t Frames=0;					//����APP֡����һ֡8�ֽ�
uint8_t ReplySuccess[8] = {0,0,0,0,0,0,0x4F,0x4B};			//Ӧ��ɹ�	OK(0x4F,0x4B)
uint8_t FailedToReply[8] = {0,0,0,0,0,0,0x4E,0x4F};			//Ӧ��ʧ��	NO(0x4E,0x4F)

pFunction Jump_To_Application;                           	//��ת��Ӧ�ó���
uint32_t JumpAddress;                                    	//��ת��ַ

void JumpToUserApplication(void)
{
    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
    
    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (SOWAYAPP_START_ADDR + 4);
    Jump_To_Application = (pFunction) JumpAddress;

    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) SOWAYAPP_START_ADDR);
	
    /* Jump to application */
    Jump_To_Application();
}

void mystrncpy(uint8_t *in,uint8_t *out,uint8_t len)
{
	uint8_t i = 0;
	while(len--)
	{
		in[i] = out[i];
		i++;
	}
}

static void GD32F103_FDWDGInit( void )
{
	rcu_osci_on(RCU_IRC40K);
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K));
	
	/*	FWDG clock is independent lsi 40KHz, DIV256 IS 156.2Hz, that is 6.4 ms per clock
	reload value is 1000, delay time 2500 * 6.4ms = 16 s	*/
	fwdgt_config( 2500, FWDGT_PSC_DIV256 );
	fwdgt_enable();

}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
uint32_t ByteSum=0;			//���յ����ֽ���
uint32_t ByteSum1=0;		//�ۼƽ��յ����ֽ���
int main(void)	
{	
	uint16_t time=0;
	uint8_t baudrate=*(__IO uint8_t *)BAUDRATE;		//APP�Ĳ�����
	uint8_t buf[2048];
	uint8_t buf1[8]={0xAA,0X55,0xAA,0x04,0x55,0xAA,0x55,0xAA};
	CANID =  *(__IO uint32_t *)CANID_ADDR;			//APP��CANid��ŵ�ַ
	UpgradeSign = *(__IO uint8_t *)UPGRADE_MAEKS;
	systick_config();
	Can0_init(baudrate);
	//GD32F103_FDWDGInit();	//���Ź�16s
	while(1)
	{
		delay_1ms(100);
		time++;
		UpgradeSign = *(__IO uint8_t *)UPGRADE_MAEKS;
		if((time >= 30) && (UpgradeSign == 0x00))							//3s��תAPP	
		{
			JumpToUserApplication();
		}
		if(strncmp((char*)receive_message.rx_data,(char*)buf1,8) == 0)		//���յ�ִ���³���ָ��
		{
			ReplySuccess[3] = 4;
			CAN_Send_Msg(CANID,ReplySuccess,8);								//Ӧ��ɹ�
			JumpToUserApplication();
		}
		if((Frames != 0)&& (Frames <= (SOWAYAPP_FLASH_PAGES*1024)))			//��������
		{
			while(1)
			{
				if(strncmp((char*)receive_message.rx_data,(char*)buf1,8) == 0)	//���յ�ִ���³���ָ��
				{
					ReplySuccess[0] = 0;
					ReplySuccess[3] = 4;
					CAN_Send_Msg(CANID,ReplySuccess,8);							//Ӧ��ɹ�	
					receive_message.rx_data[3] = 0xFF;
					buf[0]=0x00;
					Flash_Write_MultiBytes(UPGRADE_MAEKS,buf, 1);				//����������־
					Frames = 0;
					JumpToUserApplication();
				}
				else if((receive_message.rx_dlen !=0) && (strncmp((char*)receive_message.rx_data,(char*)buf1,8) != 0))			//����APP����
				{
					//strncpy((char*)&buf[ByteSum],(char*)receive_message.rx_data,receive_message.rx_dlen);
					mystrncpy(&buf[ByteSum],receive_message.rx_data,receive_message.rx_dlen);
					ByteSum += receive_message.rx_dlen;
					
					if(ByteSum >= 1024)
					{
						Flash_Write_MultiBytes(SOWAYAPP_START_ADDR+ByteSum1,buf, ByteSum);
						ByteSum1 += ByteSum;
						ByteSum = 0;
				//		GD32F103_FEEDWATCHDOG();

					}
					if(ByteSum1+ByteSum >= Frames)
					{
						if(ByteSum < 1024)
						{
							Flash_Write_MultiBytes(SOWAYAPP_START_ADDR+ByteSum1,buf, ByteSum);	
						}
						buf[0]=0x00;
						Flash_Write_MultiBytes(UPGRADE_MAEKS,buf, 1);				//����������־
						Frames = 0;
					}
					ReplySuccess[0] = 1;
					CAN_Send_Msg1(CANID,ReplySuccess,1);							//Ӧ��ɹ�
					receive_message.rx_dlen = 0;
				}
			}		
		}	
	}
}


/*!
    \brief  +    this function handles CAN0 RX0 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBD_LP_CAN0_RX0_IRQHandler(void)
{
	uint8_t buf[9] = {0xAA,0x55,0xAA,0x01,0x55,0xAA,0x55,0xAA,0xFF};
	uint8_t buf1[4] = {0xAA,0x55,0xAA,0x02};
    /* check the receive message */
    can_message_receive(CAN0, CAN_FIFO0, &receive_message);
	
	if(CANID > 0x1FFFFFFF)
	{
		if(receive_message.rx_efid != 0)
			CANID = receive_message.rx_efid;
		else
			CANID = receive_message.rx_sfid;
	}
	if((receive_message.rx_efid != CANID) && (receive_message.rx_sfid != CANID))	//ID����
		return ;
	
	if((strncmp((char*)receive_message.rx_data,(char*)buf,8) == 0) && (Frames ==0))
	{
		Flash_Write_MultiBytes(UPGRADE_MAEKS,buf, 1);		//����������־
		UpgradeSign = 0xFF;
		ReplySuccess[3] = 1;
		CAN_Send_Msg(CANID,ReplySuccess,8);					//Ӧ��ɹ�
	}
	if(strncmp((char*)receive_message.rx_data,(char*)buf1,4) == 0)
	{
		Frames = receive_message.rx_data[4];
		Frames <<=8;
		Frames |=receive_message.rx_data[5];
		Frames <<=8;
		Frames |=receive_message.rx_data[6];
		Frames <<=8;
		Frames |=receive_message.rx_data[7];
		if((UpgradeSign!= 0x00) && (Frames <= (SOWAYAPP_FLASH_PAGES*1024)))		//�ж�������־��֡�����Ƿ��ڷ�Χ
		{
			ErasureFlashPage(SOWAYAPP_START_ADDR,SOWAYAPP_FLASH_PAGES);			//��������
			ReplySuccess[3] = 2;
			receive_message.rx_dlen = 0;
			CAN_Send_Msg(CANID,ReplySuccess,8);									//Ӧ��ɹ�
		}
		else
		{
			FailedToReply[3] = 2;
			CAN_Send_Msg(CANID,FailedToReply,8);								//Ӧ��ʧ��
		}	
	}
}
