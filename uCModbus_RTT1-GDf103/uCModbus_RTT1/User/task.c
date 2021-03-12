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

#include "BSP.h"
#include "iwdg.h"


double ldVolutage,ldVolutag[8];
long ulResult;
extern volatile uint16_t MB_DATA_04[610];
uint32_t buf1,rd485t = 1;
uint16_t redeeming[9];


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

#define BANK0_WRITE_START_ADDR  ((uint32_t)0x08010000)

void RS485()
{
	GPIO_InitTypeDef GPIO_InitStructure;
    //RCC->APB2ENR|=1<<4;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	
}
/**@brief       ����ĳ�ʼ��
* @return       ����ִ�н��1
*/
uint8_t task_init(void) 
{
	Target_Init();	//ϵͳ����
	MB_Init(1000);
	
	EEProm_Init();
	Rd_Dev_Param();				//��ȡ��������modbus��ʼ��
	IWDG_Init();
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
uint16_t aa=0;
/**@brief       �߳�1
* @return       ��
*/
static void task1_thread_entry(void* parameter)
{	
	RS485();
	App_ADC_Task();

//		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//		while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));//ת��������־λ
//		aa=ADC_GetConversionValue(ADC1);//�������һ��ADCx�������ת�����
		rt_thread_mdelay(200);/* ��ʱ200��tick */
			
}

/**@brief       �߳�2 modbus����
* @return       ��
*/
static void task2_thread_entry(void* parameter)
{
	while(1)
	{
		MB_OS_RxTask();
		rt_thread_mdelay(5);/* ��ʱ200��tick */	
		
	}
}



/**@brief       �߳�3 
* @return       ��
*/
static void task3_thread_entry(void* parameter)
{	
	static uint8_t i=0;
	while(1)
	{
		IWDG_Feed();//ι������
		rt_thread_mdelay(10);/* ��ʱ1000��tick */	
		if((rd485t == 1 )|| (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == 1))
		{
			i++;
		}
		else
			i = 0;
		if(i >= 10)
		{	
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);
			rd485t=0;
		}
	
//		rt_thread_mdelay(1000);/* ��ʱ1000��tick */	
//		GPIO_SetBits(GPIOA,GPIO_Pin_8);
	}
}



