/*************************************************************************************
							This file is the GD32F103 
						GD seiral mpu Systick source file
*************************************************************************************/
//Include files
#include "GD32F103_Systick.h"

//Variables define
static u32 delaymscnt;

//Variables define
static S_TMR_INFO TmrTbl[eTmr_CountId];

//Local functions declare
static void GD32F103_Systick_TimerControlInit( void );

//Functions implement
/*******************************************************************************
Name:   
Func:   
Para:   
Retn:   
*******************************************************************************/
void GD32F103_Systick_Config( void )
{
    if (SysTick_Config(SystemCoreClock / 1000U))	/* setup systick timer for 1000Hz interrupts */
	{
		while (1)
		{
			//Add capture error lines
		}
	}
	GD32F103_Systick_TimerControlInit( );
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
static void GD32F103_Systick_TimerControlInit( void )
{
    uint32_t i;
    for (i = 0; i < eTmr_CountId; i ++)
    {
        TmrTbl[i].delay = 0;
        TmrTbl[i].remain = 0;
        TmrTbl[i].mFlag = 0;
        TmrTbl[i].mState = TimerNoUse;
        TmrTbl[i].pFCB = NULL;
        TmrTbl[i].pdata = NULL;
    }
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void GD32F103_Systick_Delayxms( u32 ms )
{
    delaymscnt = ms;
    
    while (delaymscnt)
    {
        //Enalbe watchdog
#ifdef USE_IDWDG
            FEED_I_WATCHDOG();
#endif
    }
}

/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void GD32F103_Systick_TimerProcess( void )
{
    uint32_t i;
    for (i = 0; i < eTmr_CountId; i ++)
    {
        if (TmrTbl[i].mFlag)
        {
            TmrTbl[i].mFlag = 0;
            
            if ((TmrTbl[i].pFCB)(TmrTbl[i].pdata))
            {
                TmrTbl[i].remain = TmrTbl[i].delay;
                TmrTbl[i].mState = TimerUsing;
            }
        }
    }
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void GD32F103_TimerSet( uint8_t tmrid, TmrCallBack pcb, void* pdat, uint32_t dlyms )
{
    if ((tmrid >= eTmr_CountId) || (!pcb) || (!dlyms))
        return;
    
    TmrTbl[tmrid].mState = TimerNoUse;
    
    TmrTbl[tmrid].pFCB = pcb;
    TmrTbl[tmrid].pdata = pdat;
    TmrTbl[tmrid].delay = dlyms;
    TmrTbl[tmrid].remain = dlyms;
    TmrTbl[tmrid].mState = TimerUsing;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void GD32F103_Systick_TimerKill( uint32_t i )
{
	TmrTbl[i].delay = 0;
    TmrTbl[i].remain = 0;
    TmrTbl[i].mFlag = 0;
    TmrTbl[i].mState = TimerNoUse;
    TmrTbl[i].pFCB = NULL;
    TmrTbl[i].pdata = NULL;
}



/***************************************************************************************
Name:   
Func:   
Para:   
Retn:   
***************************************************************************************/
void SysTick_Handler( void )
{
	//Delay x ms
    if (delaymscnt)
    {
        delaymscnt --;
    }
	//Timer
    uint32_t i;
    for (i = 0; i < eTmr_CountId; i ++)
    {
        if (TmrTbl[i].mState == TimerUsing)
        {
            if (TmrTbl[i].remain)
            {
                TmrTbl[i].remain --;
            }
            else
            {
                TmrTbl[i].mFlag = 1;
                TmrTbl[i].mState = TimerNoUse;
            }
        }
    }
}

