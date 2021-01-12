/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                          (c) Copyright 2003-2009; Micrium, Inc.; Weston, FL               
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                     uC/MODBUS SLAVE COMMAND PROCESSOR
*              
* Filename      : mbs_core.c
* Version       : V2.12
* Programmer(s) : JJL
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  MBS_MODULE
#include "mb.h"
#include "flash.h"
/*
*********************************************************************************************************
*                                                MACROS
*********************************************************************************************************
*/

extern uint32_t CalibrationValue[88];	//标定的参数
extern uint32_t v_buf[8];	//8个通道的滤波后的电压标定时使用
#define  MBS_RX_DATA_START     (((CPU_INT16U)pch->RxFrameData[2] << 8) + (CPU_INT16U)pch->RxFrameData[3])
#define  MBS_RX_DATA_START_H    (pch->RxFrameData[2])
#define  MBS_RX_DATA_START_L    (pch->RxFrameData[3])

#define  MBS_RX_DATA_POINTS    (((CPU_INT16U)pch->RxFrameData[4] << 8) + (CPU_INT16U)pch->RxFrameData[5])
#define  MBS_RX_DATA_POINTS_H   (pch->RxFrameData[4])
#define  MBS_RX_DATA_POINTS_L   (pch->RxFrameData[5])

#define  MBS_RX_DATA_BYTES      (pch->RxFrameData[6])

#define  MBS_RX_DATA_COIL      (((CPU_INT16U)pch->RxFrameData[4] << 8) + (CPU_INT16U)pch->RxFrameData[5])
#define  MBS_RX_DATA_COIL_H     (pch->RxFrameData[4])
#define  MBS_RX_DATA_COIL_L     (pch->RxFrameData[5])

#define  MBS_RX_DATA_REG       (((CPU_INT16U)pch->RxFrameData[4] << 8) + (CPU_INT16U)pch->RxFrameData[5])
#define  MBS_RX_DATA_REG_H      (pch->RxFrameData[4])
#define  MBS_RX_DATA_REG_L      (pch->RxFrameData[5])

#define  MBS_RX_DIAG_CODE      (((CPU_INT16U)pch->RxFrameData[2] << 8) + (CPU_INT16U)pch->RxFrameData[3])
#define  MBS_RX_DIAG_CODE_H     (pch->RxFrameData[2])
#define  MBS_RX_DIAG_CODE_L     (pch->RxFrameData[3])
#define  MBS_RX_DIAG_DATA      (((CPU_INT16U)pch->RxFrameData[4] << 8) + (CPU_INT16U)pch->RxFrameData[5])
#define  MBS_RX_DIAG_DATA_H     (pch->RxFrameData[4])
#define  MBS_RX_DIAG_DATA_L     (pch->RxFrameData[5])

#define  MBS_RX_FRAME          (&pch->RxFrame)
#define  MBS_RX_FRAME_ADDR      (pch->RxFrameData[0])
#define  MBS_RX_FRAME_FC        (pch->RxFrameData[1])
#define  MBS_RX_FRAME_DATA      (pch->RxFrameData[2])
#define  MBS_RX_FRAME_NBYTES    (pch->RxFrameNDataBytes)


#define  MBS_TX_DATA_START_H    (pch->TxFrameData[2])
#define  MBS_TX_DATA_START_L    (pch->TxFrameData[3])

#define  MBS_TX_DATA_POINTS_H   (pch->TxFrameData[4])
#define  MBS_TX_DATA_POINTS_L   (pch->TxFrameData[5])

#define  MBS_TX_DATA_COIL_H     (pch->TxFrameData[4])
#define  MBS_TX_DATA_COIL_L     (pch->TxFrameData[5])

#define  MBS_TX_DATA_REG_H      (pch->TxFrameData[4])
#define  MBS_TX_DATA_REG_L      (pch->TxFrameData[5])

#define  MBS_TX_DIAG_CODE_H     (pch->TxFrameData[2])
#define  MBS_TX_DIAG_CODE_L     (pch->TxFrameData[3])
#define  MBS_TX_DIAG_DATA_H     (pch->TxFrameData[4])
#define  MBS_TX_DIAG_DATA_L     (pch->TxFrameData[5])


#define  MBS_TX_FRAME          (&pch->TxFrame)
#define  MBS_TX_FRAME_ADDR      (pch->TxFrameData[0])
#define  MBS_TX_FRAME_FC        (pch->TxFrameData[1])
#define  MBS_TX_FRAME_DATA      (pch->TxFrameData[2])
#define  MBS_TX_FRAME_NBYTES    (pch->TxFrameNDataBytes)

/*$PAGE*/
/*
*********************************************************************************************************
*                                    LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
extern uint16_t redeeming[9];
#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)

static  void                 MBS_ErrRespSet                (MODBUS_CH *pch, CPU_INT08U errcode);

#if     (MODBUS_CFG_FC01_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC01_CoilRd               (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC02_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC02_DIRd                 (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC03_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC03_HoldingRegRd         (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC04_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC04_InRegRd              (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC05_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC05_CoilWr               (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC06_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC06_HoldingRegWr         (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC08_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC08_Loopback             (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC10_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC10_writes            	 (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC15_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC15_CoilWrMultiple       (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC16_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC16_HoldingRegWrMultiple (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC20_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC20_FileRd               (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_FC21_EN == DEF_ENABLED)
static  CPU_BOOLEAN          MBS_FC21_FileWr               (MODBUS_CH *pch);
#endif

#if     (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
static  void                 MBS_ASCII_Task                (MODBUS_CH   *pch);
#endif

#if     (MODBUS_CFG_RTU_EN   == DEF_ENABLED)
static  void                 MBS_RTU_Task                  (MODBUS_CH   *pch);
#endif

#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                         MBS_ErrRespSet()
*
* Description : This function sets the indicated error response code into the response frame.  Then the
*               routine is called to calculate the error check value.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
*               errcode     An unsigned byte value containing the error code that is to be placed in the
*                           response frame.
*
* Return(s)   : none.
*
* Caller(s)   : MBS_FCxx_Handler()
*             : Modbus Slave functions
*
* Note(s)     : none.                
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static  void  MBS_ErrRespSet (MODBUS_CH  *pch,
                              CPU_INT08U  err_code)
{
    pch->StatExceptCtr++;
    MBS_TX_FRAME_ADDR   = MBS_RX_FRAME_ADDR;
    MBS_TX_FRAME_FC     = MBS_RX_FRAME_FC | 0x80;                /* Set the high order bit of the function code.     */
    MBS_TX_FRAME_DATA   = err_code;                              /* Set the high order bit of the function code.     */
    MBS_TX_FRAME_NBYTES = 1;                                     /* Nbr of data bytes in exception response is 1.    */
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           MBS_FCxx_Handler()
*
* Description : This is the main processing function for MODBUS commands.  The message integrity is
*               verified, and if valid, the function requested is processed.  Unimplemented functions
*               will generate an Illegal Function Exception Response code (01).
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MBS_ASCII_Task()
*               MBS_RTU_Task
*
* Note(s)     : none. 
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
CPU_BOOLEAN  MBS_FCxx_Handler (MODBUS_CH  *pch)
{
    CPU_BOOLEAN   send_reply;


    if ((MBS_RX_FRAME_ADDR == pch->NodeAddr) ||       /* Proper node address? (i.e. Is this message for us?)   */
        (MBS_RX_FRAME_ADDR == 0))        {      		  /* ... or a 'broadcast' address?                         */
                             
        pch->StatSlaveMsgCtr++;
        
        switch (MBS_RX_FRAME_FC) {                           /* Handle the function requested in the frame.           */ 
#if (MODBUS_CFG_FC01_EN == DEF_ENABLED)
            case MODBUS_FC01_COIL_RD:
                 send_reply = MBS_FC01_CoilRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC02_EN == DEF_ENABLED)
            case MODBUS_FC02_DI_RD:
                 send_reply = MBS_FC02_DIRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
            case MODBUS_FC03_HOLDING_REG_RD:                 /* Process read output registers command.                   */
                 send_reply = MBS_FC03_HoldingRegRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
            case MODBUS_FC04_IN_REG_RD:
                 send_reply = MBS_FC04_InRegRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC05_EN == DEF_ENABLED)
            case MODBUS_FC05_COIL_WR:
                 if (pch->WrEn == DEF_TRUE) {
                     send_reply  = MBS_FC05_CoilWr(pch);
                 } else {
                     send_reply  = DEF_FALSE;
                 }
                 break;
#endif

#if (MODBUS_CFG_FC06_EN == DEF_ENABLED)
            case MODBUS_FC06_HOLDING_REG_WR:
                 if (pch->WrEn == DEF_TRUE) {
                     send_reply  = MBS_FC06_HoldingRegWr(pch);
                 } else {
                     send_reply  = DEF_FALSE;
                 }
                 break;
#endif

#if (MODBUS_CFG_FC08_EN == DEF_ENABLED)
            case MODBUS_FC08_LOOPBACK:
                 send_reply = MBS_FC08_Loopback(pch);        /* Process loopback command.                                */
                 break;
#endif

#if (MODBUS_CFG_FC15_EN == DEF_ENABLED)
            case MODBUS_FC15_COIL_WR_MULTIPLE:
                 if (pch->WrEn == DEF_TRUE) {
                     send_reply  = MBS_FC15_CoilWrMultiple(pch);
                 } else {
                     send_reply  = DEF_FALSE;
                 }
                 break;
#endif

#if (MODBUS_CFG_FC16_EN == DEF_ENABLED)
            case MODBUS_FC16_HOLDING_REG_WR_MULTIPLE:
                 if (pch->WrEn == DEF_TRUE) {
                     send_reply  = MBS_FC16_HoldingRegWrMultiple(pch);
				
                 } else {
                     send_reply  = DEF_FALSE;
                 }
                 break;
#endif

#if (MODBUS_CFG_FC20_EN == DEF_ENABLED)
            case MODBUS_FC20_FILE_RD:
                 send_reply  = MBS_FC20_FileRd(pch);
                 break;
#endif

#if (MODBUS_CFG_FC21_EN == DEF_ENABLED)
            case MODBUS_FC21_FILE_WR:
                 if (pch->WrEn == DEF_TRUE) {
                     send_reply = MBS_FC21_FileWr(pch);
                 } else {
                     send_reply = DEF_FALSE;
                 }
                 break;
#endif        

            default:                                         /* Function code not implemented, set error response. */
                 pch->Err   = MODBUS_ERR_ILLEGAL_FC;
                 MBS_ErrRespSet(pch, 
                                MODBUS_ERR_ILLEGAL_FC);
                 send_reply = DEF_TRUE;
                 break;
        }
    }
    else
    {

    }
    
    if (MBS_RX_FRAME_ADDR == 0) 												 /* Was the command received a 'broadcast'?   */
		{     
        return (DEF_FALSE);                              /* Yes, don't reply                           */
    } else {
        return (send_reply);                             /* No,  reply according to the outcome of the command    */
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           MBS_FC01_CoilRd()
*
* Description : Responds to a request to read the status of any number of coils.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC01_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC01_CoilRd (MODBUS_CH  *pch)
{
    CPU_INT08U     *presp;
    CPU_INT16U      coil;
    CPU_BOOLEAN     coil_val;
    CPU_INT16U      err;
    CPU_INT16U      nbr_coils;
    CPU_INT16U      nbr_bytes;
    CPU_INT08U      bit_mask;
    CPU_INT16U      ix;


    if (pch->RxFrameNDataBytes != 4) {                           /* 4 data bytes for this message.                           */
        return (DEF_FALSE);                                      /* Tell caller that we DON'T need to send a response        */
    }
    coil      = MBS_RX_DATA_START;                               /* Get the starting address of the desired coils            */
    nbr_coils = MBS_RX_DATA_POINTS;                              /* Find out how many coils                                  */
    if (nbr_coils == 0 || nbr_coils > 2000) {                    /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC01_01;
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (DEF_TRUE);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes              = ((nbr_coils - 1) / 8) + 1;          /* Find #bytes needed for response.                         */
    pch->TxFrameNDataBytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                  = &pch->TxFrameData[0];               /* Clear bytes in response                                  */
    for (ix = 0; ix < (nbr_bytes + 3); ix++) {
        *presp++ = 0x00;
    }
    bit_mask = 0x01;                                             /* Start with bit 0 in response byte data mask.             */
    ix       =    0;                                             /* Initialize loop counter.                                 */
    presp    = &pch->TxFrameData[0];                             /* Reset the pointer to the start of the response           */
    *presp++ = MBS_RX_FRAME_ADDR;                                /* Prepare response packet                                  */
    *presp++ = MBS_RX_FRAME_FC;
    *presp++ = (CPU_INT08U)nbr_bytes;                            /* Set number of data bytes in response message.            */
    while (ix < nbr_coils) {                                     /* Loop through each COIL requested.                        */
        coil_val = MB_CoilRd(coil,                               /* Get the current value of the coil                        */
                             &err);                        
        switch (err) {
            case MODBUS_ERR_NONE:
                 if (coil_val == MODBUS_COIL_ON) {               /* Only set data response bit if COIL is on.                */
                     *presp |= bit_mask;
                 }
                 coil++;
                 ix++;                                           /* Increment COIL counter.                                  */
                 if ((ix % 8) == 0) {                            /* Determine if 8 data bits have been filled.               */
                     bit_mask   = 0x01;                          /* Reset the data mask.                                     */
                     presp++;                                    /* Increment data frame index.                              */
                 } else {                                        /* Still in same data byte, so                              */
                     bit_mask <<= 1;                             /* Shift the data mask to the next higher bit position.     */
                 }
                 break;

            case MODBUS_ERR_RANGE:
            default:
                 pch->Err = MODBUS_ERR_FC01_02;
                 MBS_ErrRespSet(pch, 
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                 return (DEF_TRUE);                              /* Tell caller that we need to send a response              */
        }
    }
    pch->Err = MODBUS_ERR_NONE;
    return (DEF_TRUE);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                            MBS_FC02_DIRd()
*
* Description : Responds to a request to read the status of any number of Discrete Inputs (DIs).
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC02_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC02_DIRd (MODBUS_CH  *pch)
{
    CPU_INT08U     *presp;
    CPU_INT16U      di;
    CPU_BOOLEAN     di_val;
    CPU_INT16U      err;
    CPU_INT16U      nbr_di;
    CPU_INT16U      nbr_bytes;
    CPU_INT08U      bit_mask;
    CPU_INT16U      ix;


    if (pch->RxFrameNDataBytes != 4) {                           /* 4 data bytes for this message.                           */
        return (DEF_FALSE);                                      /* Tell caller that we DON'T need to send a response        */
    }
    di     = MBS_RX_DATA_START;                                  /* Get the starting address of the desired DIs              */
    nbr_di = MBS_RX_DATA_POINTS;                                 /* Find out how many DIs                                    */
    if (nbr_di == 0 || nbr_di > 2000) {                          /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC02_01;
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (DEF_TRUE);                                       /* Tell caller that we need to send a response              */
    }
    nbr_bytes             = ((nbr_di - 1) / 8) + 1;              /* Find #bytes needed for response.                         */
    pch->TxFrameNDataBytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->TxFrameData[0];                /* Clear bytes in response                                  */
    for (ix = 0; ix < (nbr_bytes + 3); ix++) {
        *presp++ = 0x00;
    }
    bit_mask = 0x01;                                             /* Start with bit 0 in response byte data mask.             */
    ix       =    0;                                             /* Initialize loop counter.                                 */
    presp    = &pch->TxFrameData[0];                             /* Reset the pointer to the start of the response           */
    *presp++ =  MBS_RX_FRAME_ADDR;                               /* Prepare response packet                                  */
    *presp++ =  MBS_RX_FRAME_FC;
    *presp++ = (CPU_INT08U)nbr_bytes;                            /* Set number of data bytes in response message.            */
    while (ix < nbr_di) {                                        /* Loop through each DI requested.                          */
        di_val = MB_DIRd(di,                                     /* Get the current value of the DI                          */
                         &err);                              
        switch (err) {
            case MODBUS_ERR_NONE:
                 if (di_val == MODBUS_COIL_ON) {                 /* Only set data response bit if DI is on.                  */
                     *presp |= bit_mask;
                 }
                 di++;
                 ix++;                                           /* Increment DI counter.                                    */
                 if ((ix % 8) == 0) {                            /* Determine if 8 data bits have been filled.               */
                     bit_mask   = 0x01;                          /* Reset the data mask.                                     */
                     presp++;                                    /* Increment data frame index.                              */
                 } else {                                        /* Still in same data byte, so                              */
                     bit_mask <<= 1;                             /* Shift the data mask to the next higher bit position.     */
                 }
                 break;

            case MODBUS_ERR_RANGE:
            default:
                 pch->Err = MODBUS_ERR_FC02_02;
                 MBS_ErrRespSet(pch, 
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                 return (DEF_TRUE);                              /* Tell caller that we need to send a response              */
        }
    }
    pch->Err = MODBUS_ERR_NONE;
    return (DEF_TRUE);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                        MBS_FC03_HoldingRegRd()
*
* Description : Obtains the contents of the specified holding registers.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC03_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC03_HoldingRegRd (MODBUS_CH  *pch)
{
    CPU_INT08U    *presp;
    CPU_INT16U     err;
    CPU_INT16U     reg;
    CPU_INT16U     nbr_regs;
    CPU_INT16U     nbr_bytes;
    CPU_INT16U     reg_val_16;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    CPU_INT08U     ix;
    CPU_FP32       reg_val_fp;
    CPU_INT08U    *pfp;
#endif


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (DEF_FALSE);                                      /* Tell caller that we DON'T need to send a response        */
    }
    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;
    
#if SE_MODBUS_CODE > 0

    MB_DATA_holdingRegRd(pch, &err);

    if(err == MODBUS_ERR_RANGE)
    {
       pch->Err = MODBUS_ERR_FC03_01;
       MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
       return (DEF_TRUE);
    }
#else 
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    if (reg < MODBUS_CFG_FP_START_IX) {                          /* See if we want integer registers                         */
        if (nbr_regs == 0 || nbr_regs > 125) {                   /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC03_03;
            MBS_ErrRespSet(pch, 
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (DEF_TRUE);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (CPU_INT08U)(nbr_regs * sizeof(CPU_INT16U)); /* Find #bytes needed for response.                         */
    } else {
        if (nbr_regs == 0 || nbr_regs > 62) {                    /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC03_04;
            MBS_ErrRespSet(pch, 
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (DEF_TRUE);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (CPU_INT08U)(nbr_regs * sizeof(CPU_FP32));   /* Find #bytes needed for response.                         */
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125) {                       /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC03_03;                     
        MBS_ErrRespSet(pch,                                
                       MODBUS_ERR_ILLEGAL_DATA_QTY);       
        return (DEF_TRUE);                                       /* Tell caller that we need to send a response              */
    }                                                      
    nbr_bytes = (CPU_INT08U)(nbr_regs * sizeof(CPU_INT16U));     /* Find #bytes needed for response.                         */
#endif
    pch->TxFrameNDataBytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->TxFrameData[0];                /* Reset the pointer to the start of the response           */
    *presp++              =  MBS_RX_FRAME_ADDR;
    *presp++              =  MBS_RX_FRAME_FC;
    *presp++              = (CPU_INT08U)nbr_bytes;               /* Set number of data bytes in response message             */
    while (nbr_regs > 0) {                                       /* Loop through each register requested.                    */
        if (reg < MODBUS_CFG_FP_START_IX) {                      /* See if we want an integer register                       */
            reg_val_16 = MB_HoldingRegRd(reg,                    /* Yes, get its value                                       */
                                         &err);             
            switch (err) {
                case MODBUS_ERR_NONE:
                     *presp++ = (CPU_INT08U)((reg_val_16 >> 8) & 0x00FF); /*      Get MSB first.                             */
                     *presp++ = (CPU_INT08U)(reg_val_16 & 0x00FF);        /*      Get LSB next.                              */
                     break;

                case MODBUS_ERR_RANGE:
                default:
                     pch->Err = MODBUS_ERR_FC03_01;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);
            }
        } else {
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
            reg_val_fp = MB_HoldingRegRdFP(reg,                  /* No,  get the value of the FP register                    */
                                           &err);           
            switch (err) {
                case MODBUS_ERR_NONE:
                     pfp = (CPU_INT08U *)&reg_val_fp;            /* Point to the FP register                                 */
#if (CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG)
                     for (ix = 0; ix < sizeof(CPU_FP32); ix++) { /* Copy value to response buffer                            */
                         *presp++ = *pfp++;
                     }
#else
                     pfp += sizeof(CPU_FP32) - 1;
                     for (ix = 0; ix < sizeof(CPU_FP32); ix++) {
                         *presp++ = *pfp--;
                     }
#endif
                     break;

                case MODBUS_ERR_RANGE:
                default:
                     pch->Err = MODBUS_ERR_FC03_02;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);
            }
#endif
        }
        reg++;                                                   /* Increment current register number                        */
        nbr_regs--;
    }
#endif
    pch->Err = MODBUS_ERR_NONE;
    return (DEF_TRUE);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          MBS_FC04_InRegRd()
*
* Description : Obtains the contents of the specified input registers.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC04_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC04_InRegRd (MODBUS_CH  *pch)
{
    CPU_INT08U    *presp;
    CPU_INT16U     err;
    CPU_INT16U     reg;
    CPU_INT16U     nbr_regs;
    CPU_INT16U     nbr_bytes;
    CPU_INT16U     reg_val_16;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    CPU_INT08U     ix;
    CPU_FP32       reg_val_fp;
    CPU_INT08U    *pfp;
#endif


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (DEF_FALSE);                                      /* Tell caller that we DON'T need to send a response        */
    }
    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;

#if SE_MODBUS_CODE > 0

    MB_DATA_inRegRd(pch, &err);
    
    if(err == MODBUS_ERR_RANGE)
    {
       pch->Err = MODBUS_ERR_FC04_01;
       MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
       return (DEF_TRUE);
    }
    pch->Err = MODBUS_ERR_NONE;
    return (DEF_TRUE); 

#else 
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    if (reg < MODBUS_CFG_FP_START_IX) {                          /* See if we want integer registers                         */
        if (nbr_regs == 0 || nbr_regs > 125) {                   /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC04_03;
            MBS_ErrRespSet(pch, 
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (DEF_TRUE);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (CPU_INT08U)(nbr_regs * sizeof(CPU_INT16U)); /* Find #bytes needed for response.                         */
    } else {
        if (nbr_regs == 0 || nbr_regs > 62) {                    /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC04_04;
            MBS_ErrRespSet(pch, 
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (DEF_TRUE);                                   /* Tell caller that we need to send a response              */
        }
        nbr_bytes = (CPU_INT08U)(nbr_regs * sizeof(CPU_FP32));   /* Find #bytes needed for response.                         */
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125) {                       /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC04_03;                     
        MBS_ErrRespSet(pch,                                
                       MODBUS_ERR_ILLEGAL_DATA_QTY);       
        return (DEF_TRUE);                                       /* Tell caller that we need to send a response              */
    }                                                      
    nbr_bytes = (CPU_INT08U)(nbr_regs * sizeof(CPU_INT16U));     /* Find #bytes needed for response.                         */
#endif
    pch->TxFrameNDataBytes = nbr_bytes + 1;                      /* Number of data bytes + byte count.                       */
    presp                 = &pch->TxFrameData[0];                /* Reset the pointer to the start of the response           */
    *presp++              =  MBS_RX_FRAME_ADDR;                  /* Prepare response packet                                  */
    *presp++              =  MBS_RX_FRAME_FC;
    *presp++              = (CPU_INT08U)nbr_bytes;               /* Set number of data bytes in response message             */
    while (nbr_regs > 0) {                                       /* Loop through each register requested.                    */
        if (reg < MODBUS_CFG_FP_START_IX) {                      /* See if we want an integer register                       */
            reg_val_16 = MB_InRegRd(reg,                         /* Yes, get its value                                       */
                                    &err);                  
            switch (err) {
                case 0:
                     *presp++ = (CPU_INT08U)((reg_val_16 >> 8) & 0x00FF); /*      Get MSB first.                             */
                     *presp++ = (CPU_INT08U)(reg_val_16 & 0x00FF);        /*      Get LSB next.                              */
                     break;
				case 10:
                     *presp++ = (CPU_INT08U)((reg_val_16 >> 8) & 0x00FF); /*      Get MSB first.                             */
                     *presp++ = (CPU_INT08U)(reg_val_16 & 0x00FF);        /*      Get LSB next.                              */
                     break;
				
                case MODBUS_ERR_RANGE:
                default:
                     pch->Err = MODBUS_ERR_FC04_01;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);
            }
        } else {
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
            reg_val_fp = MB_InRegRdFP(reg,                       /* No,  get the value of the FP register                    */
                                      &err);                
            switch (err) {
                case MODBUS_ERR_NONE:
                     pfp = (CPU_INT08U *)&reg_val_fp;            /* Point to the FP register                                 */
#if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
                     for (ix = 0; ix < sizeof(CPU_FP32); ix++) { /* Copy value to response buffer                            */
                         *presp++ = *pfp++;
                     }
#else
                     pfp += sizeof(CPU_FP32) - 1;
                     for (ix = 0; ix < sizeof(CPU_FP32); ix++) {
                         *presp++ = *pfp--;
                     }
#endif
                     break;

                case MODBUS_ERR_RANGE:
                default:
                     pch->Err = MODBUS_ERR_FC04_02;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);
            }
#endif
        }
        reg++;                                                   /* Increment current register number                        */
        nbr_regs--;
    }
#endif 
    pch->Err = MODBUS_ERR_NONE;
    return (DEF_TRUE);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           MBS_FC05_CoilWr()
*
* Description : Responds to a request to force a coil to a specified state.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC05_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC05_CoilWr (MODBUS_CH  *pch)
{
    CPU_INT08U     *prx_data;
    CPU_INT08U     *ptx_data;
    CPU_INT08U      i;
    CPU_INT16U      coil;
    CPU_BOOLEAN     coil_val;
    CPU_INT16U      temp;
    CPU_INT16U      err;
	static uint8_t ch0_sing=0;		//标定标志
	static uint8_t ch1_sing=0;		//标定标志
	static uint8_t ch2_sing=0;		//标定标志
	static uint8_t ch3_sing=0;		//标定标志
	static uint8_t ch4_sing=0;		//标定标志
	static uint8_t ch5_sing=0;		//标定标志
	static uint8_t ch6_sing=0;		//标定标志
	static uint8_t ch7_sing=0;		//标定标志
    if (pch->RxFrameNDataBytes > 4) {                           /* Nbr of data bytes must be 4.                             */
        return (DEF_FALSE);                                      /* Tell caller that we DON'T need to send a response        */
    }
    coil = MBS_RX_DATA_START;                                    /* Get the desired coil number                              */
    temp = MBS_RX_DATA_COIL;
    if (pch->WrEn == DEF_TRUE) {
        if (temp == MODBUS_COIL_OFF_CODE) {                      /* See if coil needs to be OFF?                             */
            coil_val = 0;                                        /* Yes, Turn coil OFF                                       */
        } else {
            coil_val = 1;                                        /* No,  Turn coil ON                                        */
        }
        MB_CoilWr(coil,                                          /* Force coil                                               */
                  coil_val, 
                  &err);                         
        switch (coil) {
            case 0x00:                              
					FLASH_W(ADD_I0,temp);
                 break;
			
			case 0x01:                              
                 FLASH_W(ADD_I1,temp);
                 break;
			case 0x02:                              
                 FLASH_W(ADD_I2,temp);
                 break;
			case 0x03:                              
                 FLASH_W(ADD_I3,temp);
                 break;
			case 0x04:                              
                 FLASH_W(ADD_I4,temp);
                 break;
			case 0x05:                              
                 FLASH_W(ADD_I5,temp);
                 break;
			case 0x06:                              
                 FLASH_W(ADD_I6,temp);
                 break;
			case 0x07:                              
                 FLASH_W(ADD_I7,temp);
                 break;
			case 0x0505: 
			{
				if((temp == 0)  && (ch0_sing == 0))
				{
					CalibrationValue[0] =  v_buf[0];
					ch0_sing = 1;
				}
				else if((temp == 1)  && (ch0_sing == 1))
				{
					CalibrationValue[1] =  v_buf[0];
					ch0_sing = 2;
				}
				else if((temp == 2)  && (ch0_sing == 2))
				{
					CalibrationValue[2] =  v_buf[0];
					ch0_sing = 3;
				}
				else if((temp == 3)  && (ch0_sing == 3))
				{
					CalibrationValue[3] =  v_buf[0];
					ch0_sing = 4;
				}
				else if((temp == 4)  && (ch0_sing == 4))
				{
					CalibrationValue[4] =  v_buf[0];
					ch0_sing = 5;
				}
				else if((temp == 5)  && (ch0_sing == 5))
				{
					CalibrationValue[5] =  v_buf[0];
					ch0_sing = 6;
				}
				else if((temp == 6)  && (ch0_sing == 6))
				{
					CalibrationValue[6] =  v_buf[0];
					ch0_sing = 7;
				}
				else if((temp == 7)  && (ch0_sing == 7))
				{
					CalibrationValue[7] =  v_buf[0];
					ch0_sing = 8;
				}
				else if((temp == 8)  && (ch0_sing == 8))
				{
					CalibrationValue[8] =  v_buf[0];
					ch0_sing = 9;
				}
				else if((temp == 9)  && (ch0_sing == 9))
				{
					CalibrationValue[9] =  v_buf[0];
					ch0_sing = 10;
				}
				else if((temp == 10)  && (ch0_sing == 10))
				{
					ch0_sing = 0;
					CalibrationValue[10] =  v_buf[0];
					Flash_Write_fourByte(CH0_0,CalibrationValue[0]);
					Flash_Write_fourByte(CH0_1,CalibrationValue[1]);
					Flash_Write_fourByte(CH0_2,CalibrationValue[2]);
					Flash_Write_fourByte(CH0_3,CalibrationValue[3]);
					Flash_Write_fourByte(CH0_4,CalibrationValue[4]);
					Flash_Write_fourByte(CH0_5,CalibrationValue[5]);
					Flash_Write_fourByte(CH0_6,CalibrationValue[6]);
					Flash_Write_fourByte(CH0_7,CalibrationValue[7]);
					Flash_Write_fourByte(CH0_8,CalibrationValue[8]);
					Flash_Write_fourByte(CH0_9,CalibrationValue[9]);
					Flash_Write_fourByte(CH0_10,CalibrationValue[10]);
				}
				else if((temp == 11)  && (ch1_sing == 0))
				{
					CalibrationValue[11] =  v_buf[1];
					ch1_sing = 1;
				}
				else if((temp == 12)  && (ch1_sing == 1))
				{
					CalibrationValue[12] =  v_buf[1];
					ch1_sing = 2;
				}
				else if((temp == 13)  && (ch1_sing == 2))
				{
					CalibrationValue[13] =  v_buf[1];
					ch1_sing = 3;
				}
				else if((temp == 14)  && (ch1_sing == 3))
				{
					CalibrationValue[14] =  v_buf[1];
					ch1_sing = 4;
				}
				else if((temp == 15)  && (ch1_sing == 4))
				{
					CalibrationValue[15] =  v_buf[1];
					ch1_sing = 5;
				}
				else if((temp == 16)  && (ch1_sing == 5))
				{
					CalibrationValue[16] =  v_buf[1];
					ch1_sing = 6;
				}
				else if((temp == 17)  && (ch1_sing == 6))
				{
					CalibrationValue[17] =  v_buf[1];
					ch1_sing = 7;
				}
				else if((temp == 18)  && (ch1_sing == 7))
				{
					CalibrationValue[18] =  v_buf[1];
					ch1_sing = 8;
				}
				else if((temp == 19)  && (ch1_sing == 8))
				{
					CalibrationValue[19] =  v_buf[1];
					ch1_sing = 9;
				}
				else if((temp == 20)  && (ch1_sing == 9))
				{
					CalibrationValue[20] =  v_buf[1];
					ch1_sing = 10;
				}
				else if((temp == 21)  && (ch1_sing == 10))
				{
					ch1_sing = 0;
					CalibrationValue[21] =  v_buf[1];
					Flash_Write_fourByte(CH1_0,CalibrationValue[11]);
					Flash_Write_fourByte(CH1_1,CalibrationValue[12]);
					Flash_Write_fourByte(CH1_2,CalibrationValue[13]);
					Flash_Write_fourByte(CH1_3,CalibrationValue[14]);
					Flash_Write_fourByte(CH1_4,CalibrationValue[15]);
					Flash_Write_fourByte(CH1_5,CalibrationValue[16]);
					Flash_Write_fourByte(CH1_6,CalibrationValue[17]);
					Flash_Write_fourByte(CH1_7,CalibrationValue[18]);
					Flash_Write_fourByte(CH1_8,CalibrationValue[19]);
					Flash_Write_fourByte(CH1_9,CalibrationValue[20]);
					Flash_Write_fourByte(CH1_10,CalibrationValue[21]);
				}
				else if((temp == 22)  && (ch2_sing == 0))
				{
					CalibrationValue[22] =  v_buf[2];
					ch2_sing = 1;
				}
				else if((temp == 23)  && (ch2_sing == 1))
				{
					CalibrationValue[23] =  v_buf[2];
					ch2_sing = 2;
				}
				else if((temp == 24)  && (ch2_sing == 2))
				{
					CalibrationValue[24] =  v_buf[2];
					ch2_sing = 3;
				}
				else if((temp == 25)  && (ch2_sing == 3))
				{
					CalibrationValue[25] =  v_buf[2];
					ch2_sing = 4;
				}
				else if((temp == 26)  && (ch2_sing == 4))
				{
					CalibrationValue[26] =  v_buf[2];
					ch2_sing = 5;
				}
				else if((temp == 27)  && (ch2_sing == 5))
				{
					CalibrationValue[27] =  v_buf[2];
					ch2_sing = 6;
				}
				else if((temp == 28)  && (ch2_sing == 6))
				{
					CalibrationValue[28] =  v_buf[2];
					ch2_sing = 7;
				}
				else if((temp == 29)  && (ch2_sing == 7))
				{
					CalibrationValue[29] =  v_buf[2];
					ch2_sing = 8;
				}
				else if((temp == 30)  && (ch2_sing == 8))
				{
					CalibrationValue[30] =  v_buf[2];
					ch2_sing = 9;
				}
				else if((temp == 31)  && (ch2_sing == 9))
				{
					CalibrationValue[31] =  v_buf[2];
					ch2_sing = 10;
				}
				else if((temp == 32)  && (ch2_sing == 10))
				{
					CalibrationValue[32] =  v_buf[2];
					ch2_sing = 0;
					Flash_Write_fourByte(CH2_0,CalibrationValue[22]);
					Flash_Write_fourByte(CH2_1,CalibrationValue[23]);
					Flash_Write_fourByte(CH2_2,CalibrationValue[24]);
					Flash_Write_fourByte(CH2_3,CalibrationValue[25]);
					Flash_Write_fourByte(CH2_4,CalibrationValue[26]);
					Flash_Write_fourByte(CH2_5,CalibrationValue[27]);
					Flash_Write_fourByte(CH2_6,CalibrationValue[28]);
					Flash_Write_fourByte(CH2_7,CalibrationValue[29]);
					Flash_Write_fourByte(CH2_8,CalibrationValue[30]);
					Flash_Write_fourByte(CH2_9,CalibrationValue[31]);
					Flash_Write_fourByte(CH2_10,CalibrationValue[32]);	
				}
				else if((temp == 33)  && (ch3_sing == 0))
				{
					CalibrationValue[33] =  v_buf[3];
					ch3_sing = 1;
				}
				else if((temp == 34)  && (ch3_sing == 1))
				{
					CalibrationValue[34] =  v_buf[3];
					ch3_sing = 2;
				}
				else if((temp == 35)  && (ch3_sing == 2))
				{
					CalibrationValue[35] =  v_buf[3];
					ch3_sing = 3;
				}
				else if((temp == 36)  && (ch3_sing == 3))
				{
					CalibrationValue[36] =  v_buf[3];
					ch3_sing = 4;
				}
				else if((temp == 37)  && (ch3_sing == 4))
				{
					CalibrationValue[37] =  v_buf[3];
					ch3_sing = 5;
				}
				else if((temp == 38)  && (ch3_sing == 5))
				{
					CalibrationValue[38] =  v_buf[3];
					ch3_sing = 6;
				}
				else if((temp == 39)  && (ch3_sing == 6))
				{
					CalibrationValue[39] =  v_buf[3];
					ch3_sing = 7;
				}
				else if((temp == 40)  && (ch3_sing == 7))
				{
					CalibrationValue[40] =  v_buf[3];
					ch3_sing = 8;
				}
				else if((temp == 41)  && (ch3_sing == 8))
				{
					CalibrationValue[41] =  v_buf[3];
					ch3_sing = 9;
				}
				else if((temp == 42)  && (ch3_sing == 9))
				{
					CalibrationValue[42] =  v_buf[3];
					ch3_sing = 10;
				}
				else if((temp == 43)  && (ch3_sing == 10))
				{
					CalibrationValue[43] =  v_buf[3];
					ch3_sing = 0;
					Flash_Write_fourByte(CH3_0,CalibrationValue[33]);
					Flash_Write_fourByte(CH3_1,CalibrationValue[34]);
					Flash_Write_fourByte(CH3_2,CalibrationValue[35]);
					Flash_Write_fourByte(CH3_3,CalibrationValue[36]);
					Flash_Write_fourByte(CH3_4,CalibrationValue[37]);
					Flash_Write_fourByte(CH3_5,CalibrationValue[38]);
					Flash_Write_fourByte(CH3_6,CalibrationValue[39]);
					Flash_Write_fourByte(CH3_7,CalibrationValue[40]);
					Flash_Write_fourByte(CH3_8,CalibrationValue[41]);
					Flash_Write_fourByte(CH3_9,CalibrationValue[42]);
					Flash_Write_fourByte(CH3_10,CalibrationValue[43]);	
				}
				else if((temp == 44)  && (ch4_sing == 0))
				{
					CalibrationValue[44] =  v_buf[4];
					ch4_sing = 1;
				}
				else if((temp == 45)  && (ch4_sing == 1))
				{
					CalibrationValue[45] =  v_buf[4];
					ch4_sing = 2;
				}
				else if((temp == 46)  && (ch4_sing == 2))
				{
					CalibrationValue[46] =  v_buf[4];
					ch4_sing = 3;
				}
				else if((temp == 47)  && (ch4_sing == 3))
				{
					CalibrationValue[47] =  v_buf[4];
					ch4_sing = 4;
				}
				else if((temp == 48)  && (ch4_sing == 4))
				{
					CalibrationValue[48] =  v_buf[4];
					ch4_sing = 5;
				}
				else if((temp == 49)  && (ch4_sing == 5))
				{
					CalibrationValue[49] =  v_buf[4];
					ch4_sing = 6;
				}
				else if((temp == 50)  && (ch4_sing == 6))
				{
					CalibrationValue[50] =  v_buf[4];
					ch4_sing = 7;
				}
				else if((temp == 51)  && (ch4_sing == 7))
				{
					CalibrationValue[51] =  v_buf[4];
					ch4_sing = 8;
				}
				else if((temp == 52)  && (ch4_sing == 8))
				{
					CalibrationValue[52] =  v_buf[4];
					ch4_sing = 9;
				}
				else if((temp == 53)  && (ch4_sing == 9))
				{
					CalibrationValue[53] =  v_buf[4];
					ch4_sing = 10;
				}
				else if((temp == 54)  && (ch4_sing == 10))
				{
					CalibrationValue[54] =  v_buf[4];
					ch4_sing = 0;
					Flash_Write_fourByte(CH4_0,CalibrationValue[44]);
					Flash_Write_fourByte(CH4_1,CalibrationValue[45]);
					Flash_Write_fourByte(CH4_2,CalibrationValue[46]);
					Flash_Write_fourByte(CH4_3,CalibrationValue[47]);
					Flash_Write_fourByte(CH4_4,CalibrationValue[48]);
					Flash_Write_fourByte(CH4_5,CalibrationValue[49]);
					Flash_Write_fourByte(CH4_6,CalibrationValue[50]);
					Flash_Write_fourByte(CH4_7,CalibrationValue[51]);
					Flash_Write_fourByte(CH4_8,CalibrationValue[52]);
					Flash_Write_fourByte(CH4_9,CalibrationValue[53]);
					Flash_Write_fourByte(CH4_10,CalibrationValue[54]);	
				}
				else if((temp == 55)  && (ch5_sing == 0))
				{
					CalibrationValue[55] =  v_buf[5];
					ch5_sing = 1;
				}
				else if((temp == 56)  && (ch5_sing == 1))
				{
					CalibrationValue[56] =  v_buf[5];
					ch5_sing = 2;
				}
				else if((temp == 57)  && (ch5_sing == 2))
				{
					CalibrationValue[57] =  v_buf[5];
					ch5_sing = 3;
				}
				else if((temp == 58)  && (ch5_sing == 3))
				{
					CalibrationValue[58] =  v_buf[5];
					ch5_sing = 4;
				}
				else if((temp == 59)  && (ch5_sing == 4))
				{
					CalibrationValue[59] =  v_buf[5];
					ch5_sing = 5;
				}
				else if((temp == 60)  && (ch5_sing == 5))
				{
					CalibrationValue[60] =  v_buf[5];
					ch5_sing = 6;
				}
				else if((temp == 61)  && (ch5_sing == 6))
				{
					CalibrationValue[61] =  v_buf[5];
					ch5_sing = 7;
				}
				else if((temp == 62)  && (ch5_sing == 7))
				{
					CalibrationValue[62] =  v_buf[5];
					ch5_sing = 8;
				}
				else if((temp == 63)  && (ch5_sing == 8))
				{
					CalibrationValue[63] =  v_buf[5];
					ch5_sing = 9;
				}
				else if((temp == 64)  && (ch5_sing == 9))
				{
					CalibrationValue[64] =  v_buf[5];
					ch5_sing = 10;
				}
				else if((temp == 65)  && (ch5_sing == 10))
				{
					CalibrationValue[65] =  v_buf[5];
					ch5_sing = 0;
					Flash_Write_fourByte(CH5_0,CalibrationValue[55]);
					Flash_Write_fourByte(CH5_1,CalibrationValue[56]);
					Flash_Write_fourByte(CH5_2,CalibrationValue[57]);
					Flash_Write_fourByte(CH5_3,CalibrationValue[58]);
					Flash_Write_fourByte(CH5_4,CalibrationValue[59]);
					Flash_Write_fourByte(CH5_5,CalibrationValue[60]);
					Flash_Write_fourByte(CH5_6,CalibrationValue[61]);
					Flash_Write_fourByte(CH5_7,CalibrationValue[62]);
					Flash_Write_fourByte(CH5_8,CalibrationValue[63]);
					Flash_Write_fourByte(CH5_9,CalibrationValue[64]);
					Flash_Write_fourByte(CH5_10,CalibrationValue[65]);	
				}
				else if((temp == 66)  && (ch6_sing == 0))
				{
					CalibrationValue[66] =  v_buf[6];
					ch6_sing = 1;
				}
				else if((temp == 67)  && (ch6_sing == 1))
				{
					CalibrationValue[67] =  v_buf[6];
					ch6_sing = 2;
				}
				else if((temp == 68)  && (ch6_sing == 2))
				{
					CalibrationValue[68] =  v_buf[6];
					ch6_sing = 3;
				}
				else if((temp == 69)  && (ch6_sing == 3))
				{
					CalibrationValue[69] =  v_buf[6];
					ch6_sing = 4;
				}
				else if((temp == 70)  && (ch6_sing == 4))
				{
					CalibrationValue[70] =  v_buf[6];
					ch6_sing = 5;
				}
				else if((temp == 71)  && (ch6_sing == 5))
				{
					CalibrationValue[71] =  v_buf[6];
					ch6_sing = 6;
				}
				else if((temp == 72)  && (ch6_sing == 6))
				{
					CalibrationValue[72] =  v_buf[6];
					ch6_sing = 7;
				}
				else if((temp == 73)  && (ch6_sing == 7))
				{
					CalibrationValue[73] =  v_buf[6];
					ch6_sing = 8;
				}
				else if((temp == 74)  && (ch6_sing == 8))
				{
					CalibrationValue[74] =  v_buf[6];
					ch6_sing = 9;
				}
				else if((temp == 75)  && (ch6_sing == 9))
				{
					CalibrationValue[75] =  v_buf[6];
					ch6_sing = 10;
				}
				else if((temp == 76)  && (ch6_sing == 10))
				{
					CalibrationValue[76] =  v_buf[6];
					ch6_sing = 0;
					Flash_Write_fourByte(CH6_0,CalibrationValue[66]);
					Flash_Write_fourByte(CH6_1,CalibrationValue[67]);
					Flash_Write_fourByte(CH6_2,CalibrationValue[68]);
					Flash_Write_fourByte(CH6_3,CalibrationValue[69]);
					Flash_Write_fourByte(CH6_4,CalibrationValue[70]);
					Flash_Write_fourByte(CH6_5,CalibrationValue[71]);
					Flash_Write_fourByte(CH6_6,CalibrationValue[72]);
					Flash_Write_fourByte(CH6_7,CalibrationValue[73]);
					Flash_Write_fourByte(CH6_8,CalibrationValue[74]);
					Flash_Write_fourByte(CH6_9,CalibrationValue[75]);
					Flash_Write_fourByte(CH6_10,CalibrationValue[76]);
				}
				else if((temp == 77)  && (ch7_sing == 0))
				{
					CalibrationValue[77] =  v_buf[7];
					ch7_sing = 1;
				}
				else if((temp == 78)  && (ch7_sing == 1))
				{
					CalibrationValue[78] =  v_buf[7];
					ch7_sing = 2;
				}
				else if((temp == 79)  && (ch7_sing == 2))
				{
					CalibrationValue[79] =  v_buf[7];
					ch7_sing = 3;
				}
				else if((temp == 80)  && (ch7_sing == 3))
				{
					CalibrationValue[80] =  v_buf[7];
					ch7_sing = 4;
				}
				else if((temp == 81)  && (ch7_sing == 4))
				{
					CalibrationValue[81] =  v_buf[7];
					ch7_sing = 5;
				}
				else if((temp == 82)  && (ch7_sing == 5))
				{
					CalibrationValue[82] =  v_buf[7];
					ch7_sing = 6;
				}
				else if((temp == 83)  && (ch7_sing == 6))
				{
					CalibrationValue[83] =  v_buf[7];
					ch7_sing = 7;
				}
				else if((temp == 84)  && (ch7_sing == 7))
				{
					CalibrationValue[84] =  v_buf[7];
					ch7_sing = 8;
				}
				else if((temp == 85)  && (ch7_sing == 8))
				{
					CalibrationValue[85] =  v_buf[7];
					ch7_sing = 9;
				}
				else if((temp == 86)  && (ch7_sing == 9))
				{
					CalibrationValue[86] =  v_buf[7];
					ch7_sing = 10;
				}
				else if((temp == 87)  && (ch7_sing == 10))
				{
					CalibrationValue[87] =  v_buf[7];
					ch7_sing = 0;
					Flash_Write_fourByte(CH7_0,CalibrationValue[77]);
					Flash_Write_fourByte(CH7_1,CalibrationValue[78]);
					Flash_Write_fourByte(CH7_2,CalibrationValue[79]);
					Flash_Write_fourByte(CH7_3,CalibrationValue[80]);
					Flash_Write_fourByte(CH7_4,CalibrationValue[81]);
					Flash_Write_fourByte(CH7_5,CalibrationValue[82]);
					Flash_Write_fourByte(CH7_6,CalibrationValue[83]);
					Flash_Write_fourByte(CH7_7,CalibrationValue[84]);
					Flash_Write_fourByte(CH7_8,CalibrationValue[85]);
					Flash_Write_fourByte(CH7_9,CalibrationValue[86]);
					Flash_Write_fourByte(CH7_10,CalibrationValue[87]);
				}
				else return (DEF_FALSE); 
			
			}
				
            default:
                 pch->Err = MODBUS_ERR_FC05_01;
                 MBS_ErrRespSet(pch, 
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                 break;
        }
    } else {
        pch->Err = MODBUS_ERR_FC05_02;
        MBS_ErrRespSet(pch,                                      /* Writes are not enabled                                   */
                       MODBUS_ERR_ILLEGAL_DATA_VAL);        
    }
    pch->TxFrameNDataBytes = 4;
    MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;                       /* Prepare response packet                                  */
    MBS_TX_FRAME_FC   = MBS_RX_FRAME_FC;
    prx_data          = &pch->RxFrameData[2];                    /* Copy four data bytes from the receive packet             */
    ptx_data          = &pch->TxFrameData[2];
    for (i = 0; i < 4; i++) {
        *ptx_data++ = *prx_data++;
    }
    pch->Err = MODBUS_ERR_NONE;
    for(i = 0;i<8;)
	{
		redeeming[i] = FLASH_R(ADD_I0+(i*2));
		i++;
	}
	return (DEF_TRUE);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                        MBS_FC06_HoldingRegWr()
*
* Description : Responds to a request to set a single register.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC06_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC06_HoldingRegWr (MODBUS_CH *pch)
{
    CPU_INT08U    *prx_data;
    CPU_INT08U    *ptx_data;
    CPU_INT08U     i;
    CPU_INT08U     max;
    CPU_INT16U     err;
    CPU_INT16U     reg;
    CPU_INT16U     reg_val_16;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    CPU_FP32       reg_val_fp;
    CPU_INT08U    *pfp;
#endif


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (DEF_FALSE);
    }
    pch->TxFrameNDataBytes = pch->RxFrameNDataBytes;
    reg =  MBS_RX_DATA_START;
#if SE_MODBUS_CODE > 0

    MB_DATA_holdingRegWr(pch, &err);

    if(err == MODBUS_ERR_RANGE)
    {
       pch->Err = MODBUS_ERR_FC06_01;
       MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
       return (DEF_TRUE);
    }
    
    pch->WrCtr++;
    MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;                       /* Prepare response packet (duplicate Rx frame)             */
    MBS_TX_FRAME_FC   = MBS_RX_FRAME_FC;
    prx_data          = &pch->RxFrameData[2];                    /* Copy received register address and data to response      */
    ptx_data          = &pch->TxFrameData[2];
    
    max = sizeof(CPU_INT16U) + 2;
    
    for (i = 0; i < max; i++) {
        *ptx_data++ = *prx_data++;
    }
    

#else
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    if (reg < MODBUS_CFG_FP_START_IX) {
        reg_val_16 = MBS_RX_DATA_REG;
        MB_HoldingRegWr(reg,                                     /* Write to integer register                                */
                        reg_val_16, 
                        &err);                  
    } else {
        prx_data = &pch->RxFrameData[4];                         /* Point to data in the received frame.                     */
        pfp      = (CPU_INT08U *)&reg_val_fp;
#if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
        for (i = 0; i < sizeof(CPU_FP32); i++) {
            *pfp++ = *prx_data++;
        }
#else
        prx_data += sizeof(CPU_FP32) - 1;
        for (i = 0; i < sizeof(CPU_FP32); i++) {
            *pfp++ = *prx_data--;
        }
#endif
        MB_HoldingRegWrFP(reg,                                   /* Write to floating point register                         */
                          reg_val_fp, 
                          &err);                
    }
#else
    reg_val_16 = MBS_RX_DATA_REG;       
    MB_HoldingRegWr(reg,                                         /* Write to integer register                                */
                    reg_val_16,         
                    &err);                      
#endif
    switch (err) {
        case MODBUS_ERR_NONE:                                    /* Reply with echoe of command received                     */
             pch->WrCtr++;
             break;

        case MODBUS_ERR_RANGE:
        default:
             pch->Err = MODBUS_ERR_FC06_01;
             MBS_ErrRespSet(pch, 
                            MODBUS_ERR_ILLEGAL_DATA_ADDR);
             break;
    }
    pch->TxFrameNDataBytes = 4;
    MBS_TX_FRAME_ADDR = MBS_RX_FRAME_ADDR;                       /* Prepare response packet (duplicate Rx frame)             */
    MBS_TX_FRAME_FC   = MBS_RX_FRAME_FC;
    prx_data          = &pch->RxFrameData[2];                    /* Copy received register address and data to response      */
    ptx_data          = &pch->TxFrameData[2];
    if (reg < MODBUS_CFG_FP_START_IX) {
        max = sizeof(CPU_INT16U) + 2;
    } else {
        max = sizeof(CPU_FP32) + 2;
    }
    for (i = 0; i < max; i++) {
        *ptx_data++ = *prx_data++;
    }
#endif
    pch->Err = MODBUS_ERR_NONE;
    return (DEF_TRUE);
}
#endif
#endif

/*
*********************************************************************************************************
*                                           MBS_FC08_Loopback()
*
* Description : The LOOPBACK function contains various diagnostic codes that perform specific actions.
*               This function processes individual diagnostic requests and formats the response message
*               frame accordingly.  Unimplemented diagnostic codes will return an Illegal Data Value
*               Exception Response Code (03).
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC08_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC08_Loopback (MODBUS_CH  *pch)
{
    CPU_INT16U  diagcode;


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (DEF_FALSE);
    }
    diagcode           = MBS_RX_DIAG_CODE;
    MBS_TX_FRAME_ADDR  = MBS_RX_FRAME_ADDR;                      /* Prepare response packet                                  */
    MBS_TX_FRAME_FC    = MBS_RX_FRAME_FC;
    MBS_TX_DIAG_CODE_H = MBS_RX_DIAG_CODE_H;
    MBS_TX_DIAG_CODE_L = MBS_RX_DIAG_CODE_L;
    switch (diagcode) {
        case MODBUS_FC08_LOOPBACK_QUERY:                         /* Return Query function code - no need to do anything.     */
             MBS_TX_DIAG_DATA_H = MBS_RX_DIAG_DATA_H;
             MBS_TX_DIAG_DATA_L = MBS_RX_DIAG_DATA_L;
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_CLR_CTR:
#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FC08_EN  == DEF_ENABLED)
             MBS_StatInit(pch);                                  /* Initialize the system counters, echo response back.      */
#endif
             pch->Err = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_MSG_CTR:                   /* Return the message count in diag information field.      */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatMsgCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatMsgCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_CRC_CTR:                   /* Return the CRC error count in diag information field.    */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatCRCErrCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatCRCErrCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_EXCEPT_CTR:                /* Return exception count in diag information field.        */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatExceptCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatExceptCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_SLAVE_MSG_CTR:                 /* Return slave message count in diag information field.    */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatSlaveMsgCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatSlaveMsgCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_SLAVE_NO_RESP_CTR:             /* Return no response count in diag information field.      */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatNoRespCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatNoRespCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        default:
             pch->Err = MODBUS_ERR_FC08_01;
             MBS_ErrRespSet(pch, 
                            MODBUS_ERR_ILLEGAL_DATA_VAL);
             break;
    }
    return (DEF_TRUE);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif


#if (MODBUS_CFG_FC10_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC10_writes (MODBUS_CH  *pch)
{
    CPU_INT16U  diagcode;


    if (pch->RxFrameNDataBytes != 4) {                           /* Nbr of data bytes must be 4.                             */
        return (DEF_FALSE);
    }
    diagcode           = MBS_RX_DIAG_CODE;
    MBS_TX_FRAME_ADDR  = MBS_RX_FRAME_ADDR;                      /* Prepare response packet                                  */
    MBS_TX_FRAME_FC    = MBS_RX_FRAME_FC;
    MBS_TX_DIAG_CODE_H = MBS_RX_DIAG_CODE_H;
    MBS_TX_DIAG_CODE_L = MBS_RX_DIAG_CODE_L;
    switch (diagcode) {
        case MODBUS_FC08_LOOPBACK_QUERY:                         /* Return Query function code - no need to do anything.     */
             MBS_TX_DIAG_DATA_H = MBS_RX_DIAG_DATA_H;
             MBS_TX_DIAG_DATA_L = MBS_RX_DIAG_DATA_L;
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_CLR_CTR:
#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FC08_EN  == DEF_ENABLED)
             MBS_StatInit(pch);                                  /* Initialize the system counters, echo response back.      */
#endif
             pch->Err = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_MSG_CTR:                   /* Return the message count in diag information field.      */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatMsgCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatMsgCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_CRC_CTR:                   /* Return the CRC error count in diag information field.    */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatCRCErrCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatCRCErrCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_BUS_EXCEPT_CTR:                /* Return exception count in diag information field.        */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatExceptCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatExceptCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_SLAVE_MSG_CTR:                 /* Return slave message count in diag information field.    */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatSlaveMsgCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatSlaveMsgCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        case MODBUS_FC08_LOOPBACK_SLAVE_NO_RESP_CTR:             /* Return no response count in diag information field.      */
             MBS_TX_DIAG_DATA_H = (CPU_INT08U)((pch->StatNoRespCtr & 0xFF00) >> 8);
             MBS_TX_DIAG_DATA_L = (CPU_INT08U) (pch->StatNoRespCtr & 0x00FF);
             pch->Err           = MODBUS_ERR_NONE;
             break;

        default:
             pch->Err = MODBUS_ERR_FC08_01;
             MBS_ErrRespSet(pch, 
                            MODBUS_ERR_ILLEGAL_DATA_VAL);
             break;
    }
    return (DEF_TRUE);                                           /* Tell caller that we need to send a response              */
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                       MBS_FC15_CoilWrMultiple()
*
* Description : Processes the MODBUS "Force Multiple COILS" command and writes the COIL states.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC15_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC15_CoilWrMultiple (MODBUS_CH  *pch)
{
    CPU_INT16U    ix;
    CPU_INT16U    coil;
    CPU_INT16U    nbr_coils;
    CPU_INT16U    nbr_bytes;
    CPU_INT08U    data_ix;
    CPU_BOOLEAN   coil_val;
    CPU_INT08U    temp;
    CPU_INT16U    err;


    if (pch->WrEn == DEF_TRUE) {
        if (pch->RxFrameNDataBytes < 6) {                        /* Minimum Nbr of data bytes must be 6.                     */
            return (DEF_FALSE);                                  /* Tell caller that we DON'T need to send a response        */
        }
        coil      = MBS_RX_DATA_START;
        nbr_coils = MBS_RX_DATA_POINTS;
        nbr_bytes = MBS_RX_DATA_BYTES;                           /* Get the byte count for the data.                         */
        if (((((nbr_coils - 1) / 8) + 1) ==  nbr_bytes) &&       /* Be sure #bytes valid for number COILS.                   */
            (pch->RxFrameNDataBytes  == (nbr_bytes + 5))) {
            ix      = 0;                                         /* Initialize COIL/loop counter variable.                   */
            data_ix = 7;                                         /* The 1st COIL data byte is 5th element in data frame.     */
            while (ix < nbr_coils) {                             /* Loop through each COIL to be forced.                     */
                if ((ix % 8) == 0) {                             /* Move to the next data byte after every eight bits.       */
                    temp = pch->RxFrameData[data_ix++];
                }
                if (temp & 0x01) {                               /* Get LSBit                                                */
                    coil_val = MODBUS_COIL_ON;
                } else {
                    coil_val = MODBUS_COIL_OFF;
                }
                MB_CoilWr(coil + ix, 
                          coil_val, 
                          &err);
                switch (err) {
                    case MODBUS_ERR_NONE:
                         break;                                  /* Continue with the next coil if no error                  */

                    case MODBUS_ERR_RANGE:
                    default:
                         pch->Err = MODBUS_ERR_FC15_01;
                         MBS_ErrRespSet(pch, 
                                        MODBUS_ERR_ILLEGAL_DATA_ADDR);
                         return (DEF_TRUE);                      /* Tell caller that we need to send a response              */
                }
                temp >>= 1;                                      /* Shift the data one bit position to the right.            */
                ix++;                                            /* Increment the COIL counter.                              */
            }
        } else {
            pch->Err = MODBUS_ERR_FC15_02;
            MBS_ErrRespSet(pch, 
                           MODBUS_ERR_ILLEGAL_DATA_VAL);
            return (DEF_TRUE);                                   /* Tell caller that we need to send a response              */
        }
        pch->TxFrameNDataBytes = 4;                              /* Don't echo the whole message back!                       */
        MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;              /* Prepare response packet                                  */
        MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
        MBS_TX_DATA_START_H    = MBS_RX_DATA_START_H;
        MBS_TX_DATA_START_L    = MBS_RX_DATA_START_L;
        MBS_TX_DATA_POINTS_H   = MBS_RX_DATA_POINTS_H;
        MBS_TX_DATA_POINTS_L   = MBS_RX_DATA_POINTS_L;
        pch->Err               = MODBUS_ERR_NONE;
    } else {
        pch->Err               = MODBUS_ERR_FC15_03;              /* Number of bytes incorrect for number of COILS.           */
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_VAL);
    }
    return (DEF_TRUE);                                            /* Tell caller that we need to send a response              */
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                    MBS_FC16_HoldingRegWrMultiple()
*
* Description : This function is called to write to multiple holding registers.  If the address of the
*               rquest exceeds or is equal to MODBUS_CFG_FP_START_IX, then the command would write to
*               multiple 'floating-point' according to the 'Daniels Flow Meter' extensions.  This means
*               that each register requested is considered as a 32-bit IEEE-754 floating-point format.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
*
* Note(s)     : none
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC16_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC16_HoldingRegWrMultiple (MODBUS_CH *pch)
{
    CPU_INT08U     *prx_data;
    CPU_INT16U      err;
    CPU_INT16U      reg;
    CPU_INT16U      reg_val_16;
    CPU_INT16U      nbr_regs;
    CPU_INT16U      nbr_bytes;
    CPU_INT08U      data_size;
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    CPU_INT08U      i;
    CPU_FP32        reg_val_fp;
    CPU_INT08U     *pfp;
#endif


    reg       = MBS_RX_DATA_START;
    nbr_regs  = MBS_RX_DATA_POINTS;
#if SE_MODBUS_CODE > 0

    MB_DATA_holdingRegWrMultiple(pch, &err);
    
    switch(err)
    {
       case MODBUS_ERR_RANGE:
          pch->Err = MODBUS_ERR_FC16_03;
          MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_ADDR);
          return (DEF_TRUE);
       case MODBUS_ERR_ILLEGAL_DATA_QTY:
          pch->Err = MODBUS_ERR_FC16_01;
          MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_QTY);
          return (DEF_TRUE);
       case MODBUS_ERR_ILLEGAL_DATA_VAL:
          pch->Err = MODBUS_ERR_FC16_02;
           MBS_ErrRespSet(pch, MODBUS_ERR_ILLEGAL_DATA_VAL);
           return (DEF_TRUE); 
       default:
          break;

    }

#else 
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
    if (reg < MODBUS_CFG_FP_START_IX) {
        if (nbr_regs == 0 || nbr_regs > 125) {                   /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC16_04;
            MBS_ErrRespSet(pch, 
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (DEF_TRUE);                                   /* Tell caller that we need to send a response              */
        }
        data_size  = sizeof(CPU_INT16U);
    } else {
        if (nbr_regs == 0 || nbr_regs > 62) {                    /* Make sure we don't exceed the allowed limit per request  */
            pch->Err = MODBUS_ERR_FC16_05;
            MBS_ErrRespSet(pch, 
                           MODBUS_ERR_ILLEGAL_DATA_QTY);
            return (DEF_TRUE);                                   /* Tell caller that we need to send a response              */
        }
        data_size  = sizeof(CPU_FP32);
    }
#else
    if (nbr_regs == 0 || nbr_regs > 125) {                       /* Make sure we don't exceed the allowed limit per request  */
        pch->Err = MODBUS_ERR_FC16_04;                    
        MBS_ErrRespSet(pch,                               
                       MODBUS_ERR_ILLEGAL_DATA_QTY);      
        return (DEF_TRUE);                                       /* Tell caller that we need to send a response              */
    }                                                     
    data_size  = sizeof(CPU_INT16U);
#endif

    prx_data  = &pch->RxFrameData[6];                            /* Point to number of bytes in request frame                */
    nbr_bytes = (CPU_INT16U)*prx_data++;
    if ((pch->RxFrameNDataBytes - 5) != nbr_bytes) {             /* Compare actual number of bytes to what they say.         */
        pch->Err = MODBUS_ERR_FC16_01;
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (DEF_TRUE);
    }
    if ((nbr_bytes / nbr_regs) != (CPU_INT16U)data_size) {
        pch->Err = MODBUS_ERR_FC16_02;
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_VAL);
        return (DEF_TRUE);                                       /* Tell caller that we need to send a response              */
    }
    while (nbr_regs > 0) {
#if (MODBUS_CFG_FP_EN == DEF_ENABLED)
        if (reg < MODBUS_CFG_FP_START_IX) {
            reg_val_16  = ((CPU_INT16U)*prx_data++) << 8;        /* Get MSB first.                                           */
            reg_val_16 +=  (CPU_INT16U)*prx_data++;              /* Add in the LSB.                                          */
            MB_HoldingRegWr(reg, 
                            reg_val_16, 
                            &err);
        } else {
            pfp = (CPU_INT08U *)&reg_val_fp;
#if CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG
            for (i = 0; i < sizeof(CPU_FP32); i++) {
                *pfp++   = *prx_data++;
            }
#else
            pfp += sizeof(CPU_FP32) - 1;
            for (i = 0; i < sizeof(CPU_FP32); i++) {
                *pfp--   = *prx_data++;
            }
#endif
            MB_HoldingRegWrFP(reg, 
                              reg_val_fp, 
                              &err);
        }
#else        
        reg_val_16  = ((CPU_INT16U)*prx_data++) << 8;            /* Get MSB first.                                           */
        reg_val_16 +=  (CPU_INT16U)*prx_data++;                  /* Add in the LSB.                                          */
        MB_HoldingRegWr(reg,                               
                        reg_val_16,                        
                        &err);                             
#endif
        
        switch (err) {                                           /* See if any errors in writing the data                    */
            case MODBUS_ERR_NONE:                                /* Reply with echoe of command received                     */
                 pch->WrCtr++;
                 reg++;
                 nbr_regs--;
                 break;

            case MODBUS_ERR_RANGE:
            default:
                 pch->Err = MODBUS_ERR_FC16_03;
                 MBS_ErrRespSet(pch, 
                                MODBUS_ERR_ILLEGAL_DATA_ADDR);
                 return (DEF_TRUE);                              /* Tell caller that we need to send a response              */
        }
    }
#endif 
    pch->TxFrameNDataBytes = 4;                                  /* Don't echo the whole message back!                       */
    MBS_TX_FRAME_ADDR      = MBS_RX_FRAME_ADDR;                  /* Prepare response packet                                  */
    MBS_TX_FRAME_FC        = MBS_RX_FRAME_FC;
    MBS_TX_DATA_START_H    = MBS_RX_DATA_START_H;
    MBS_TX_DATA_START_L    = MBS_RX_DATA_START_L;
    MBS_TX_DATA_POINTS_H   = MBS_RX_DATA_POINTS_H;
    MBS_TX_DATA_POINTS_L   = MBS_RX_DATA_POINTS_L;
    return (DEF_TRUE);                                           /* Tell caller that we need to send a response              */
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          MBS_FC20_FileRd()
*
* Description : Read a record from a file.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler()
* 
* Note(s)     : (1) The current version of this software only supports ONE Sub-request at a time.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC20_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC20_FileRd (MODBUS_CH  *pch)
{
    CPU_INT08U    *presp;
    CPU_INT16U     file_nbr;
    CPU_INT16U     record_nbr;
    CPU_INT16U     record_len;
    CPU_INT16U     cmd_len;
    CPU_INT08U     cmd_type;
    CPU_INT16U     err;
    CPU_INT16U     reg_val;
    CPU_INT16U     ix;


    cmd_len = pch->RxFrameData[2];                                           /* Get the number of bytes in the command received          */
    if (cmd_len < 7 || cmd_len > 245) {                                      /* Make sure the byte count Rx'd is within expected range   */
        pch->Err = MODBUS_ERR_FC20_01;
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (DEF_TRUE);                                                   /* Tell caller that we need to send a response              */
    }
    cmd_type    =  pch->RxFrameData[3];                                      /* Get the reference type                                   */
    file_nbr    = ((CPU_INT16U)pch->RxFrameData[4] << 8)                     /* Get the file number                                      */
                +  (CPU_INT16U)pch->RxFrameData[5];
    record_nbr  = ((CPU_INT16U)pch->RxFrameData[6] << 8)                     /* Get the record number                                    */
                +  (CPU_INT16U)pch->RxFrameData[7];
    record_len  = ((CPU_INT16U)pch->RxFrameData[8] << 8)                     /* Get the record length                                    */
                +  (CPU_INT16U)pch->RxFrameData[9];
    presp       = &pch->TxFrameData[0];                                      /* Point to first location in response buffer               */
    *presp++    = MBS_RX_FRAME_ADDR;                                         /* Reply back with the node address                         */
    *presp++    = MBS_RX_FRAME_FC;                                           /* Include the function code                                */
    if (cmd_type == 6) {                                                     /* File type should ALWAYS be 6.                            */
        pch->TxFrameNDataBytes = record_len * sizeof(CPU_INT16U) + 3;        /* Determine the total number of data bytes in the Tx frame */
        *presp++               = (CPU_INT08U)(pch->TxFrameNDataBytes - 1);   /* Total byte count (excluding byte count)                  */
        *presp++               = (CPU_INT08U)(pch->TxFrameNDataBytes - 2);   /* Sub request byte count (excluding sub-request byte cnt)  */
        *presp++               = 6;                                          /* Reference type is ALWAYS 6.                              */
        ix                     = 0;                                          /* Initialize the index into the record                     */
        while (record_len > 0) {
            reg_val = MB_FileRd(file_nbr,                                    /* Get one value from the file                              */
                                record_nbr, 
                                ix, 
                                record_len, 
                                &err); 
            switch (err) {
                case MODBUS_ERR_NONE:
                     *presp++ = (CPU_INT08U)(reg_val >> 8);                  /* Store high byte of record data                           */
                     *presp++ = (CPU_INT08U)(reg_val & 0x00FF);              /* Store low  byte of record data                           */
                     break;

                case MODBUS_ERR_FILE:
                     pch->Err = MODBUS_ERR_FC20_02;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);                                      /* Tell caller that we need to send a response              */

                case MODBUS_ERR_RECORD:
                     pch->Err = MODBUS_ERR_FC20_03;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);                                      /* Tell caller that we need to send a response              */

                case MODBUS_ERR_IX:
                default:
                     pch->Err = MODBUS_ERR_FC20_04;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);                                      /* Tell caller that we need to send a response              */
            }
            ix++;
            record_len--;
        }
    } else {
        pch->Err = MODBUS_ERR_FC20_05;
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_ADDR);
        return (DEF_TRUE);                                                   /* Tell caller that we need to send a response              */
    }
    pch->Err = MODBUS_ERR_NONE;
    return (DEF_TRUE);                                                       /* Tell caller that we need to send a response              */
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                       MBS_FC21_FileWr()
*
* Description : Write a record to a file.
*
* Argument(s) : pch       Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : DEF_TRUE      If a response needs to be sent
*               DEF_FALSE     If not
*
* Caller(s)   : MBS_FCxx_Handler().
*
* Note(s)     : (1) The current version of this software only supports ONE Sub-request at a time.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
#if (MODBUS_CFG_FC21_EN  == DEF_ENABLED)
static  CPU_BOOLEAN  MBS_FC21_FileWr (MODBUS_CH  *pch)
{
    CPU_INT08U    *prx_data;
    CPU_INT08U    *pcmd;
    CPU_INT08U    *presp;
    CPU_INT16U     file_nbr;
    CPU_INT16U     record_nbr;
    CPU_INT16U     record_len;
    CPU_INT16U     cmd_len;
    CPU_INT08U     cmd_type;
    CPU_INT16U     err;
    CPU_INT08U     max;
    CPU_INT16U     reg_val;
    CPU_INT16U     ix;


    cmd_len = pch->RxFrameData[2];
    if (cmd_len < 7 || cmd_len > 245) {                                    /* Make sure the byte count Rx'd is within expected range   */
        pch->Err = MODBUS_ERR_FC21_01;
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_QTY);
        return (DEF_TRUE);                                                 /* Tell caller that we need to send a response              */
    }
    cmd_type    =  pch->RxFrameData[3];                                    /* Get the reference type                                   */
    file_nbr    = ((CPU_INT16U)pch->RxFrameData[4] << 8)                   /* Get the file number                                      */
                +  (CPU_INT16U)pch->RxFrameData[5];
    record_nbr  = ((CPU_INT16U)pch->RxFrameData[6] << 8)                   /* Get the record number                                    */
                + (CPU_INT16U)pch->RxFrameData[7];
    record_len  = ((CPU_INT16U)pch->RxFrameData[8] << 8)                   /* Get the record length                                    */
                + (CPU_INT16U)pch->RxFrameData[9];
    prx_data    = &pch->RxFrameData[10];                                   /* Point to first data byte                                 */

    if (cmd_type == 6) {                                                   /* File type should ALWAYS be 6.                            */
        ix = 0;                                                            /* Initialize the index into the record                     */
        while (record_len > 0) {
            reg_val  = ((CPU_INT16U)*prx_data++ << 8) & 0xFF00;            /* Get data to write to file                                */
            reg_val |=  (CPU_INT16U)*prx_data++ & 0x00FF;
            MB_FileWr(file_nbr,                                            /* Write one value to the file                              */
                      record_nbr, 
                      ix, 
                      record_len, 
                      reg_val, 
                      &err);
            switch (err) {
                case MODBUS_ERR_NONE:
                     pch->WrCtr++;
                     break;

                case MODBUS_ERR_FILE:
                     pch->Err = MODBUS_ERR_FC21_02;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);                                    /* Tell caller that we need to send a response              */

                case MODBUS_ERR_RECORD:
                     pch->Err = MODBUS_ERR_FC21_03;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);                                    /* Tell caller that we need to send a response              */

                case MODBUS_ERR_IX:
                default:
                     pch->Err = MODBUS_ERR_FC21_04;
                     MBS_ErrRespSet(pch, 
                                    MODBUS_ERR_ILLEGAL_DATA_ADDR);
                     return (DEF_TRUE);                                    /* Tell caller that we need to send a response              */
            }
            ix++;
            record_len--;
        }
    } else {
        pch->Err = MODBUS_ERR_FC21_05;
        MBS_ErrRespSet(pch, 
                       MODBUS_ERR_ILLEGAL_DATA_ADDR);
    }
    record_len = ((CPU_INT16U)pch->RxFrameData[8] << 8) + (CPU_INT16U)pch->RxFrameData[9];    /* Get the record length                 */
    pcmd       = &pch->RxFrameData[0];
    presp      = &pch->TxFrameData[0];                                     /* Point to first location in response buffer               */
    max        = (record_len * 2) + 9;
    for (ix = 0; ix < max; ix++) {                                         /* Copy the request into the transmit packet                */
        *presp++ = *pcmd++;
    }
    pch->Err = MODBUS_ERR_NONE;
    return (DEF_TRUE);                                                     /* Tell caller that we need to send a response              */
}
#endif
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                            MBS_StatInit()
*
* Description : This function is used to initialize/reset the MODBUS statistics/communications counters.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Init()
*               MBS_FC08_Loopback()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_FC08_EN  == DEF_ENABLED)
void  MBS_StatInit (MODBUS_CH  *pch)
{
    pch->StatMsgCtr      = 0;                       /* Initialize all MODBUS event counters.                     */
    pch->StatCRCErrCtr   = 0;
    pch->StatExceptCtr   = 0;
    pch->StatSlaveMsgCtr = 0;
    pch->StatNoRespCtr   = 0;
}
#endif

/*
*********************************************************************************************************
*                                           MBS_RxTask()
*
* Description : Handle either Modbus ASCII or Modbus RTU received packets.
*
* Argument(s) : ch       Specifies the Modbus channel that needs servicing.
*
* Return(s)   : none.
*
* Caller(s)   : MB_RxTask()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
void  MBS_RxTask (MODBUS_CH *pch)
{
    if (pch != (MODBUS_CH *)0) {
#if (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
        if (pch->Mode == MODBUS_MODE_ASCII) {
            MBS_ASCII_Task(pch);
        }
#endif

#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
        if (pch->Mode == MODBUS_MODE_RTU) {
            MBS_RTU_Task(pch);
        }
#endif
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           MBS_ASCII_Task()
*
* Description : Received a packet that should be encoded for Modbus ASCII mode.  Process request.
*
* Argument(s) : pch         Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MBS_RxTask()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_ASCII_EN == DEF_ENABLED)
static  void  MBS_ASCII_Task (MODBUS_CH  *pch)
{
    CPU_BOOLEAN   ok;
    CPU_INT16U    calc_lrc;                               /* Used for LRC                                                    */
    CPU_BOOLEAN   send_reply;


    pch->StatMsgCtr++;
    if (pch->RxBufByteCtr >= MODBUS_ASCII_MIN_MSG_SIZE) {
        ok = MB_ASCII_Rx(pch);                            /* Extract received command from .RxBuf[] & move to .RxFrameData[] */
        if (ok == DEF_TRUE) {
            calc_lrc = MB_ASCII_RxCalcLRC(pch);           /* Calculate LRC on received ASCII packet                          */
            if (calc_lrc != pch->RxFrameCRC) {            /* If sum of all data plus received LRC is not the same            */
                pch->StatCRCErrCtr++;                     /* then the frame was not received properly.                       */
                pch->StatNoRespCtr++;
            } else {
                send_reply = MBS_FCxx_Handler(pch);       /* Execute received command and formulate a response               */
                if (send_reply == DEF_TRUE) {
                    MB_ASCII_Tx(pch);                     /* Send back reply.                                                */
					
				} else {
                    pch->StatNoRespCtr++;
                }
            }
        } else {
            pch->StatNoRespCtr++;
        }
    }
    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                            MBS_RTU_Task()
*
* Description : This function processes a packet received on the Modbus channel assuming that it's an RTU
*               packet.
*
* Argument(s) : pch      Is a pointer to the Modbus channel's data structure.
*
* Return(s)   : none.
*
* Caller(s)   : MBS_RTU_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED) && \
    (MODBUS_CFG_RTU_EN   == DEF_ENABLED)
static  void  MBS_RTU_Task (MODBUS_CH  *pch)
{
    CPU_BOOLEAN   ok;
    CPU_INT16U    calc_crc;                            /* Used for CRC                                                    */
    CPU_BOOLEAN   send_reply;


    pch->StatMsgCtr++;
    if (pch->RxBufByteCtr >= MODBUS_RTU_MIN_MSG_SIZE) {
        ok = MB_RTU_Rx(pch);                           /* Extract received command from .RxBuf[] & move to .RxFrameData[] */
        if (ok == DEF_TRUE) {
            calc_crc = MB_RTU_RxCalcCRC(pch);          /* Do our own calculation of the CRC.                              */
            if (calc_crc != pch->RxFrameCRC) {         /* If the calculated CRC does not match the CRC received,          */
                pch->StatCRCErrCtr++;                  /* then the frame is bad.                                          */
                pch->StatNoRespCtr++;
            } else {
                send_reply = MBS_FCxx_Handler(pch);    /* Execute received command and formulate a response               */
                if (send_reply == DEF_TRUE) {
                    MB_RTU_Tx(pch);                    /* Send back reply.                                                */
                } else {
                    pch->StatNoRespCtr++;
                }
            }
        }
    }
    pch->RxBufByteCtr = 0;
    pch->RxBufPtr     = &pch->RxBuf[0];
}
#endif
	 		  	 	  	 		 		   		  	  			 	  	 		 	 	 			 	 		 		 
