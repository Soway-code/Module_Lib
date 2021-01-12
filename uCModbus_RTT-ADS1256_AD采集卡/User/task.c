/**@file        task.c
* @brief        ����ģ��
* @details      ����������ִ������
* @author       �����
* @date         2020-08-14
* @version      V1.0.0
* @copyright    2020-2030,��������Ϊ�Ƽ���չ���޹�˾
**********************************************************************************
* @par �޸���־:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2020/08/01  <td>1.0.0    <td>�����    <td>������ʼ�汾
* </table>
*
**********************************************************************************
*/

#include "task.h"
#include "stdio.h"
#include "ADS1256.h"
#include "stm32f10x_gpio.h"
#include "flash.h"
#include "DataProcessing.h"

uint32_t ldVolutage,ldVolutag[8];
long ulResult;
extern volatile uint16_t MB_DATA_04[610];
uint32_t buf1;
uint16_t redeeming[9];
uint32_t v_buf[8];	//8��ͨ�����˲���ĵ�ѹ�궨ʱʹ��
uint32_t CalibrationValue[88];	//�궨�Ĳ���

static struct rt_thread task1_thread;			 	//�����߳̿��ƿ� 
static rt_uint8_t rt_task1_thread_stack[1024];		//�����߳�ջ

static struct rt_thread task2_thread;				//�����߳̿��ƿ� 
static rt_uint8_t rt_task2_thread_stack[1024];		//�����߳�ջ

static struct rt_thread task3_thread;				//�����߳̿��ƿ� 
static rt_uint8_t rt_task3_thread_stack[1024];		//�����߳�ջ

//��������
static void task1_thread_entry(void* parameter);
static void task2_thread_entry(void* parameter);
static void task3_thread_entry(void* parameter);


//���ò���ʹ�õĹܽų�ʼ��
void ConfigurationGpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); 
//GPIO_Mode_IPU ��������	19 B1  17 A7  15 A5  13  A3
	
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_7; 	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_7|GPIO_Pin_5|GPIO_Pin_3; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);

//  32 A11    41 B5       43  B7
//  18  B0    16  A6   14 A4      12  A2
// GPIO_Mode_Out_PP ������� �͵�ƽ
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_4|GPIO_Pin_2; 
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_0; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_5|GPIO_Pin_0); 
	GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_4|GPIO_Pin_2); 
}
void ReadsTheStatus(uint32_t *baud,uint8_t *node_addr)
{
	uint8_t buf[4]={0};
//��������
	buf[0]= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11);
	buf[1]= GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7);
	if((buf[0]==0)&&(buf[1]==1))
	{
		*baud = 9600;
	}
	else if((buf[0]==1)&&(buf[1]==0))
	{
		*baud = 115200;
	}
	else
		*baud = 9600;
	
//����վ���
	buf[3]= GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
	buf[2]= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7);	
	buf[1]= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
	buf[0]= GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3);
	buf[0]= buf[0]<< 1;
	buf[0]|= buf[1];
	buf[0]= buf[0]<< 1;
	buf[0]|= buf[2];
	buf[0]= buf[0]<< 1;
	buf[0]|= buf[3];
	*node_addr = buf[0]+100;
}

/**@brief       ����ĳ�ʼ��
* @return       ����ִ�н��1
*/
uint8_t task_init(void) 
{
	Target_Init();	//ϵͳ����
	MB_Init(1000);
	ConfigurationGpioInit();
	Init_ADS1256_GPIO(); //��ʼ��ADS1256 GPIO�ܽ� 
	GPIO_SetBits(GPIOB, GPIO_Pin_11 );  
	ADS1256_Init();
	
	rt_thread_init(&task1_thread, 						//�߳̿��ƿ� 
					"task1", 							//�߳����� 
					task1_thread_entry, 				// �߳���ں��� 
					RT_NULL, 							// �߳���ں������� 
					&rt_task1_thread_stack[0],			// �߳�ջ��ʼ��ַ 
					sizeof(rt_task1_thread_stack), 		// �߳�ջ��С 
					3, 									// �̵߳����ȼ� 
					20); 								// �߳�ʱ��Ƭ 
	
	rt_thread_init(&task2_thread, 						// �߳̿��ƿ� 
					"task2", 							// �߳����� 
					task2_thread_entry, 				// �߳���ں��� 
					RT_NULL, 							// �߳���ں������� 
					&rt_task2_thread_stack[0],			// �߳�ջ��ʼ��ַ 
					sizeof(rt_task2_thread_stack), 		// �߳�ջ��С 
					4, 									// �̵߳����ȼ� 
					20); 								// �߳�ʱ��Ƭ 
					
	rt_thread_init(&task3_thread, 						// �߳̿��ƿ� 
					"task3", 							// �߳����� 
					task3_thread_entry, 				// �߳���ں��� 
					RT_NULL, 							// �߳���ں������� 
					&rt_task3_thread_stack[0],			// �߳�ջ��ʼ��ַ 
					sizeof(rt_task3_thread_stack), 		// �߳�ջ��С 
					5, 									// �̵߳����ȼ� 
					20); 								// �߳�ʱ��Ƭ 
	rt_thread_startup(&task3_thread); 					// �����̣߳��������� 
	rt_thread_mdelay(50);/* ��ʱ50��tick */		
	rt_thread_startup(&task2_thread); 					// �����̣߳��������� 
	rt_thread_startup(&task1_thread); 					// �����̣߳��������� 
					
	return 1;
}

/**@brief       �߳�1��ȡ����
* @return       ��
*/
static void task1_thread_entry(void* parameter)
{
	uint8_t i=0,ii=0;
	for(i = 0;i<8;)
	{
		redeeming[i] = FLASH_R(ADD_I0+(i*2));
		i++;
	}
	redeeming[8] = FLASH_R(SIGN);
	if(redeeming[8] != 0x55AA)
	{
		FLASH_W(SIGN,0x55AA);
	}
	while(1)
	{
		for(i=0;i < 8;i++)
		{
			ulResult = ADS_sum( (i << 4) | ADS1256_MUXN_AINCOM);	
			if( ulResult & 0x800000 )
			{
				ulResult = ~(unsigned long)ulResult;
				ulResult &= 0x7fffff;
				ulResult += 1;
				ulResult = -ulResult;
			}
			ldVolutage = (double)ulResult*0.59604644775390625;
			ii=((i & 0x07)?(i & 0x07) - 1:7);
			ldVolutag[ii] = ldVolutage;		
			
		}
		//ͨ�����ݷ���Ĵ���			
		for(i=0;i < 8;)		
		{
			if(redeeming[i] > 0x7D00)
				ldVolutag[i] = (ldVolutag[i] + ((redeeming[i] - 0X7D00)*10));		//����+
			else if(redeeming[i] < 0x7D00)
				ldVolutag[i] = (ldVolutag[i] - ((0x7D00 - redeeming[i])*10));		//����-
//			MB_DATA_04[100+(i*10)] = ((uint32_t)ldVolutag[i]>>16);
//			MB_DATA_04[101+(i*10)] = (uint16_t)((uint32_t)ldVolutag[i]&0x0000FFFF);
//			buf1 =(ldVolutag[i] -((uint32_t)ldVolutag[i]))*1000000;
//			MB_DATA_04[102+(i*10)] = ((uint32_t)buf1>>16);
//			MB_DATA_04[103+(i*10)] = (uint16_t)((uint32_t)buf1&0x0000FFFF);
			i++;
		}
		rt_thread_mdelay(100);/* ��ʱ300��tick */		
	}
}

/**@brief       �߳�2 modbus����
* @return       ��
*/
static void task2_thread_entry(void* parameter)
{	
	uint8_t i = 0;
	for(;i<=10;i++)
	{
		CalibrationValue[i] = Flash_Read_fourByte(CH0_0 +(i*0x04));
		CalibrationValue[i+11] = Flash_Read_fourByte(CH1_0 +(i*0x04));
		CalibrationValue[i+22] = Flash_Read_fourByte(CH2_0 +(i*0x04));
		CalibrationValue[i+33] = Flash_Read_fourByte(CH3_0 +(i*0x04));
		CalibrationValue[i+44] = Flash_Read_fourByte(CH4_0 +(i*0x04));
		CalibrationValue[i+55] = Flash_Read_fourByte(CH5_0 +(i*0x04));
		CalibrationValue[i+66] = Flash_Read_fourByte(CH6_0 +(i*0x04));
		CalibrationValue[i+77] = Flash_Read_fourByte(CH7_0 +(i*0x04));
		CalibrationValue[0] = 0;
		CalibrationValue[11] = 0;
		CalibrationValue[22] = 0;
		CalibrationValue[33] = 0;
		CalibrationValue[44] = 0;
		CalibrationValue[55] = 0;
		CalibrationValue[66] = 0;
		CalibrationValue[77] = 0;

	}
	
	parameter_init(CalibrationValue);
	while(1)
	{
		MB_OS_RxTask();
		rt_thread_mdelay(5);/* ��ʱ5��tick */	
	}
}


/**@brief       �߳�3 ��ַ���������޸��ж�
* @return       ��
*/
static void task3_thread_entry(void* parameter)
{	
	uint8_t i=0;
	uint32_t v_buf1[8];
	uint32_t baud=0,baud1=0;
	uint8_t node_addr = 0,node_addr1 = 0;
	ReadsTheStatus(&baud,&node_addr);
	baud1 = baud;
	node_addr1 = node_addr;
	MB_CfgCh(node_addr,MODBUS_SLAVE,0,MODBUS_MODE_RTU,1,baud,8,MODBUS_PARITY_NONE,1,MODBUS_WR_EN);
	
	while(1)
	{
		ReadsTheStatus(&baud,&node_addr);
		if((baud != baud1) || (node_addr != node_addr1))
		{
			baud1 = baud;
			node_addr1 = node_addr;
			Target_Init();	//ϵͳ����
			MB_Init(1000);
			MB_CfgCh(node_addr,MODBUS_SLAVE,0,MODBUS_MODE_RTU,1,baud,8,MODBUS_PARITY_NONE,1,MODBUS_WR_EN);
		}
		
		v_buf1[0] = Channel_0(ldVolutag[0],&v_buf[0]);
		v_buf1[1] = Channel_1(ldVolutag[1],&v_buf[1]);
		v_buf1[2] = Channel_2(ldVolutag[2],&v_buf[2]);
		v_buf1[3] = Channel_3(ldVolutag[3],&v_buf[3]);
		v_buf1[4] = Channel_4(ldVolutag[4],&v_buf[4]);
		v_buf1[5] = Channel_5(ldVolutag[5],&v_buf[5]);
		v_buf1[6] = Channel_6(ldVolutag[6],&v_buf[6]);
		v_buf1[7] = Channel_7(ldVolutag[7],&v_buf[7]);
		for(i=0;i < 8;)		
		{
			MB_DATA_04[100+(i*10)] = ((uint32_t)v_buf1[i]>>16);
			MB_DATA_04[101+(i*10)] = (uint16_t)v_buf1[i];
			MB_DATA_04[102+(i*10)] = 0xFFFF;
			MB_DATA_04[103+(i*10)] = 0xFFFF;
			i++;
		}
		rt_thread_mdelay(60);/* ��ʱ1000��tick */	
	}
}



