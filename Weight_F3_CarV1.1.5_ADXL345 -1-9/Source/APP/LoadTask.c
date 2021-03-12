/************************************************************************************************************************************************************************
** ��Ȩ��   2015-2025, ��������Ϊ�Ƽ���չ���޹�˾
** �ļ���:  LoadTask.c
** ����:    �½�
** �汾:    V1.0.0
** ����:    2015-09-30
** ����:    ����������
** ����:         
*************************************************************************************************************************************************************************
** �޸���:          No
** �汾:  		
** �޸�����:    No 
** ����:            No
*************************************************************************************************************************************************************************/

#include "LoadTask.h"

u16 g_wEmptyBuffer[LOAD_DATA_FIFO_NUM]; //����ѧϰ����FIFO
u16 g_wFullBuffer[LOAD_DATA_FIFO_NUM];  //����ѧϰ����FIFO
u8  g_cyWorkState;                       //����״̬
u8  g_cyStudyState;                    //ѧϰ״̬
u8  g_cyRecordShovelNum;                //��¼����

extern u8 g_cyCtrlFlag; //���Ʊ�׼  1��ֻ���ٶȣ� 2�����ٶȣ�����״̬ 3�����ٶȣ�����״̬���е���Χ��
extern u8 g_cyCarState; //��״̬ 0x00��ʾͣ����0x01��ʾ��ʻ
extern u8 g_cyPbState;  //��״̬ 0x00��ʾ�񲼴򿪣�0x01��ʾ�񲼹رգ�
extern u8 g_cyElState;  //����Χ��״̬ 0x00��ʾ����ͣ������0x01��ʾ����װ������0x02��ʾ����ж������0x03��ʾ��������

u16 g_wEmptyValue;                      //����ֵ
u16 g_wFullValue;                       //����ֵ
float g_fVarianceValue;                 //����ֵ
u16 g_wRatedValue;                      //�������
float g_fOverValueK;                    //����ֵϵ��

u8  g_cyCurrentShovelNum = 0;               //��ǰ����
u8 g_cyShovelDTime = 0;                  //�����֮���ӳ�ʱ��                        

#define MAX_FIFO_RECORD_NUM   10        //�ɼ�����
#define MAX_STUDY_NUM         30         //һ��״̬����¼
static u16 g_wFifoBuffer[MAX_FIFO_RECORD_NUM];      //�ɼ�����
static u8  g_cyFifoCnt;                 			//�ɼ������ۼ�
static u16 g_wCurrentStateBuffer[MAX_STUDY_NUM];    //һ��״̬��ѧϰ�ɹ�����
static u8  g_cyCurrentStateCnt;         			//һ��״̬��ѧϰ�ɹ�����
u8  g_cyOverFlag;                			//���ر�־

#define MAX_CHECK_NUM   10
static u16 g_wCheckBuffer[MAX_CHECK_NUM];          //�������
static u8 g_cyCheckCnt;                            //������

static u8 g_cyVarianceFlag = false;                 //����ֵ��־
static u8 g_cyShovelFlag = false;                   //����־
static u8 g_cyShovelWaitCnt;                       //����ʱ����
static u16  g_wUnloadCnt = 0;                        //ж�ؼ���

#define MAX_LOAD_AD_NUM 600                         //�ӳ�30�봦������
#define GET_START_DATA_NUM  10                      //��ȡ���ݿ�ʼ���
static u16 g_wLoadADBuffer[MAX_LOAD_AD_NUM];        //�ɼ�AD���ݻ���
static u16 g_wLoadADCnt = 0;                       //�ɼ�AD������

static u8 g_cyExitFlag;                            //ж�سɹ��˳���־

static u8 g_cyCheckFullFlag;                       //�������ֵ��־
static u8 g_cyRunFullCnt[2];

static u16 g_wIdleWait100Ms;

u16 g_wDlyMaxTime;      //�����ʱʱ�� 0.1S ��λ


//**************************************************************************************************
// ����         : LOAD_GetAverage()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ��ƽ��ֵ
// �������     : ���ݴ�(u16 *pWBuffer), ���ݳ���(u8 cyLen)
// �������     : ��
// ���ؽ��     : ƽ��ֵ
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

u16 LOAD_GetAverage(u16 *pWBuffer, u8 cyLen)
{
    u32 dwSum;
    u8 i;
    
    dwSum = 0;
    for (i = 0; i < cyLen; i++)
    {
        dwSum += *(pWBuffer + i);
    }
    
    dwSum /= cyLen;
    
    return ( (u16)dwSum);
}

//**************************************************************************************************
// ����         : LOAD_FifoData()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : FIFO
// �������     : ���ݴ�(u16 *pWBuffer), ����(u16 wData), ���ݳ���(u16 wLen)
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   : ���½�����һ�����ݷ�����������һ��λ��
// �޸�����     :
//**************************************************************************************************

static void LOAD_FifoData(u16 *pWBuffer, u16 wData, u16 wLen)
{
    u16 i;
    
    for (i = 0; i < (wLen - 1); i++)
    {
        *(pWBuffer + i) = *(pWBuffer + i + 1);
    }
    
    *(pWBuffer + i) = wData;
}

//**************************************************************************************************
// ����         : LOAD_GetPar()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ��ȡ����
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

void LOAD_GetPar(void)
{	
    RunVar.LoadADValue = 1;
    
    g_cyFifoCnt = 0;
    g_cyCurrentStateCnt = 0;
    g_cyCheckCnt = 0;
    
    g_cyStudyState = LOAD_NO_STUDY;
    
    if ( (0xFFFF == g_wEmptyBuffer[0]) && (0xFFFF == g_wFullBuffer[0]) )        //buffer��ֵΪ0xffff��Ϊûѧϰ��               
    {
        g_cyStudyState = LOAD_NO_STUDY;                                         //���ؿ��ض�ûѧϰ��
        g_wEmptyValue = 600;                                                    //���س�ֵ600
        g_wFullValue = 2800;                                                    //���س�ֵ2800
        return; 
    }
    
    if (0xFFFF == g_wEmptyBuffer[0])
    {
        g_cyStudyState |= LOAD_FULL_STUDY;                                      //����ûѧϰ��                                   
        g_wFullValue = LOAD_GetAverage(g_wFullBuffer, LOAD_DATA_FIFO_NUM);      //����Ϊ�ǿ���ûѧϰ��
        g_wEmptyValue = 600;                                                    //����ֵ600
        return;
    }
    
    if (0xFFFF == g_wFullBuffer[0])
    {
        g_cyStudyState |= LOAD_EMPTY_STUDY;                                     //����ûѧϰ��
        g_wEmptyValue = LOAD_GetAverage(g_wEmptyBuffer, LOAD_DATA_FIFO_NUM);    //����Ϊ������ûѧϰ��
        g_wFullValue = 2800;                                                    //����ֵ2800
        return;
    }
    
    g_cyStudyState = LOAD_STUDY;                                                //��ѧϰ��   
    g_wEmptyValue = LOAD_GetAverage(g_wEmptyBuffer, LOAD_DATA_FIFO_NUM);
    g_wFullValue = LOAD_GetAverage(g_wFullBuffer, LOAD_DATA_FIFO_NUM);	
}

//**************************************************************************************************
// ����         : GetVariance()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ��������ֵ
// �������     : ���ݴ�(u16 *pWBuffer), ���ݳ���(u8 cyLen)
// �������     : ��
// ���ؽ��     : ��������ֵ
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

//���ʽ:
//ƽ����: M = (x1 + x2 + ...+ x2)/n��n��ʾ�������ݸ�����x1��x2��x3����xn��ʾ�������ݾ�����ֵ��
//���ʽ: S = ( (x1-M)^2 + (x2-M)^2 +...(xn-M)^2 ) / (n - 1)
float GetVariance(u16 *pWBuffer, u8 cyLen)
{
    float fTemp;
    float fSum;
    u8 i;
    
    fSum = 0;
    for (i = 0; i < cyLen; i++)
    {
        fSum += *(pWBuffer + i);
    }
    
    fTemp = fSum / cyLen;
    
    fSum = 0;
    for (i = 0; i < cyLen; i++)
    {
        fSum += (*(pWBuffer + i) - fTemp) * (*(pWBuffer + i) - fTemp);
    }
    
    fSum /= (cyLen - 1);
    
    return (fSum);
}

//**************************************************************************************************
// ����         : LOAD_CheckInstallShovel()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ���װ��
// �������     : ��
// �������     : ��
// ���ؽ��     : �ɹ�(true 1, flase 0)
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

static u8 LOAD_CheckInstallShovel(void)
{
    float fTemp;
    float fTempVarianceValue;
    
    if (true == g_cyShovelFlag)                                                //����־��1����ʾװ���ɹ�         
    {
        g_cyShovelWaitCnt++;
        if (g_cyShovelDTime == g_cyShovelWaitCnt)                               //�ӳ�10�� ÿ����1 ����100ms  ÿ100ms�ɼ�һ��
        {                                                                       //g_cyShovelDTime=100    
            g_cyShovelFlag = false;                                             //���㣬���¼���
            g_cyVarianceFlag = false;
            g_cyCheckCnt = 0;
        }
        return (false);
    }
    
    if (MAX_CHECK_NUM > g_cyCheckCnt)
    { 
        g_wCheckBuffer[g_cyCheckCnt] = g_wLoadADBuffer[GET_START_DATA_NUM];     //ȡ��10��ֵ����buffer
        g_cyCheckCnt++;
    }
    else
    {                                                                           //FIFO����buffer
        LOAD_FifoData(g_wCheckBuffer, g_wLoadADBuffer[GET_START_DATA_NUM], MAX_CHECK_NUM);
    }
    
    if ( ( 2 <= g_cyCheckCnt) && ( (g_wCheckBuffer[g_cyCheckCnt - 2] + 10) < g_wCheckBuffer[g_cyCheckCnt - 1] ) )
    {
        g_cyShovelFlag = true;                                                  //��һ��ֵ��ǰһ��ֵ��10����װ�����ɹ�
        g_cyShovelWaitCnt = 0;
        return (true);
    }
    
    if ( ( 2 <= g_cyCheckCnt) && ( (g_wCheckBuffer[g_cyCheckCnt - 2] - 10) > g_wCheckBuffer[g_cyCheckCnt - 1] ) ) 
    {
        g_cyShovelFlag = true;                                                  //��һ��ֵ��ǰһ��ֵС10����ж�����ɹ�
        g_cyShovelWaitCnt = 0;
        return (true);
    }
    
    if (MAX_CHECK_NUM == g_cyCheckCnt) 
    {
        fTempVarianceValue = GetVariance(g_wCheckBuffer, g_cyCheckCnt);         //�󷽲�
        
        if (false == g_cyVarianceFlag)
        {
            if (g_fVarianceValue < fTempVarianceValue)                          //��������g_fVarianceValue=1.2
            {
                g_cyVarianceFlag = true;
            }
        }
        else
        {
            if (g_fVarianceValue > fTempVarianceValue)
            {
                fTemp = g_fVarianceValue;
                fTemp *= 0.4;
                if (fTempVarianceValue < fTemp)
                {
                    g_cyVarianceFlag = false;
                    g_cyShovelFlag = true;
                    g_cyShovelWaitCnt = 0;
                    return (true);
                }
            }
        }
    }
    return (false);
}

//**************************************************************************************************
// ����         : LOAD_HandleEmptyTask()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : �����������
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

static void LOAD_HandleEmptyTask(void)
{
    u16 wTemp;
    u8  i;
    float fTemp;
    
    if (3 == g_cyCtrlFlag) //���Ʊ�׼  1��ֻ���ٶȣ� 2�����ٶȣ�����״̬ 3�����ٶȣ�����״̬���е���Χ��
    {
        if (0x01 != g_cyElState) //����Χ��״̬ 0x00��ʾ����ͣ������0x01��ʾ����װ������0x02��ʾ����ж������0x03��ʾ��������
        {                                                                       
            return;                                                             //��������װ�����򷵻�
        }
        
        if (0x01 == g_cyPbState) //��״̬ 0x00��ʾ�񲼴򿪣�0x01��ʾ�񲼹رգ�
        {
            return;                                                             //�񲼹ر��򷵻�
        }
    }
    
    if (2 == g_cyCtrlFlag) //���Ʊ�׼  1��ֻ���ٶȣ� 2�����ٶȣ�����״̬ 3�����ٶȣ�����״̬���е���Χ��
    {
        if (0x01 == g_cyPbState) //��״̬ 0x00��ʾ�񲼴򿪣�0x01��ʾ�񲼹رգ�
        {
                return;                                                        //�񲼹ر��򷵻�
        }
    }
    
    if (CAR_STOP_STATE != g_cyCarState) //����״̬����������
    {
        g_cyFifoCnt = 0;
        g_cyShovelFlag = false;
        g_cyVarianceFlag = false;
        g_cyCheckCnt = 0;
        g_cyCurrentShovelNum = 0;
        return;
    }
    
    if (true == LOAD_CheckInstallShovel())                                    //װ���ɹ�
    {
        g_cyCurrentShovelNum++;                                                 //��ǰ����+1
        if (5 == g_cyCurrentShovelNum)                                          //3������ɹ�װ�أ�5����
        {
            if ( (LOAD_STUDY == g_cyStudyState) && (0 != g_cyRecordShovelNum) && (g_wEmptyValue < g_wFullValue) )
            {
                fTemp = g_wFullValue - g_wEmptyValue;
                fTemp /= g_cyRecordShovelNum;                                   //ÿ��������
            }
            else
            {
                fTemp = 25;                                                     //ÿ������Ĭ��25
            }
            
            fTemp *= g_cyCurrentShovelNum;
            if ( (fTemp > (g_wFullValue - g_wEmptyValue) ) && (g_wEmptyValue < g_wFullValue) )
            {
                RunVar.LoadADValue += 3;
            }
            else if (fTemp < 1000)
            {
                RunVar.LoadADValue = fTemp;
            }
            else 
            {
                RunVar.LoadADValue = 300;
            }
    
            g_cyWorkState = LOAD_INSTALL_STATE;                                 //װ��״̬
            ProductPara.bFlashWEn = FLASH_WR_ENABLE;
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_WORK_STATE, 1,  &g_cyWorkState);
            ProductPara.bFlashWEn = FLASH_WR_DISABLE;
            
            g_cyFifoCnt = 0;
            g_cyCheckCnt = 0;
            g_cyVarianceFlag = false;
                
            if (2 < g_cyCurrentStateCnt)                                        //��ѧϰ�ɹ�����
            {
                if (100 < GetVariance(g_wCurrentStateBuffer, g_cyCurrentStateCnt) )
                {
                    g_cyCurrentStateCnt = 0;
                    return;
                }
                
                wTemp = LOAD_GetAverage(g_wCurrentStateBuffer, g_cyCurrentStateCnt);
                
                if ( (wTemp > (g_wFullValue - 100) ) || (wTemp > 1500) )
                {
                    g_cyCurrentStateCnt = 0;
                    return;
                }
                        
                if (LOAD_EMPTY_STUDY != (g_cyStudyState & LOAD_EMPTY_STUDY) )   //����ֵû��ѧϰ�ɹ� ��������
                {
                    g_wEmptyValue = wTemp;
                    for (i = 0; i < LOAD_DATA_FIFO_NUM; i++)
                    {
                            g_wEmptyBuffer[i] = wTemp;
                    }
                    
                    g_cyStudyState |= LOAD_EMPTY_STUDY;
                }
                else
                {
                    if (LOAD_STUDY == g_cyStudyState)  //��ѧϰ�ɹ� ��������
                    {
                        //�¿���ֵ�������(ԭ����ֵ-ԭ������ֵ��*50% + ԭ������ֵ����ô��(ԭ����ֵ-ԭ������ֵ��*50% + ԭ������ֵ��Ϊ�µĿ���ֵ��
                        //�¿���ֵ���С��ԭ������ֵ - (ԭ����ֵ-ԭ������ֵ��* 50%����ô��ԭ������ֵ - (ԭ����ֵ-ԭ������ֵ��* 50%��Ϊ�µĿ���ֵ
                        fTemp = g_wFullValue - g_wEmptyValue;
                        fTemp *= 0.5;
                        if (wTemp > (g_wEmptyValue + fTemp) )
                        {
                            wTemp = g_wEmptyValue + (u16)fTemp;
                        }
                        else if (wTemp < (g_wEmptyValue - fTemp) )
                        {
                            wTemp = g_wEmptyValue - (u16)fTemp;
                        }       
                    }
                    LOAD_FifoData(g_wEmptyBuffer, wTemp, LOAD_DATA_FIFO_NUM);
                }
                g_wEmptyValue = LOAD_GetAverage(g_wEmptyBuffer, LOAD_DATA_FIFO_NUM);
                ProductPara.bFlashWEn = FLASH_WR_ENABLE;
                I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_EMPTY_FIFO, 20, (u8 *)&g_wEmptyBuffer);
                ProductPara.bFlashWEn = FLASH_WR_DISABLE;
            }  
            g_cyCurrentStateCnt = 0;         
            return;
        }
    }
    g_wFifoBuffer[g_cyFifoCnt] = g_wLoadADBuffer[GET_START_DATA_NUM];
    g_cyFifoCnt++;
    if (MAX_FIFO_RECORD_NUM == g_cyFifoCnt)
    {
        g_cyFifoCnt = 0;
        if (100 < GetVariance(g_wFifoBuffer, MAX_FIFO_RECORD_NUM) )
        {
            return;
        }

        wTemp = LOAD_GetAverage(g_wFifoBuffer, MAX_FIFO_RECORD_NUM);
        if (wTemp > g_wFullValue)
        {
            return;
        }
        
        if (MAX_STUDY_NUM > g_cyCurrentStateCnt)
        {
            g_wCurrentStateBuffer[g_cyCurrentStateCnt] = wTemp;
            g_cyCurrentStateCnt++;
        }
    }
}

//**************************************************************************************************
// ����         : LOAD_HandleInstallTask()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ����װ������
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

static void LOAD_HandleInstallTask(void)
{
	float fTemp;
	//u16 wTemp;
	
	if (CAR_RUN_STATE == g_cyCarState) //����
	{
		g_wIdleWait100Ms = 0;
		g_cyWorkState = LOAD_IN_FULL_STATE;
		
		ProductPara.bFlashWEn = FLASH_WR_ENABLE;
		I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_WORK_STATE, 1,  &g_cyWorkState);
		ProductPara.bFlashWEn = FLASH_WR_DISABLE;
		
		g_cyFifoCnt = 0;
		g_cyCheckCnt = 0;
		g_cyVarianceFlag = false;
		g_cyShovelFlag = false;
		g_cyExitFlag = false;
		
		fTemp = g_wFullValue - g_wEmptyValue;
		fTemp *= 0.85;
		if (fTemp < RunVar.LoadADValue)  //��������ֵ85% �Ͳ����ֵ
		{
			g_cyCheckFullFlag = true;
		}
		else
		{
			g_cyCheckFullFlag = false;
		}
		g_cyRunFullCnt[0] = 0;
		g_cyRunFullCnt[1] = 0;
		return;
	}
	
	if (MAX_FIFO_RECORD_NUM < g_cyFifoCnt)
	{
		g_wFifoBuffer[g_cyFifoCnt] = g_wLoadADBuffer[GET_START_DATA_NUM];
		g_cyFifoCnt++;
	}
	else
	{
		LOAD_FifoData(g_wFifoBuffer, g_wLoadADBuffer[GET_START_DATA_NUM], MAX_FIFO_RECORD_NUM);
	}	
	//wTemp = LOAD_GetAverage(g_wFifoBuffer, g_cyFifoCnt);
	
	if (true == LOAD_CheckInstallShovel() )  //װ���ɹ�
	{
		g_cyCurrentShovelNum++;
		
		if ( (LOAD_STUDY == g_cyStudyState) && (0 != g_cyRecordShovelNum) && (g_wEmptyValue < g_wFullValue) )
		{
			fTemp = g_wFullValue - g_wEmptyValue;
			fTemp /= g_cyRecordShovelNum;
		}
		else
		{
			fTemp = 25;
		}
			
		fTemp *= g_cyCurrentShovelNum;
		if ( (fTemp > (g_wFullValue - g_wEmptyValue) ) && (g_wEmptyValue < g_wFullValue) ) 
		{
			RunVar.LoadADValue += 3;
		}
		else if (fTemp < 1000)
		{
			RunVar.LoadADValue = fTemp;
		}	
	}
	
	/*wTemp -= g_wEmptyValue;
	
	if (wTemp > (g_wRatedValue * (1 + g_fOverValueK) ) )
	{
		g_cyOverFlag = true;
	}
	else
	{
		g_cyOverFlag = false;
	}*/
}

//**************************************************************************************************
// ����         : LOAD_CheckUnload()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ���ж��
// �������     : ��
// �������     : ��
// ���ؽ��     : �ɹ�(true 1, flase 0)
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

#define MAX_RUN_UNLOAD_NUM 10
#define MAX_UNLOAD_NUM     10
static u16 g_wUnLoadBuffer[MAX_RUN_UNLOAD_NUM];
static u8  g_cyUnLoadDataCnt = 0;
static u16 g_wOldUnLoadValue;
static u8  g_cyCUBuffer[2];

static u8 LOAD_CheckUnload(void)
{
	//u8 cyCnt;
	//u8 i;
	u16 wTemp;
	float fTemp;
	u16 wUpValue;
	u16 wDownValue;
	
	if (3 == g_cyCtrlFlag) //���Ʊ�׼  1��ֻ���ٶȣ� 2�����ٶȣ�����״̬ 3�����ٶȣ�����״̬���е���Χ��
	{
		if (0x01 == g_cyElState) //����Χ��״̬ 0x00��ʾ����ͣ������0x01��ʾ����װ������0x02��ʾ����ж������0x03��ʾ��������
		{
			return (false);
		}
		
		if (0x01 == g_cyPbState) //��״̬ 0x00��ʾ�񲼴򿪣�0x01��ʾ�񲼹رգ�
		{
			return (false);
		}
	}
	
	if (2 == g_cyCtrlFlag) //���Ʊ�׼  1��ֻ���ٶȣ� 2�����ٶȣ�����״̬ 3�����ٶȣ�����״̬���е���Χ��
	{
		if (0x01 == g_cyPbState) //��״̬ 0x00��ʾ�񲼴򿪣�0x01��ʾ�񲼹رգ�
		{
			return (false);
		}
	}
	
	g_wCheckBuffer[g_cyCheckCnt] = g_wLoadADBuffer[GET_START_DATA_NUM];
	g_cyCheckCnt++;
	if (MAX_CHECK_NUM == g_cyCheckCnt)
	{
		g_cyCheckCnt = 0;
		wTemp = LOAD_GetAverage(g_wCheckBuffer, MAX_CHECK_NUM);
		
		if (MAX_UNLOAD_NUM > g_cyUnLoadDataCnt)
		{
			g_wUnLoadBuffer[g_cyUnLoadDataCnt] = wTemp;
			g_cyUnLoadDataCnt++;
		}
		else
		{
			LOAD_FifoData(g_wUnLoadBuffer, wTemp, MAX_UNLOAD_NUM);
		}
		/*
		if (MAX_UNLOAD_NUM == g_cyUnLoadDataCnt)
		{
			cyCnt = 0;
			for (i = 0; i < MAX_UNLOAD_NUM - 1; i++)
			{
				if (g_wUnLoadBuffer[i] > g_wUnLoadBuffer[i + 1] )
				{
					cyCnt++;
				}
			}
			
			if (5 <= cyCnt)
			{
				return (true);
			}
		}*/
		
		if (1 < g_cyUnLoadDataCnt)
		{
			if ( (g_wUnLoadBuffer[g_cyUnLoadDataCnt - 2] + 5) > g_wUnLoadBuffer[g_cyUnLoadDataCnt - 1] )
			{
				g_cyCUBuffer[1]++;
			}
			else if ( (g_wUnLoadBuffer[g_cyUnLoadDataCnt - 2] + 5) < g_wUnLoadBuffer[g_cyUnLoadDataCnt - 1] )
			{
				g_cyCUBuffer[0]++;
			}
			
			
			if ( (4 <= g_cyCUBuffer[1]) && (3 <= g_cyCUBuffer[0]) )
			{
				return (true);
			}
		} 
		
		//С��(����ֵ + (����ֵ �C ����ֵ) * 30%)
		//����(����ֵ + (����ֵ �C ����ֵ) * 20%)
		fTemp = g_wFullValue - g_wEmptyValue;
		wUpValue = (u16)(g_wEmptyValue + fTemp * 0.3);
		wDownValue = (u16)(g_wEmptyValue - fTemp * 0.2);
			
		if ( (wDownValue < wTemp) && (wTemp < wUpValue) )
		{
			g_wUnloadCnt++;
			if (1 < g_wUnloadCnt)
			{
				if ( ( (g_wOldUnLoadValue - 3) < wTemp) && (wTemp < (g_wOldUnLoadValue + 3) ) )  //б��ͣ��
				{
					g_wUnloadCnt = 0;
				}
			}
			
			g_wOldUnLoadValue = wTemp;
			
			if (4 == g_wUnloadCnt)  //����4���Ӷ�С��50% �ж�ж�سɹ�
			{
				return (true);
			}
		}
		else
		{
			g_wUnloadCnt = 0;
		}
	}
	
	return (false);
}


//**************************************************************************************************
// ����         : LOAD_HandleSuccessUnload()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ж�سɹ�����
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

static void LOAD_HandleSuccessUnload(void)
{
	u16 wTemp;
	u8  i;
	float fTemp;
		
	g_cyExitFlag = true;
	
	if (2 < g_cyCurrentStateCnt) //��ѧϰ�ɹ�����
	{
		if (100 < GetVariance(g_wCurrentStateBuffer, g_cyCurrentStateCnt) )
		{
			return;
		}
			
		wTemp = LOAD_GetAverage(g_wCurrentStateBuffer, g_cyCurrentStateCnt);
		if ( (g_wEmptyValue + 100) > wTemp )
		{
			return;
		}
		
		if (LOAD_FULL_STUDY != (g_cyStudyState & LOAD_FULL_STUDY) ) //����ֵû��ѧϰ�ɹ� ��������
		{
			g_wFullValue = wTemp;
			for (i = 0; i < LOAD_DATA_FIFO_NUM; i++)
			{
				g_wFullBuffer[i] = wTemp;
			}
				
			g_cyStudyState |= LOAD_FULL_STUDY;
			ProductPara.bFlashWEn = FLASH_WR_ENABLE;
			I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_FULL_FIFO, 20,  (u8 *)&g_wFullBuffer);
			ProductPara.bFlashWEn = FLASH_WR_DISABLE;
		}
		else
		{
			if (LOAD_STUDY == g_cyStudyState)  //��ѧϰ�ɹ� ��������
			{
				//������ֵ�������(ԭ����ֵ-ԭ������ֵ��*50% + ԭ������ֵ����ô��(ԭ����ֵ-ԭ������ֵ��*50% + ԭ������ֵ��Ϊ�µ�����ֵ��
				//������ֵ���С��ԭ������ֵ - (ԭ����ֵ-ԭ������ֵ��* 50%����ô��ԭ������ֵ - (ԭ����ֵ-ԭ������ֵ��* 50%��Ϊ�µ�����ֵ
				fTemp = g_wFullValue - g_wEmptyValue;
				fTemp *= 0.5;
				if (wTemp > (g_wFullValue + fTemp) )
				{
					wTemp = g_wFullValue + (u16)fTemp;
				}
				else if (wTemp < (g_wFullValue - fTemp) )
				{
					wTemp = g_wFullValue - (u16)fTemp;
				}					
			}
				
			//if (false == g_cyOverFlag) //������
			//{
				LOAD_FifoData(g_wFullBuffer, wTemp, LOAD_DATA_FIFO_NUM);
				g_wFullValue = LOAD_GetAverage(g_wFullBuffer, LOAD_DATA_FIFO_NUM);
				ProductPara.bFlashWEn = FLASH_WR_ENABLE;
				I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_FULL_FIFO, 20,  (u8 *)&g_wFullBuffer);
				ProductPara.bFlashWEn = FLASH_WR_DISABLE;
			//}
		}
	} 
}

//**************************************************************************************************
// ����         : LOAD_CheckRunUnload()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ���������ж��
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

static u8 g_cyFifiCntRun;
static u8  g_cyRunUnLoadDataCnt;
static u8  g_cyRunUnloadCnt;

void LOAD_CheckRunUnload(void)
{
	u16 wTemp;
	float fTemp;
	u16 wUpValue;
	u16 wDownValue;
	
	g_wFifoBuffer[g_cyFifiCntRun] = g_wLoadADBuffer[GET_START_DATA_NUM];
	g_cyFifiCntRun++;
	if (MAX_FIFO_RECORD_NUM == g_cyFifiCntRun)
	{
		g_cyFifiCntRun = 0;
		
		if (300 < GetVariance(g_wFifoBuffer, MAX_FIFO_RECORD_NUM) )
		{
			g_cyRunUnloadCnt = 0;
			return;
		}
		
		g_wUnLoadBuffer[g_cyRunUnLoadDataCnt] = LOAD_GetAverage(g_wFifoBuffer, MAX_FIFO_RECORD_NUM);
		g_cyRunUnLoadDataCnt++;
		if (MAX_RUN_UNLOAD_NUM == g_cyRunUnLoadDataCnt)
		{
			g_cyRunUnLoadDataCnt = 0;
			if (300 < GetVariance(g_wUnLoadBuffer, MAX_RUN_UNLOAD_NUM) )
			{
				g_cyRunUnloadCnt = 0;
				return;
			}
			
			//С��(����ֵ + (����ֵ �C ����ֵ) * 30%)
			//����(����ֵ + (����ֵ �C ����ֵ) * 20%)
			wTemp = LOAD_GetAverage(g_wUnLoadBuffer, MAX_RUN_UNLOAD_NUM);
			fTemp = g_wFullValue - g_wEmptyValue;
			wUpValue = (u16)(g_wEmptyValue + fTemp * 0.3);
			wDownValue = (u16)(g_wEmptyValue - fTemp * 0.2);
			
			if ( (wDownValue < wTemp) && (wTemp < wUpValue) )
			{
				g_cyRunUnloadCnt++;
				if (5 == g_cyRunUnloadCnt)  //����5���Ӷ��������� �ж�ж�سɹ�
				{
					LOAD_HandleSuccessUnload();
				}
			}
			else
			{
				g_cyRunUnloadCnt = 0;
			}
		}
	}
}

//**************************************************************************************************
// ����         : LOAD_CheckFullValue()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : �������������ֵ
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

#define MAX_CHECK_FULL  10
static u16 g_wRunFull[2][MAX_CHECK_FULL];          //�������

void LOAD_CheckFullValue(void)
{
	u16 wTemp;
	float fTemp;
	
	if (true == g_cyCheckFullFlag)
	{
		return;
	}
	
	g_wRunFull[0][g_cyRunFullCnt[0]] = g_wLoadADBuffer[GET_START_DATA_NUM];
	g_cyRunFullCnt[0]++;
	if (MAX_CHECK_FULL == g_cyRunFullCnt[0])
	{
		g_cyRunFullCnt[0] = 0;
		
		if (300 < GetVariance((u16 *)&g_wRunFull[0][0], MAX_CHECK_FULL) )
		{
			return;
		}
		
		wTemp = LOAD_GetAverage((u16 *)&g_wRunFull[0][0], MAX_CHECK_FULL);
		if (MAX_CHECK_FULL > g_cyRunFullCnt[1])
		{
			g_wRunFull[1][g_cyRunFullCnt[1]] = wTemp;
			g_cyRunFullCnt[1]++;
		}
		else
		{
			LOAD_FifoData((u16 *)&g_wRunFull[1][0], wTemp, MAX_CHECK_FULL);
		}
		
		if (MAX_CHECK_FULL == g_cyRunFullCnt[1])
		{
			if (300 < GetVariance((u16 *)&g_wRunFull[1][0], MAX_CHECK_FULL) )
			{
				//g_cyRunFullCnt[0] = 0;
				return;
			}
			
			wTemp = LOAD_GetAverage((u16 *)&g_wRunFull[1][0], MAX_CHECK_FULL);
			if (g_wEmptyValue < wTemp)
			{
				wTemp -= g_wEmptyValue;
				fTemp = g_wFullValue - g_wEmptyValue;
				fTemp *= 0.85;
				if (wTemp > fTemp)
				{
					RunVar.LoadADValue = wTemp;
					g_cyCheckFullFlag = true;
				}
				
				/*fTemp = g_wFullValue - g_wEmptyValue;
				fTemp *= 0.25;
				if (wTemp < fTemp)   //ж�سɹ������Ҳ���¼����
				{
					g_cyExitFlag = true;
					g_cyOverFlag = true;
				}*/
			}
			/*else    //ж�سɹ������Ҳ���¼����
			{
				g_cyExitFlag = true;
				g_cyOverFlag = true;
			}*/
		}
	}
}

//**************************************************************************************************
// ����         : LOAD_HandleFullTask()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ������������
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

static void LOAD_HandleFullTask(void)
{
	u16 wTemp;
	float fTemp;
	
	if (CAR_RUN_STATE == g_cyCarState) //����
	{
		g_cyFifoCnt = 0;
		g_wUnloadCnt = 0;
		g_cyCheckCnt = 0;
		g_cyUnLoadDataCnt = 0;
		g_cyCUBuffer[0] = 0;
		g_cyCUBuffer[1] = 0;
		if (true == g_cyExitFlag)
		{
			g_cyExitFlag = false;
			RunVar.LoadADValue = 1;
			
			//if ( (false == g_cyOverFlag) && (g_cyCurrentShovelNum > 5) ) //5�����²�����¼
			if (g_cyCurrentShovelNum > 8)  //5�����²�����¼
			{
				g_cyRecordShovelNum = g_cyCurrentShovelNum;
				ProductPara.bFlashWEn = FLASH_WR_ENABLE;
				I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_SHOVEL_NUM, 1,  &g_cyRecordShovelNum);
				ProductPara.bFlashWEn = FLASH_WR_DISABLE;
			}
			
			
			g_cyFifoCnt = 0;
			g_cyCurrentStateCnt = 0;
			g_cyCheckCnt = 0;
			g_cyCurrentShovelNum = 0;
			g_cyWorkState = LOAD_IDLE_STATE;
			g_wIdleWait100Ms = 0;
			
			ProductPara.bFlashWEn = FLASH_WR_ENABLE;
			I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_WORK_STATE, 1,  &g_cyWorkState);
			ProductPara.bFlashWEn = FLASH_WR_DISABLE;
		}
		/*else
		{
			//LOAD_CheckRunUnload();
			//LOAD_CheckFullValue();
		}*/
		return;
	}
	
	if (true == g_cyExitFlag)
	{
		return;
	}
	
	g_cyFifiCntRun = 0;
	g_cyRunUnLoadDataCnt = 0;
	g_cyRunUnloadCnt = 0;
	
	g_cyRunFullCnt[0] = 0;
	g_cyRunFullCnt[1] = 0;
	
	if (true == LOAD_CheckUnload() )  //ж�سɹ�
	{
		LOAD_HandleSuccessUnload();
		return;
	}
	
	g_wFifoBuffer[g_cyFifoCnt] = g_wLoadADBuffer[GET_START_DATA_NUM];
	g_cyFifoCnt++;
	if (MAX_FIFO_RECORD_NUM == g_cyFifoCnt)
	{
		g_cyFifoCnt = 0;
		
		if (100 < GetVariance(g_wFifoBuffer, MAX_FIFO_RECORD_NUM) )
		{
			return;
		}
		
		wTemp = LOAD_GetAverage(g_wFifoBuffer, MAX_FIFO_RECORD_NUM);
		if (wTemp < g_wEmptyValue)
		{
			return;
		}
		
		if (MAX_STUDY_NUM > g_cyCurrentStateCnt)
		{
			g_wCurrentStateBuffer[g_cyCurrentStateCnt] = LOAD_GetAverage(g_wFifoBuffer, MAX_FIFO_RECORD_NUM);
			g_cyCurrentStateCnt++;
		}
		
		if (10 > g_cyCurrentStateCnt)  //10��ƽ��ֵ
		{
			return;
		} 
		
		wTemp = LOAD_GetAverage(g_wCurrentStateBuffer, g_cyCurrentStateCnt);
		wTemp -= g_wEmptyValue;
		
		/*if (wTemp  > (g_wRatedValue * (1 + g_fOverValueK) ) )
		{
			g_cyOverFlag = true;
		}
		else
		{
			g_cyOverFlag = false;
		}*/
		
		if (false == g_cyCheckFullFlag)
		{
			fTemp = g_wFullValue - g_wEmptyValue;
			fTemp *= 0.85;
			if ( (wTemp > fTemp) && (wTemp < 1000) )
			{
				RunVar.LoadADValue = wTemp;
				g_cyCheckFullFlag = true;
			}
		}
	}
}

//**************************************************************************************************
// ����         : LOAD_HandleInFullTask()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ����װ�ص���������
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

static void LOAD_HandleInFullTask(void)
{
	u16 wTemp;
	float fTemp;
	
	if (CAR_RUN_STATE == g_cyCarState) //����
	{
		g_cyFifoCnt = 0;
		return;
	}
	
	g_wFifoBuffer[g_cyFifoCnt] = g_wLoadADBuffer[GET_START_DATA_NUM];
	g_cyFifoCnt++;
	if (MAX_FIFO_RECORD_NUM == g_cyFifoCnt)
	{
		g_cyFifoCnt = 0;
		
		if (100 < GetVariance(g_wFifoBuffer, MAX_FIFO_RECORD_NUM) )
		{
			return;
		}
		
		wTemp = LOAD_GetAverage(g_wFifoBuffer, MAX_FIFO_RECORD_NUM);
		if (wTemp < g_wEmptyValue)
		{
			return;
		}
		
		if (MAX_STUDY_NUM > g_cyCurrentStateCnt)
		{
			g_wCurrentStateBuffer[g_cyCurrentStateCnt] = LOAD_GetAverage(g_wFifoBuffer, MAX_FIFO_RECORD_NUM);
			g_cyCurrentStateCnt++;
		}
		
		if (10 > g_cyCurrentStateCnt)  //10��ƽ��ֵ
		{
			return;
		} 
		
		wTemp = LOAD_GetAverage(g_wCurrentStateBuffer, g_cyCurrentStateCnt);
		wTemp -= g_wEmptyValue;
		
		if (false == g_cyCheckFullFlag)
		{
			fTemp = g_wFullValue - g_wEmptyValue;
			fTemp *= 0.85;
			if ( (wTemp > fTemp) && (wTemp < 1000) )
			{
				RunVar.LoadADValue = wTemp;
				g_cyCheckFullFlag = true;
			}
		}
	}
}


//**************************************************************************************************
// ����         : LOAD_LoadTask()
// ��������     : 2015-09-30
// ����         : �½�
// ����         : ������������
// �������     : ��
// �������     : ��
// ���ؽ��     : ��
// ע���˵��   :
// �޸�����     :
//**************************************************************************************************

void LOAD_LoadTask(void)
{
    if (MAX_LOAD_AD_NUM > g_wLoadADCnt)  //�������ͳ���״̬������Ҫ1��(100msһ�Σ��ɼ�600����Ҫ1����)
    {
        g_wLoadADBuffer[g_wLoadADCnt] = RunVar.uiAI[0];
        g_wLoadADCnt++;
        return;
    }
    else
    {
        LOAD_FifoData(g_wLoadADBuffer, RunVar.uiAI[0], MAX_LOAD_AD_NUM);        //�ɼ���600���Ժ����FIFO��ʽ������
    }
    
    if (LOAD_EMPTY_STATE == g_cyWorkState)                                      //����״̬
    {
        LOAD_HandleEmptyTask();                                                 //�����������
    }
    else if (LOAD_INSTALL_STATE == g_cyWorkState)                              //װ��״̬
    {
        LOAD_HandleInstallTask();                                               //����װ������
    }
    else if (LOAD_IN_FULL_STATE == g_cyWorkState)                              //װ�ص�����״̬
    {
        g_wIdleWait100Ms++;
        if (g_wDlyMaxTime <= g_wIdleWait100Ms)
        {
            g_cyWorkState = LOAD_FULL_STATE;
            ProductPara.bFlashWEn = FLASH_WR_ENABLE;
            
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_WORK_STATE, 1,  &g_cyWorkState);
            
            ProductPara.bFlashWEn = FLASH_WR_DISABLE;
            
            g_cyUnLoadDataCnt = 0;
            g_cyCUBuffer[0] = 0;
            g_cyCUBuffer[1] = 0;
        }
        LOAD_HandleInFullTask();                                                //����װ�ص�����״̬
    }
    else if (LOAD_FULL_STATE == g_cyWorkState)                                 //����״̬
    {
        LOAD_HandleFullTask();                                                  //������������
    }
    else
    {
        g_cyWorkState = LOAD_IDLE_STATE;                                        //����״̬
        g_wIdleWait100Ms++;
        if (g_wDlyMaxTime <= g_wIdleWait100Ms)
        {
            g_cyWorkState = LOAD_EMPTY_STATE;
            ProductPara.bFlashWEn = FLASH_WR_ENABLE;
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_WORK_STATE, 1,  &g_cyWorkState);
            ProductPara.bFlashWEn = FLASH_WR_DISABLE;
        }
    }
}


