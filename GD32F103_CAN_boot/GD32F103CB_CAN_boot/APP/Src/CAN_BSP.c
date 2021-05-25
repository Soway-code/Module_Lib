/**@file        CAN_APP.c
* @brief        CAN_APP模块的应用
* @details      适用于GD32F103CB单片机,CAN函数
* @author       马灿林
* @date         2021-4-09
* @version      V1.0.0
* @copyright    2021-2030,深圳市信为科技发展有限公司
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* <tr><td>2021/4/09   <td>1.0.0    <td>马灿林    <td>创建初始版本
* </table>
*
**********************************************************************************
*/
#include "CAN_BSP.h"
#include "GD_FlashApp.h"
can_trasnmit_message_struct transmit_message;
FlagStatus receive_flag;
						
/**@brief       CAN0初始化
* @param[in]    baudrate : CAN波特率：
			对应的波特率	输入值
				1Mbps 		= 9
				500Kbps		= 8
				125Kbps 	= 6
				100Kbps 	= 5
				50Kbps 		= 4
				20Kbps 		= 3
				10Kbps 		= 2
				5Kbps 		= 1
				250Kbps 	= 其他数值 
				
* @note         \n
*/
void Can0_init(uint8_t baudrate)
{	
    /* enable can clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOA);
    
    /* configure CAN0 GPIO, CAN0_TX(PD1) and CAN0_RX(PD0) */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    
	 /* configure CAN0 NVIC */
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn,0,0);
	
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;
    /* initialize CAN register */
    can_deinit(CAN0);
    
    /* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_3TQ;
    /* baudrate 1Mbps=4,500k=8,250k=16 */
	if(baudrate == 9)					//1Mbps
		can_parameter.prescaler = 4;
	else if(baudrate == 8)				//500K
		can_parameter.prescaler = 8;
	else if(baudrate == 6)				//125k
		can_parameter.prescaler = 32;
	else if(baudrate == 5)				//100k
		can_parameter.prescaler = 40;
	else if(baudrate == 4)
		can_parameter.prescaler = 80;
	else if(baudrate == 3)
		can_parameter.prescaler = 200;
	else if(baudrate == 2)
		can_parameter.prescaler = 400;
	else if(baudrate == 1)
		can_parameter.prescaler = 800;
	else
		can_parameter.prescaler = 16;		//250k
	
    can_init(CAN0, &can_parameter);

    /* initialize filter */
    /* CAN0 filter number */
    can_filter.filter_number = 0;

    /* initialize filter */    
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;  
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);
	
	can_interrupt_enable(CAN0, CAN_INT_RFNE0);
	
}


/**@brief       CAN发送一组数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为8)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_Msg(uint32_t ID,uint8_t *msg,uint8_t len)
{  
	uint8_t i = 0;
	can_transmit_state_enum flag1;
	transmit_message.tx_sfid = ID;				//标准帧id
    transmit_message.tx_efid = ID;				//扩展帧id
    transmit_message.tx_ft = CAN_FT_DATA;
	uint8_t mailbox_number1 = 0;
	uint32_t j= 0x00FFFFFF;
//	if((*(__IO uint8_t *)(CAN_FRAME_FORMAT+PARA_OFFSET_ADDRESS)) == 0x55)
//		transmit_message.tx_ff = CAN_FF_STANDARD;		//使用标准帧
//	else
		transmit_message.tx_ff = CAN_FF_EXTENDED;		//使用扩展帧
	
    transmit_message.tx_dlen = 8;
	if(len <= 0)
		return 0;
	for(i=0;i<len;i++)
		transmit_message.tx_data[i] = msg[i];
	if(len<8)
	{
		for(i=len;i<8;i++)
			transmit_message.tx_data[i] = 0x00;		//不够8字节补0
	}	
	mailbox_number1 = can_message_transmit(CAN0, &transmit_message);
	flag1 = can_transmit_states(CAN0,CAN_MAILBOX0);
	while((flag1 != 1) && j>0)
	{	
		flag1 = can_transmit_states(CAN0,CAN_MAILBOX0);
		if(j>0)
			j--;
	}
	return mailbox_number1;
}

/**@brief       CAN发送一组数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为8)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_Msg1(uint32_t ID,uint8_t *msg,uint8_t len)
{  
	uint8_t i = 0;
	can_transmit_state_enum flag1;
	transmit_message.tx_sfid = ID;				//标准帧id
    transmit_message.tx_efid = ID;				//扩展帧id
    transmit_message.tx_ft = CAN_FT_DATA;
	uint8_t mailbox_number1 = 0;
	uint32_t j= 0x00FFFFFF;
//	if((*(__IO uint8_t *)(CAN_FRAME_FORMAT+PARA_OFFSET_ADDRESS)) == 0x55)
//		transmit_message.tx_ff = CAN_FF_STANDARD;		//使用标准帧
//	else
		transmit_message.tx_ff = CAN_FF_EXTENDED;		//使用扩展帧
	
    transmit_message.tx_dlen = len;
	if(len <= 0)
		return 0;
	for(i=0;i<len;i++)
		transmit_message.tx_data[i] = msg[i];
	mailbox_number1 = can_message_transmit(CAN0, &transmit_message);
	flag1 = can_transmit_states(CAN0,CAN_MAILBOX0);
	while((flag1 != 1) && j>0)
	{	
		flag1 = can_transmit_states(CAN0,CAN_MAILBOX0);
		if(j>0)
			j--;
	}
	return mailbox_number1;
}
/**@brief       CAN发送多组数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为65535)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_MultiBytes(uint32_t ID,uint8_t *msg,uint16_t len)
{	
	uint8_t *msg1 = msg;
	uint8_t len1 = len;
	uint8_t mailbox_number1 = 0;
	while(len1)
	{
		if(len1 > 8)
		{
			mailbox_number1 += CAN_Send_Msg(ID,msg1,8);
			msg1 = msg1+8;
			len1 -=8;
		}
		else
		{
			mailbox_number1 +=CAN_Send_Msg(ID,msg1,len1);
			len1 =0;
		}
	}
	return mailbox_number1;
}


/**@brief       CAN发送2字节数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为255)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_twoByte(uint32_t ID,uint16_t *msg,uint8_t len)
{
	uint8_t buf[512];
	uint16_t i = 0,j = 0,k = 0;
	for(i=0;i<len;i++)
	{
		buf[k++] = msg[j] >> 8;
		buf[k++] = msg[j++];
	}
	return CAN_Send_MultiBytes(ID,buf,k);
}


/**@brief       CAN发送4字节数据(扩展帧,数据帧) 
* @param[in]    ID : 发送的CANID;
* @param[in]    msg : 数据指针;
* @param[in]    len : 数据长度(最大为50)
* @return       mailbox_number
* @note         \n
*/
uint8_t CAN_Send_fourByte(uint32_t ID,uint32_t *msg,uint8_t len)
{
	uint8_t buf[200];
	uint16_t i = 0,j = 0,k = 0;
	if(len>50)
		len=50;
	for(i=0;i<len;i++)
	{
		buf[k++] = msg[j] >> 24;
		buf[k++] = msg[j] >> 16;
		buf[k++] = msg[j] >> 8;
		buf[k++] = msg[j++];
	}
	return CAN_Send_MultiBytes(ID,buf,k);
}


