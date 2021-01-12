#include "BSP.h"

DADBG_TypeDef DAOilDebug = {Bit_RESET, 0};
DADBG_TypeDef DAOutDebug = {Bit_RESET, 0};

void DA_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    DAC_InitTypeDef   DAC_InitStructure;
    DAC_DeInit();

    /* DAC Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    /* GPIOA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    /* Configure PA.04 (DAC_OUT1) as analog */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits11_0;

    /* DAC Channel1 Init */
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);

    /* Enable DAC Channel1 */
    DAC_Cmd(DAC_Channel_1, ENABLE);


 	/* DA2 */
	
    /* Configure PA.05 (DAC_OUT2) as analog */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits11_0;    

    /* DAC Channel1 Init */
    DAC_Init(DAC_Channel_2, &DAC_InitStructure);

    /* Enable DAC Channel1 */
    DAC_Cmd(DAC_Channel_2, ENABLE);
}

void DA_Write(unsigned short DACode)
{
    if(DACode > 4095)
        DACode = 4095;
	
    DAC_SetChannel1Data(DAC_Align_12b_R, DACode);
    DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);
	//DAC_SetDualChannelData(DAC_Align_12b_R, DACode, DACode);
}

void DA2_Write(unsigned short DACode)
{
    if(DACode > 4095)
        DACode = 4095;	
    DAC_SetChannel2Data(DAC_Align_12b_R, DACode);
    DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
	//DAC_SetDualChannelData(DAC_Align_12b_R, DACode, DACode);
}

