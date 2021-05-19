/*************************************************************************************
                                This file is the GD32F103 
						GD seiral mpu USART via DMA process communicate
*************************************************************************************/
//Include files
#include "Usart.h"
#include "GD32F103_Systick.h"


//Datatype define
//---------------Queue datatype structure define------------------------
typedef struct{
	uint16_t	head;	//queue front
	uint16_t	tail;	//queue rear(tail)
	uint16_t	size;	//queue size
	uint8_t*	pqbuf;	//queue pointer
}S_QUEUE_TAG, *PS_QUEUE_TAG;

typedef struct{
	uint8_t	slvid;
	uint8_t	dattype;
	uint8_t	usartsn;
}S_MODBUS_PARA_TAG, *PS_MODBUS_PARA_TAG;

typedef struct{
	uint16_t	bufsize;
	uint8_t*	psndbuf;
}S_SNDDAT_PARA_TAG, *PS_SNDDAT_PARA_TAG;

//Variables define
static bool flag_send_busy;
static S_QUEUE_TAG s_rcvq;
static S_PARSE_TAG s_parse;
static S_MODBUS_PARA_TAG s_mdbs_para;
static S_SNDDAT_PARA_TAG s_sndpara;

//Local functions declare
static void USART_CommunicateControlPinInit( void );
//static bool GD32F10X_Modbus_ModbusTimer( void** pdat );

//Functions implement
/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
	//Remap PB6 and PB7 to USART0 TxD and RxD
//	gpio_pin_remap_config( GPIO_USART0_REMAP, ENABLE );
*******************************************************************************/
//Define baudrate tab
const uint8_t FrameInterval[] = { 4, 16, 8, 4, 2, 1, 1, 1 };
const uint16_t Rs485ctrldly[] = { 200, 800, 400, 200, 100, 58, 38, 18 };
const uint32_t BaudrateTab[] = { 9600, 2400, 4800, 9600, 19200, 38400, 57600, 115200 };
static uint32_t com_id[NUM_COM] = { USART0, USART1 };
static rcu_periph_enum usart_rcu_clk[NUM_COM] = { RCU_USART0, RCU_USART1 };
static uint32_t com_tx_pin[NUM_COM] = { GPIO_PIN_9,  GPIO_PIN_2 };
static uint32_t com_rx_pin[NUM_COM] = { GPIO_PIN_10, GPIO_PIN_3 };
static uint32_t com_gpio_port[NUM_COM] = { GPIOA, GPIOA };
static rcu_periph_enum com_gpio_clk[NUM_COM] = { RCU_GPIOA, RCU_GPIOA };
static IRQn_Type com_irqn[NUM_COM] = { USART0_IRQn, USART1_IRQn };
/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
bool USART_CommunicateInitial( uint8_t com_sn, PS_COMM_BUFPARA ps_combuf, PS_COMM_PARA ps_compara )
{
	if (ps_compara == NULL)
	{
		return FALSE;
	}
	
	//Judge communicate buffer parameters not empty
	if ((ps_combuf->prcv == NULL) || (!ps_combuf->rcvsize))
	{
		return FALSE;
	}
	
	if ((ps_combuf->psnd == NULL) || (!ps_combuf->sndsize))
	{
		return FALSE;
	}
	
	//Variables init
	memset( (uint8_t*)&s_rcvq, 0, sizeof(S_QUEUE_TAG) );
	s_rcvq.head = 0;
	s_rcvq.tail = 0;
	s_rcvq.size = ps_combuf->rcvsize;
	s_rcvq.pqbuf = ps_combuf->prcv;
	
	memset( (uint8_t*)&s_sndpara, 0, sizeof(S_SNDDAT_PARA_TAG) );
	s_sndpara.psndbuf = ps_combuf->psnd;
	s_sndpara.bufsize = ps_combuf->sndsize;
	
	flag_send_busy = FALSE;
	
	nvic_irq_enable( com_irqn[com_sn], 0, 0 );	//Config USARTi interrupt priority
	/* enable GPIO clock */
    rcu_periph_clock_enable( com_gpio_clk[com_sn] );
    /* enable USART clock */
    rcu_periph_clock_enable( usart_rcu_clk[com_sn] );
    /* connect port to USARTx_Tx */
    gpio_init( com_gpio_port[com_sn], GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, com_tx_pin[com_sn]);
    /* connect port to USARTx_Rx */
    gpio_init( com_gpio_port[com_sn], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, com_rx_pin[com_sn]);
	
	USART_CommunicateControlPinInit( );			//RS485 IC Rcv&Trans switch pin init and enable RxD
	//Rs485 TxD RxD control delay timer init
	USART_Timer3_Configuation( ps_compara->BaudRate );
	USART_CommunicateParaConfig( com_sn, ps_compara );
	
	return TRUE;
}



/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
bool USART_CommunicateParaConfig( uint8_t com_sn, PS_COMM_PARA ps_compara )
{
	uint32_t usartid = USART0;
	
	com_sn %= NUM_COM;			//Com port sn init
	usartid = com_id[com_sn];
	
	memset( (uint8_t*)&s_mdbs_para, 0, sizeof(S_MODBUS_PARA_TAG) );
	s_mdbs_para.dattype = ps_compara->ModbusType;
	s_mdbs_para.slvid = ps_compara->SlaveAddr;
	s_mdbs_para.usartsn = com_sn;
	
    /* USART configure */
    usart_baudrate_set( usartid, BaudrateTab[ps_compara->BaudRate] );
	switch (ps_compara->Parity)
	{
		case 1:		//Odd
			usart_word_length_set( usartid, USART_WL_9BIT );
			usart_parity_config( usartid, USART_PM_ODD );
			break;
		
		case 2:		//Even
			usart_word_length_set( usartid, USART_WL_9BIT );
			usart_parity_config( usartid, USART_PM_EVEN );
			break;
		
		case 3:		//None
			usart_word_length_set( usartid, USART_WL_8BIT );
			usart_parity_config( usartid, USART_PM_NONE );
			break;

		default:	//None
			usart_word_length_set( usartid, USART_WL_8BIT );
			usart_parity_config( usartid, USART_PM_NONE );
			break;
	}
    usart_stop_bit_set( usartid, USART_STB_1BIT );
    usart_hardware_flow_rts_config( usartid, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config( usartid, USART_CTS_DISABLE);
    usart_receive_config( usartid, USART_RECEIVE_ENABLE);
    usart_transmit_config( usartid, USART_TRANSMIT_ENABLE);
    usart_enable( usartid );
	
	usart_flag_clear( usartid,  USART_FLAG_TC );
	usart_interrupt_enable( usartid, USART_INT_IDLE );
	usart_dma_receive_config( usartid, USART_DENR_ENABLE);
	
	return TRUE;
}



/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
static void USART_CommunicateControlPinInit( void )
{
	rcu_periph_clock_enable( RS485CTL_SRC_CLK );
	gpio_init( RS485CTL_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RS485CTL_PIN );
	RS485_RCV_ENABLE();
}


/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
bool USART_ConfigPaserParameters( PS_PARSE_TAG pparse )
{
	if ((pparse == NULL) || (pparse->pbuf == NULL) || (!pparse->bufsize))
		return FALSE;
	
	s_parse.pbuf = pparse->pbuf;
	s_parse.bufsize = pparse->bufsize;
	s_parse.pparse = pparse->pparse;
	
	return TRUE;
}


/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
bool My_USART_DMA_SendData(uint16_t datlen)
{
	//	USART0+DMA Transfer data
	RS485_TXD_ENABLE();
	flag_send_busy = TRUE;
	
	dma_transfer_number_config( DMA0, DMA_CH3, datlen );
	dma_channel_enable(DMA0, DMA_CH3);
	return true;
}
/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
//uint16_t My_USART_GetOneFrameDMAData( uint8_t* pRecvBuff, uint16_t RvBuffSize )
//{
//	uint16_t RecvDataLen = 0;
//	
//	if (s_parse.head == s_parse.tail)
//	{
//		s_rcv.RecvStatus = IDLE_STATE;
//        return 0;
//	}
//	
//	memset( pRecvBuff, 0, s_rcv.bufsize );
//	
//	if( s_parse.tail < s_parse.head )
//	{
//		RecvDataLen = s_parse.size-s_parse.head;
//		if( RvBuffSize < RecvDataLen )
//			return 0;
//		memcpy( pRecvBuff, &s_parse.pqbuf[s_parse.head], RecvDataLen );
//		s_parse.head = 0;
//	}
//	if( RvBuffSize < (s_parse.tail-s_parse.head) )
//		return 0;
//	
//	memcpy( &pRecvBuff[RecvDataLen], &s_parse.pqbuf[s_parse.head], (s_parse.tail-s_parse.head) );
//	RecvDataLen += s_parse.tail - s_parse.head;
//	s_parse.head = s_parse.tail;
//	s_rcv.RecvStatus = IDLE_STATE;
//	
//    return RecvDataLen;
//}

/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
bool USART_SendDataViaUSART0DmaChnIsBusy( void )
{
	return flag_send_busy;
}


/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
#define ASCII_FRAME_HEADER  0x3A
#define ASCII_FRAME_ENTER   0x0D
#define ASCII_FRAME_NEWLINE 0x0A
#define ASCII_FRAME_ENDFLAG 0x0D0A
/*
static bool GD32F10X_Modbus_ModbusTimer( void** pdat )
{
    uint32_t len;
    uint8_t* ptmp;
    uint16_t crc_chk;
    uint16_t tmp;
	
	//Parse parameter not init
	if (s_parse.pbuf == NULL)
		return FALSE;

	//Feed watchdog
#ifdef USE_IDWDG
	FEED_I_WATCHDOG();
#endif

	len = 0;
	//Get one frame
	if (s_rcvq.tail < s_rcvq.head)
	{
		len = s_rcvq.size - s_rcvq.head;
		memcpy( s_parse.pbuf, &s_rcvq.pqbuf[s_rcvq.head], len );
		s_rcvq.head = 0;
	}
	memcpy( &s_parse.pbuf[len], &s_rcvq.pqbuf[s_rcvq.head], s_rcvq.tail - s_rcvq.head );
	len += s_rcvq.tail - s_rcvq.head;
	s_rcvq.head = s_rcvq.tail;
	
		if (s_mdbs_para.dattype == MODBUS_ASCII)
		{
			do{
				if (len < 5)
					break;
				
				ptmp = s_parse.pbuf;
				if (*ptmp != ASCII_FRAME_HEADER)
				{
					break;
				}
				ptmp += len - 2;
				if (*ptmp != ASCII_FRAME_ENTER)
					break;
				ptmp ++;
				if (*ptmp != ASCII_FRAME_NEWLINE)
					break;
				
				if (!COMMON_ConverAscii2CommonMode(s_parse.pbuf, len))
				{
					//Add LRC error response
					break;
				}
				
				ptmp = s_parse.pbuf;
				if ((*ptmp) && (*ptmp != s_mdbs_para.slvid))
					break;
				len = (len - 5) / 2;	//Remove 0x3A 0x0D 0x0A and LRC_H LRC_L
				
				len --;	//Remove slave address
				if (*ptmp ++)
				{
					s_parse.pparse( ptmp, len, 0 );	//Not broadcase command
				}
				else
				{
					s_parse.pparse( ptmp, len, 1 );	//Broadcase command
				}
			}while (0);
		}
		else
		{
			do{
				if (len < 4)
					break;
				
				ptmp = s_parse.pbuf;
				if ((*ptmp) && (*ptmp != s_mdbs_para.slvid))
					break;
				
				len -= 2;
				if (!COMMON_Generatoruint16_tbitsCRCCheckCode( &crc_chk, s_parse.pbuf, len ))
				{
					//Add CRC error response
					break;
				}
				
				ptmp += len;
				tmp = *ptmp ++;
				tmp |= (*ptmp) << 8;
				
				if (tmp != crc_chk)
				{
					//Add CRC check code error
					break;
				}
				
				len --;
				ptmp = s_parse.pbuf;
				if (*ptmp ++)
					{
					s_parse.pparse( ptmp, len, 0 );	//Not broadcase command
				}
				else
				{
					s_parse.pparse( ptmp, len, 1 );	//Broadcase command
				}
			}while (0);
		}

	return FALSE;
}
*/

//uint32_t GD32F10X_Modbus_ModbusTimer( uint8_t** pRecvBuff)
//{
//    uint32_t len;
//    uint8_t* ptmp;
//    uint16_t crc_chk;
//    uint16_t tmp;
//	
//	//Parse parameter not init
//	if (s_parse.pbuf == NULL)
//		return FALSE;

//	//Feed watchdog
//#ifdef USE_IDWDG
//	FEED_I_WATCHDOG();
//#endif

//	len = 0;
//	//Get one frame
//	if (s_rcvq.tail < s_rcvq.head)
//	{
//		len = s_rcvq.size - s_rcvq.head;
//		memcpy( s_parse.pbuf, &s_rcvq.pqbuf[s_rcvq.head], len );
//		s_rcvq.head = 0;
//	}
//	memcpy( &s_parse.pbuf[len], &s_rcvq.pqbuf[s_rcvq.head], s_rcvq.tail - s_rcvq.head );
//	//memcpy( &pRecvBuff[s_rcvq.tail - s_rcvq.head], &s_rcvq.pqbuf[s_rcvq.head], s_rcvq.tail - s_rcvq.head );
//	len += s_rcvq.tail - s_rcvq.head;
//	s_rcvq.head = s_rcvq.tail;
//	
//		if (s_mdbs_para.dattype == MODBUS_ASCII)
//		{
//			do{
//				if (len < 5)
//					break;
//				
//				ptmp = s_parse.pbuf;
//				
//				if (*ptmp != ASCII_FRAME_HEADER)
//				{
//					break;
//				}
//				ptmp += len - 2;
//				if (*ptmp != ASCII_FRAME_ENTER)
//					break;
//				ptmp ++;
//				if (*ptmp != ASCII_FRAME_NEWLINE)
//					break;
//				
//				if (!COMMON_ConverAscii2CommonMode(s_parse.pbuf, len))
//				{
//					//Add LRC error response
//					break;
//				}
//				
//				ptmp = s_parse.pbuf;
//				if ((*ptmp) && (*ptmp != s_mdbs_para.slvid))
//					break;
//				len = (len - 5) / 2;	//Remove 0x3A 0x0D 0x0A and LRC_H LRC_L
//				
//				len --;	//Remove slave address
//				if (*ptmp ++)
//				{
//					s_parse.pparse( ptmp, len, 0 );	//Not broadcase command
//				}
//				else
//				{
//					s_parse.pparse( ptmp, len, 1 );	//Broadcase command
//				}
//			}while (0);
//		}
//		else
//		{
//			do{
//				if (len < 4)
//					break;
//				
//				ptmp = s_parse.pbuf;
//				if ((*ptmp) && (*ptmp != s_mdbs_para.slvid))
//					break;
//				
//				len -= 2;
//				if (!COMMON_Generatoruint16_tbitsCRCCheckCode( &crc_chk, s_parse.pbuf, len ))
//				{
//					//Add CRC error response
//					break;
//				}
//				
//				ptmp += len;
//				tmp = *ptmp ++;
//				tmp |= (*ptmp) << 8;
//				
//				if (tmp != crc_chk)
//				{
//					//Add CRC check code error
//					break;
//				}
//				
//				len --;
//				ptmp = s_parse.pbuf;
//				if (*ptmp ++)
//					{
//					s_parse.pparse( ptmp, len, 0 );	//Not broadcase command
//				}
//				else
//				{
//					s_parse.pparse( ptmp, len, 1 );	//Broadcase command
//				}
//			}while (0);
//		}
//	*pRecvBuff = s_parse.pbuf;
//	return len;
//}

uint16_t My_USART_GetOneFrameDMAData( uint8_t* pRecvBuff, uint16_t RvBuffSize )
{
	uint16_t RecvDataLen = 0;
	
	if (s_parse.pbuf == NULL)
		return 0;
	
	//memset( pRecvBuff, 0, s_rcvq.bufsize );
	
	if( s_rcvq.tail < s_rcvq.head )
	{
		RecvDataLen = s_rcvq.size-s_rcvq.head;
		if( RvBuffSize < RecvDataLen )
			return 0;
		memcpy( pRecvBuff, &s_rcvq.pqbuf[s_rcvq.head], RecvDataLen );
		s_rcvq.head = 0;
	}
	if( RvBuffSize < (s_rcvq.tail-s_rcvq.head) )
		return 0;
	
	memcpy( &pRecvBuff[RecvDataLen], &s_rcvq.pqbuf[s_rcvq.head], (s_rcvq.tail-s_rcvq.head) );
	RecvDataLen += s_rcvq.tail - s_rcvq.head;
	s_rcvq.head = s_rcvq.tail;
	//s_parse.RecvStatus = IDLE_STATE;
	
    return RecvDataLen;
}

/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
void USART0_IRQHandler( void )
{
	if (usart_flag_get( USART0, USART_FLAG_IDLEF ) == SET)
	{
		usart_data_receive( USART0 );
		s_rcvq.tail = s_rcvq.size - dma_transfer_number_get( DMA0, DMA_CH4 );
		if (s_rcvq.head != s_rcvq.tail)
		{
			//GD32F103_TimerSet(  eTmr_ModbusTimer, GD32F10X_Modbus_ModbusTimer, NULL, FrameInterval[s_mdbs_para.usartsn] );
		//	MBASCII_Function( );
			GD32F103_TimerSet( eTmr_ModbusTimer, UserApp_ModBusProcess, NULL, FrameInterval[s_mdbs_para.usartsn] );
		}
	}
	
	
}



//User timer1 control RS485 data send over
/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
void USART_Timer3_Configuation( uint8_t dlyid )
{
    /* ----------------------------------------------------------------------------
    TIMER1 Configuration: 
    TIMER1CLK = SystemCoreClock/4800 = 10KHz.
    TIMER1 configuration is timing mode, and the timing is 1ms(10us * 10 = 1ms).
    ---------------------------------------------------------------------------- */
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER3);
    timer_deinit(TIMER3);
    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);
    /* TIMER1 configuration */
    timer_initpara.prescaler         = (SystemCoreClock / 100000 - 1);		//100KHz  10us per step SystemCoreClock
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = Rs485ctrldly[dlyid];
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER3, &timer_initpara);

	//NVIC Config
    nvic_irq_enable(TIMER3_IRQn, 0, 1);
	timer_interrupt_disable( TIMER3, TIMER_INT_UP );
	timer_flag_clear( TIMER3, TIMER_FLAG_UP );
	timer_disable( TIMER3 );
	timer_counter_value_config( TIMER3, 0 );
}



/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
void TIMER3_IRQHandler( void )
{
	if (timer_flag_get( TIMER3, TIMER_FLAG_UP ) == SET)
	{
		timer_flag_clear( TIMER3, TIMER_FLAG_UP );
	}
	
	timer_disable( TIMER3 );
	timer_counter_value_config( TIMER3, 0 );
	timer_interrupt_disable( TIMER3, TIMER_INT_UP );
	
	flag_send_busy = FALSE;
	dma_channel_disable(DMA0, DMA_CH3);
	RS485_RCV_ENABLE();
}




//-----------------Below is modbus process---------------------------------------------
//Functions declare
static void GD32F10X_Modbus_CommunicateResponseInformation( uint16_t );


//Modbus functions implement
/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void GD32F10X_Modbus_UpgradeResponseProcess( uint8_t* psrc, uint16_t len )
{
    uint8_t* ptmp;
    
    ptmp = s_sndpara.psndbuf;
    *ptmp ++ = s_mdbs_para.slvid;
    memcpy(ptmp, psrc, 3);
    ptmp += 3;
    COMMON_Bits16Convert2Bits8( ptmp, len, BIGENDDIAN );
    ptmp += 2;
    memcpy(ptmp, (psrc + 3), len);

    GD32F10X_Modbus_CommunicateResponseInformation( len + 6 );
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void GD32F10X_Modbus_CommunicateResponseInformation( uint16_t datlen )
{
	uint8_t* pans;
    
    if (!datlen)
	{
        return;
	}
    
    //Response operator result
    if (s_mdbs_para.dattype == MODBUS_ASCII)
    {
        uint8_t lrcchk;
        lrcchk = COMMON_GeneratorLRCCheckCode( s_sndpara.psndbuf, datlen);
        pans = s_sndpara.psndbuf + datlen;
        *pans = lrcchk;
        datlen ++;
        if (!COMMON_ConvertHex2Ascii( s_sndpara.psndbuf, datlen ))
		{
            return;
		}
        datlen *= 2;
        pans = s_sndpara.psndbuf;
        *pans++ = ASCII_FRAME_HEADER;
        pans += datlen;
        COMMON_Bits16Convert2Bits8( pans, ASCII_FRAME_ENDFLAG, BIGENDDIAN );
        datlen += 3;
    }
    else
    {
        uint16_t crc_chk;
        COMMON_Generatoruint16_tbitsCRCCheckCode( &crc_chk, s_sndpara.psndbuf, datlen );
        COMMON_Bits16Convert2Bits8( s_sndpara.psndbuf + datlen, crc_chk, LITTLEENDIAN );
        datlen += 2;
    }
    
    //Send response imformation
//	My_USART_DMA_SendData( datlen );
}
