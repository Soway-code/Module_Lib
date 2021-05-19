/*************************************************************************************
							This file is the GD32F103 
						GD seiral mpu Systick header file
*************************************************************************************/
#ifndef __GD32F103_SYSTICK_H
#define __GD32F103_SYSTICK_H
//Includes
#include "Includes.h"

//Datatype define
enum{
    TimerNoUse = 0,
    TimerUsing
};

//This segment defined user data
typedef enum{
	eTmr_ReadPressTimer = 0,
	eTmr_ReadTempTimer,
	eTmr_ReadAccTimer,
	eTmr_AutoLoadTimer,
    //Todo: Add user timer id start with eTmr_
    eTmr_ModbusTimer,
    eTmr_LedBlinkTimer,
    
    //user timer id
    eTmr_CountId
}E_TMR_TAG;

typedef bool (*TmrCallBack)(void *);
typedef struct{
    uint8_t          mState; //Current id timer statu 0:stopped  1:running
    uint8_t          mFlag;  //Timer timeout flag 1:time out
    uint32_t         delay;  //Delay time value
    uint32_t         remain; //Remain delay value
    void*       pdata;  //User parameter
    TmrCallBack pFCB;   //Call back function pointer
}S_TMR_INFO, *PS_TMR_INFO;



//Functions declare
void GD32F103_Systick_Config( void );
void GD32F103_Systick_TimerProcess( void );
void GD32F103_Systick_Delayxms( u32 ms );
void GD32F103_Systick_TimerKill( uint32_t i );
void GD32F103_TimerSet( uint8_t tmrid, TmrCallBack pcb, void* pdat, uint32_t dlyms );
#endif


