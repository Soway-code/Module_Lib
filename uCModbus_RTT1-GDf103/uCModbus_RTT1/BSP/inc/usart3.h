#ifndef _USART3_H_
#define _USART3_H_

    #define TX3_ON    GPIO_SetBits(GPIOB, GPIO_Pin_15)
    #define TX3_OFF   GPIO_ResetBits(GPIOB, GPIO_Pin_15)

    #define IS_AF(c)             ((c >= 'A') && (c <= 'F'))
    #define IS_af(c)             ((c >= 'a') && (c <= 'f'))
    #define IS_09(c)             ((c >= '0') && (c <= '9'))
    #define ISVALIDHEX(c)        IS_AF(c) || IS_af(c) || IS_09(c)
    #define ISVALIDDEC(c)        IS_09(c)
    #define CONVERTDEC(c)        (c - '0')

    #define CONVERTHEX_alpha(c)  (IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
    #define CONVERTHEX(c)        (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

    void UART3_IRQService(void);
    void Uart3_Init(const UartCommTypeDef *pUartStructure);
    u32 UART3_SendData(const u8* UART3_SendBuff, u32 Len);
    s32 UART3_RecvData(u8* UART3_RecvBuff, u32 Len);
    void UART3_ClearRecvBufAndOpenRecv(void);
    void Timer6_Init(u8 u8BaudRate);
    
#endif
