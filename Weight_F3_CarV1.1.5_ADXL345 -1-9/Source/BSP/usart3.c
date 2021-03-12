#include "bsp.h"

u8 UART3_RXBuff[128];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 UART3_RXBuffLen = 0;

u8 UART3_TXBUFF[128];

u8 UART3_u8SendIndex = 0;
u8 UART3_u8SendNum = 0;

extern OS_TCB AppUartMasterTCB;

static BitAction Uart3RecvFrameOK = Bit_RESET;
static BitAction Uart3RecvNewData = Bit_RESET;


void Uart3_Init(const UartCommTypeDef *pUartStructure)
{    
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    Uart3RecvNewData = Bit_RESET;
    Uart3RecvFrameOK = Bit_RESET;

    UART3_RXBuffLen = 0;

    UART3_u8SendIndex = 0;
    UART3_u8SendNum = 0;

    //USART_ITConfig( USART1, USART_IT_TXE, DISABLE );
    USART_ITConfig(USART3, USART_IT_TC, DISABLE);
    USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
    USART_Cmd(USART3, DISABLE);

    NVIC_UART3Configuration();

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    USART_DeInit(USART3);    //��λ����1

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_4);
    
    //
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //USART ��ʼ������

    switch (pUartStructure->BaudRate)
    {

    case 0:
        USART_InitStructure.USART_BaudRate = 4800;
        break;
		
    default:
    case 1:
        USART_InitStructure.USART_BaudRate = 9600;
        break;

    case 2:
        USART_InitStructure.USART_BaudRate = 19200;
        break;

    case 3:
        USART_InitStructure.USART_BaudRate = 38400;
        break;

    case 4:
        USART_InitStructure.USART_BaudRate = 57600;
        break;

    case 5:
        USART_InitStructure.USART_BaudRate = 115200;
        break;

    }

 	USART_InitStructure.USART_StopBits = USART_StopBits_1; // 1��ֹͣλ
 	
    switch (pUartStructure->Parity)
    {
    default:
    case 0:
        USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
        break;
    case 1:
        USART_InitStructure.USART_Parity = USART_Parity_Odd;//��У��λ
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;//�ֳ�Ϊ9λ���ݸ�ʽ
        break;
    case 2:
        USART_InitStructure.USART_Parity = USART_Parity_Even;//żУ��λ
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;//�ֳ�Ϊ9λ���ݸ�ʽ
        break;
    }

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������

    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //�շ�ģʽ

    USART_Init(USART3, &USART_InitStructure);   //��ʼ������
    //USART_ClearFlag(USART1, USART_FLAG_TC);
    //USART_ClearFlag(USART1, USART_ISR_TXE);

    TX3_OFF;

    //BSP_IntVectSet(BSP_INT_ID_USART3_4, UART3_IRQService);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  //�����ж�

    USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���

    if(0 != pUartStructure->RTU_1OrASCII_0)
    {
        Timer6_Init(pUartStructure->BaudRate);
    }
    else    /*�����ASCIIģʽ�£�����Ҫ��ʱ��*/
    {
        TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
        TIM_Cmd(TIM6, DISABLE);
        TIM_SetCounter(TIM6, 0);
    }
}

void UART3_IRQService(void)
{
    u8 UartReadTemp;
    
    if (RESET != USART_GetITStatus(USART3, USART_IT_TC))
    {
        USART_ClearITPendingBit(USART3, USART_IT_TC);

        if (UART3_u8SendIndex >= UART3_u8SendNum)
        {
            UART3_u8SendNum = 0;
            USART_ITConfig(USART3, USART_IT_TC, DISABLE);
            TX3_OFF;
        }
        else
        {
            USART_SendData(USART3, UART3_TXBUFF[UART3_u8SendIndex]);
            UART3_u8SendIndex++;
        }
    }

    //===============UART Recv===============//
    if (RESET != USART_GetITStatus(USART3, USART_IT_RXNE))
    {
        UartReadTemp = USART_ReceiveData(USART3);

        if('*' == UartReadTemp)
        {
            Uart3RecvFrameOK = Bit_RESET;
            UART3_RXBuff[0] = UartReadTemp;
            UART3_RXBuffLen = 1;
        }
        else
        {
            if(Bit_RESET == Uart3RecvFrameOK)
            {
                if (UART3_RXBuffLen < (sizeof(UART3_RXBuff) / sizeof(UART3_RXBuff[0])))
                {
                    UART3_RXBuff[UART3_RXBuffLen] = UartReadTemp;
                    UART3_RXBuffLen++;
                }

                if(0x0a == UartReadTemp)
                {
                    if((UART3_RXBuffLen > 2) && (0x0d == UART3_RXBuff[UART3_RXBuffLen - 2]))
                    {
                        Uart3RecvFrameOK = Bit_SET;
                        //OSTaskSemPost((OS_TCB *)&AppUartMasterTCB, (OS_OPT) OS_OPT_POST_NONE,
                        //      (OS_ERR *)&Err);
                    }
                }
            }
        }
    }

    if (RESET != USART_GetFlagStatus(USART3, USART_IT_ORE))
    {
        USART_ClearITPendingBit(USART3, USART_IT_ORE);
    }
}


void TIM6_IRQHandler(void)
{
    OS_ERR  os_err;

    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

        if (Bit_SET == Uart3RecvNewData)
        {
            Uart3RecvNewData = Bit_RESET;
            Uart3RecvFrameOK = Bit_SET;

            TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
            TIM_Cmd(TIM6, DISABLE);

            OSTaskSemPost((OS_TCB *)&AppUartMasterTCB, (OS_OPT) OS_OPT_POST_NONE,
                          (OS_ERR *)&os_err);
        }
        else
        {
            TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
            TIM_Cmd(TIM6, DISABLE);
        }
    }
}

void Timer6_Init(u8 u8BaudRate)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;

    //ʹ�ܶ�ʱ����ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    //��λTIM6��ʱ����ʹ֮�����ʼ״̬
    TIM_DeInit(TIM6);

    //�ȹرն�ʱ���жϣ����յ�һ���ַ��ٴ�
    TIM_Cmd(TIM6, DISABLE);

    NVIC_Timer6Configuration();

    //��ʱ������ģʽ�������˳���Ƶ����
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    //��ʱ������ģʽ  ����/����/���ĶԳƼ���
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    //��ʱ������

    if (u8BaudRate > 2)
    {
        TIM_TimeBaseInitStruct.TIM_Period = 1749;
    }
    else
    {
        TIM_TimeBaseInitStruct.TIM_Period = 6999 / (u8BaudRate + 1);
    }

    //��ʱ��Ԥ��Ƶ����
    //��ʱʱ�� T=(TIM_Prescaler+1)*(TIM_Period+1)/TIMxCLK
    TIM_TimeBaseInitStruct.TIM_Prescaler = 47;

    //��ʼ��TIM6��ʱ��
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);

    //��ʼ�������λ������Ч���������һ�£�����һ���жϣ����Ͻ����ж�
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

    BSP_IntVectSet(BSP_INT_ID_TIM6_DAC, TIM6_IRQHandler);

    //ʹ�ܸ����ж�
    TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);

    TIM_Cmd(TIM6, DISABLE);

    TIM_SetCounter(TIM6, 0);

}

/*
void UART3_ClearRecvBufAndOpenRecv(void)
{
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    
    Uart3RecvFrameOK = Bit_RESET;

    UART3_RXBuffLen = 0;
    CPU_CRITICAL_EXIT();
}
*/

u32 UART3_SendData(const u8* UART3_SendBuff, u32 Len)
{
    u32 i = 0;

    if ((0 == Len) || (((u8*)0) == UART3_SendBuff))
        return 0;

    if (UART3_u8SendNum != 0)
    {
        return 0;
    }

    if (Len > (sizeof(UART3_TXBUFF) / sizeof(UART3_TXBUFF[0])))
    {
        Len = (sizeof(UART3_TXBUFF) / sizeof(UART3_TXBUFF[0]));
    }

    for (i = 0; i < Len; i++)
    {
        UART3_TXBUFF[i] = UART3_SendBuff[i];
    }

    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    
    TX3_ON;

    USART_SendData(USART3, UART3_TXBUFF[0]);

    UART3_u8SendIndex = 1;
    UART3_u8SendNum = Len;
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);

    CPU_CRITICAL_EXIT();
    
    return(Len);
}

s32 UART3_RecvData(u8* UART3_RecvBuff, u32 Len)
{
    u32 i = 0;

    if ((0 == Len) || (((u8*)NULL) == UART3_RecvBuff))
    {
        return 0;
    }

    if (Bit_RESET == Uart3RecvFrameOK)
    {
        return 0;
    }

    if (0 == UART3_RXBuffLen)
    {
        return 0;
    }

    if (Len < UART3_RXBuffLen)
    {
        return -1;
    }
    
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();

    Len = UART3_RXBuffLen;

    for (i = 0; i < Len; i++)
    {
        UART3_RecvBuff[i] = UART3_RXBuff[i];
    }

    Uart3RecvFrameOK = Bit_RESET;

    UART3_RXBuffLen = 0;

    CPU_CRITICAL_EXIT();

    return Len;
}


