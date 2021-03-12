/************************************************************************************************************************************************************************
** 版权：   2015-2025, 深圳市信为科技发展有限公司
** 文件名:  LoadTask.c
** 作者:    陈锦
** 版本:    V1.0.0
** 日期:    2015-09-30
** 描述:    载重任务处理
** 功能:         
*************************************************************************************************************************************************************************
** 修改者:          No
** 版本:  		
** 修改内容:    No 
** 日期:            No
*************************************************************************************************************************************************************************/

#include "LoadTask.h"

u16 g_wEmptyBuffer[LOAD_DATA_FIFO_NUM]; //空载学习保存FIFO
u16 g_wFullBuffer[LOAD_DATA_FIFO_NUM];  //满载学习保存FIFO
u8  g_cyWorkState;                       //工作状态
u8  g_cyStudyState;                    //学习状态
u8  g_cyRecordShovelNum;                //记录铲数

extern u8 g_cyCtrlFlag; //控制标准  1：只有速度， 2：有速度，有篷布状态 3：有速度，有篷布状态，有电子围栏
extern u8 g_cyCarState; //车状态 0x00表示停车，0x01表示行驶
extern u8 g_cyPbState;  //篷布状态 0x00表示篷布打开，0x01表示篷布关闭；
extern u8 g_cyElState;  //电子围栏状态 0x00表示车辆停车场，0x01表示渣土装载区，0x02表示渣土卸载区，0x03表示其他区域

u16 g_wEmptyValue;                      //空载值
u16 g_wFullValue;                       //满载值
float g_fVarianceValue;                 //方差值
u16 g_wRatedValue;                      //额定满量程
float g_fOverValueK;                    //超载值系数

u8  g_cyCurrentShovelNum = 0;               //当前铲数
u8 g_cyShovelDTime = 0;                  //铲与铲之间延迟时间                        

#define MAX_FIFO_RECORD_NUM   10        //采集数据
#define MAX_STUDY_NUM         30         //一次状态最大记录
static u16 g_wFifoBuffer[MAX_FIFO_RECORD_NUM];      //采集数据
static u8  g_cyFifoCnt;                 			//采集数据累计
static u16 g_wCurrentStateBuffer[MAX_STUDY_NUM];    //一次状态的学习成功数据
static u8  g_cyCurrentStateCnt;         			//一次状态的学习成功次数
u8  g_cyOverFlag;                			//超载标志

#define MAX_CHECK_NUM   10
static u16 g_wCheckBuffer[MAX_CHECK_NUM];          //检测数据
static u8 g_cyCheckCnt;                            //检测计数

static u8 g_cyVarianceFlag = false;                 //方差值标志
static u8 g_cyShovelFlag = false;                   //铲标志
static u8 g_cyShovelWaitCnt;                       //铲延时计数
static u16  g_wUnloadCnt = 0;                        //卸载计数

#define MAX_LOAD_AD_NUM 600                         //延迟30秒处理数据
#define GET_START_DATA_NUM  10                      //获取数据开始编号
static u16 g_wLoadADBuffer[MAX_LOAD_AD_NUM];        //采集AD数据缓存
static u16 g_wLoadADCnt = 0;                       //采集AD计算器

static u8 g_cyExitFlag;                            //卸载成功退出标志

static u8 g_cyCheckFullFlag;                       //检测满载值标志
static u8 g_cyRunFullCnt[2];

static u16 g_wIdleWait100Ms;

u16 g_wDlyMaxTime;      //最大延时时间 0.1S 单位


//**************************************************************************************************
// 名称         : LOAD_GetAverage()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 求平均值
// 输入参数     : 数据串(u16 *pWBuffer), 数据长度(u8 cyLen)
// 输出参数     : 无
// 返回结果     : 平均值
// 注意和说明   :
// 修改内容     :
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
// 名称         : LOAD_FifoData()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : FIFO
// 输入参数     : 数据串(u16 *pWBuffer), 数据(u16 wData), 数据长度(u16 wLen)
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   : 把新进来的一个数据放在数组的最后一个位置
// 修改内容     :
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
// 名称         : LOAD_GetPar()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 获取参数
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

void LOAD_GetPar(void)
{	
    RunVar.LoadADValue = 1;
    
    g_cyFifoCnt = 0;
    g_cyCurrentStateCnt = 0;
    g_cyCheckCnt = 0;
    
    g_cyStudyState = LOAD_NO_STUDY;
    
    if ( (0xFFFF == g_wEmptyBuffer[0]) && (0xFFFF == g_wFullBuffer[0]) )        //buffer中值为0xffff即为没学习过               
    {
        g_cyStudyState = LOAD_NO_STUDY;                                         //满载空载都没学习过
        g_wEmptyValue = 600;                                                    //空载初值600
        g_wFullValue = 2800;                                                    //满载初值2800
        return; 
    }
    
    if (0xFFFF == g_wEmptyBuffer[0])
    {
        g_cyStudyState |= LOAD_FULL_STUDY;                                      //满载没学习过                                   
        g_wFullValue = LOAD_GetAverage(g_wFullBuffer, LOAD_DATA_FIFO_NUM);      //我认为是空载没学习过
        g_wEmptyValue = 600;                                                    //赋初值600
        return;
    }
    
    if (0xFFFF == g_wFullBuffer[0])
    {
        g_cyStudyState |= LOAD_EMPTY_STUDY;                                     //空载没学习过
        g_wEmptyValue = LOAD_GetAverage(g_wEmptyBuffer, LOAD_DATA_FIFO_NUM);    //我认为是满载没学习过
        g_wFullValue = 2800;                                                    //赋初值2800
        return;
    }
    
    g_cyStudyState = LOAD_STUDY;                                                //都学习过   
    g_wEmptyValue = LOAD_GetAverage(g_wEmptyBuffer, LOAD_DATA_FIFO_NUM);
    g_wFullValue = LOAD_GetAverage(g_wFullBuffer, LOAD_DATA_FIFO_NUM);	
}

//**************************************************************************************************
// 名称         : GetVariance()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 计数方差值
// 输入参数     : 数据串(u16 *pWBuffer), 数据长度(u8 cyLen)
// 输出参数     : 无
// 返回结果     : 计数方差值
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

//方差公式:
//平均数: M = (x1 + x2 + ...+ x2)/n（n表示这组数据个数，x1、x2、x3……xn表示这组数据具体数值）
//方差公式: S = ( (x1-M)^2 + (x2-M)^2 +...(xn-M)^2 ) / (n - 1)
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
// 名称         : LOAD_CheckInstallShovel()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 检测装车
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 成功(true 1, flase 0)
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

static u8 LOAD_CheckInstallShovel(void)
{
    float fTemp;
    float fTempVarianceValue;
    
    if (true == g_cyShovelFlag)                                                //铲标志置1，表示装铲成功         
    {
        g_cyShovelWaitCnt++;
        if (g_cyShovelDTime == g_cyShovelWaitCnt)                               //延迟10秒 每增加1 就是100ms  每100ms采集一次
        {                                                                       //g_cyShovelDTime=100    
            g_cyShovelFlag = false;                                             //清零，重新检测铲
            g_cyVarianceFlag = false;
            g_cyCheckCnt = 0;
        }
        return (false);
    }
    
    if (MAX_CHECK_NUM > g_cyCheckCnt)
    { 
        g_wCheckBuffer[g_cyCheckCnt] = g_wLoadADBuffer[GET_START_DATA_NUM];     //取第10个值放入buffer
        g_cyCheckCnt++;
    }
    else
    {                                                                           //FIFO放入buffer
        LOAD_FifoData(g_wCheckBuffer, g_wLoadADBuffer[GET_START_DATA_NUM], MAX_CHECK_NUM);
    }
    
    if ( ( 2 <= g_cyCheckCnt) && ( (g_wCheckBuffer[g_cyCheckCnt - 2] + 10) < g_wCheckBuffer[g_cyCheckCnt - 1] ) )
    {
        g_cyShovelFlag = true;                                                  //后一个值比前一个值大10，（装）铲成功
        g_cyShovelWaitCnt = 0;
        return (true);
    }
    
    if ( ( 2 <= g_cyCheckCnt) && ( (g_wCheckBuffer[g_cyCheckCnt - 2] - 10) > g_wCheckBuffer[g_cyCheckCnt - 1] ) ) 
    {
        g_cyShovelFlag = true;                                                  //后一个值比前一个值小10，（卸）铲成功
        g_cyShovelWaitCnt = 0;
        return (true);
    }
    
    if (MAX_CHECK_NUM == g_cyCheckCnt) 
    {
        fTempVarianceValue = GetVariance(g_wCheckBuffer, g_cyCheckCnt);         //求方差
        
        if (false == g_cyVarianceFlag)
        {
            if (g_fVarianceValue < fTempVarianceValue)                          //给定方差g_fVarianceValue=1.2
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
// 名称         : LOAD_HandleEmptyTask()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 处理空载任务
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

static void LOAD_HandleEmptyTask(void)
{
    u16 wTemp;
    u8  i;
    float fTemp;
    
    if (3 == g_cyCtrlFlag) //控制标准  1：只有速度， 2：有速度，有篷布状态 3：有速度，有篷布状态，有电子围栏
    {
        if (0x01 != g_cyElState) //电子围栏状态 0x00表示车辆停车场，0x01表示渣土装载区，0x02表示渣土卸载区，0x03表示其他区域
        {                                                                       
            return;                                                             //不在渣土装载区则返回
        }
        
        if (0x01 == g_cyPbState) //篷布状态 0x00表示篷布打开，0x01表示篷布关闭；
        {
            return;                                                             //篷布关闭则返回
        }
    }
    
    if (2 == g_cyCtrlFlag) //控制标准  1：只有速度， 2：有速度，有篷布状态 3：有速度，有篷布状态，有电子围栏
    {
        if (0x01 == g_cyPbState) //篷布状态 0x00表示篷布打开，0x01表示篷布关闭；
        {
                return;                                                        //篷布关闭则返回
        }
    }
    
    if (CAR_STOP_STATE != g_cyCarState) //开车状态，数据清零
    {
        g_cyFifoCnt = 0;
        g_cyShovelFlag = false;
        g_cyVarianceFlag = false;
        g_cyCheckCnt = 0;
        g_cyCurrentShovelNum = 0;
        return;
    }
    
    if (true == LOAD_CheckInstallShovel())                                    //装铲成功
    {
        g_cyCurrentShovelNum++;                                                 //当前铲数+1
        if (5 == g_cyCurrentShovelNum)                                          //3铲才算成功装载（5铲）
        {
            if ( (LOAD_STUDY == g_cyStudyState) && (0 != g_cyRecordShovelNum) && (g_wEmptyValue < g_wFullValue) )
            {
                fTemp = g_wFullValue - g_wEmptyValue;
                fTemp /= g_cyRecordShovelNum;                                   //每铲的重量
            }
            else
            {
                fTemp = 25;                                                     //每铲重量默认25
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
    
            g_cyWorkState = LOAD_INSTALL_STATE;                                 //装载状态
            ProductPara.bFlashWEn = FLASH_WR_ENABLE;
            I2C1_WNBytesMul3T(EEPROM_ADDRESS, LOAD_WORK_STATE, 1,  &g_cyWorkState);
            ProductPara.bFlashWEn = FLASH_WR_DISABLE;
            
            g_cyFifoCnt = 0;
            g_cyCheckCnt = 0;
            g_cyVarianceFlag = false;
                
            if (2 < g_cyCurrentStateCnt)                                        //有学习成功数据
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
                        
                if (LOAD_EMPTY_STUDY != (g_cyStudyState & LOAD_EMPTY_STUDY) )   //空载值没有学习成功 不受限制
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
                    if (LOAD_STUDY == g_cyStudyState)  //都学习成功 不受限制
                    {
                        //新空载值如果大于(原满载值-原来空载值）*50% + 原来空载值，那么按(原满载值-原来空载值）*50% + 原来空载值作为新的空载值。
                        //新空载值如果小于原来空载值 - (原满载值-原来空载值）* 50%，那么按原来空载值 - (原满载值-原来空载值）* 50%作为新的空载值
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
// 名称         : LOAD_HandleInstallTask()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 处理装载任务
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

static void LOAD_HandleInstallTask(void)
{
	float fTemp;
	//u16 wTemp;
	
	if (CAR_RUN_STATE == g_cyCarState) //开车
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
		if (fTemp < RunVar.LoadADValue)  //大于满载值85% 就不检查值
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
	
	if (true == LOAD_CheckInstallShovel() )  //装铲成功
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
// 名称         : LOAD_CheckUnload()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 检测卸载
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 成功(true 1, flase 0)
// 注意和说明   :
// 修改内容     :
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
	
	if (3 == g_cyCtrlFlag) //控制标准  1：只有速度， 2：有速度，有篷布状态 3：有速度，有篷布状态，有电子围栏
	{
		if (0x01 == g_cyElState) //电子围栏状态 0x00表示车辆停车场，0x01表示渣土装载区，0x02表示渣土卸载区，0x03表示其他区域
		{
			return (false);
		}
		
		if (0x01 == g_cyPbState) //篷布状态 0x00表示篷布打开，0x01表示篷布关闭；
		{
			return (false);
		}
	}
	
	if (2 == g_cyCtrlFlag) //控制标准  1：只有速度， 2：有速度，有篷布状态 3：有速度，有篷布状态，有电子围栏
	{
		if (0x01 == g_cyPbState) //篷布状态 0x00表示篷布打开，0x01表示篷布关闭；
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
		
		//小于(空载值 + (满载值 – 空载值) * 30%)
		//大于(空载值 + (满载值 – 空载值) * 20%)
		fTemp = g_wFullValue - g_wEmptyValue;
		wUpValue = (u16)(g_wEmptyValue + fTemp * 0.3);
		wDownValue = (u16)(g_wEmptyValue - fTemp * 0.2);
			
		if ( (wDownValue < wTemp) && (wTemp < wUpValue) )
		{
			g_wUnloadCnt++;
			if (1 < g_wUnloadCnt)
			{
				if ( ( (g_wOldUnLoadValue - 3) < wTemp) && (wTemp < (g_wOldUnLoadValue + 3) ) )  //斜波停车
				{
					g_wUnloadCnt = 0;
				}
			}
			
			g_wOldUnLoadValue = wTemp;
			
			if (4 == g_wUnloadCnt)  //连续4秒钟都小于50% 判断卸载成功
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
// 名称         : LOAD_HandleSuccessUnload()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 卸载成功处理
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

static void LOAD_HandleSuccessUnload(void)
{
	u16 wTemp;
	u8  i;
	float fTemp;
		
	g_cyExitFlag = true;
	
	if (2 < g_cyCurrentStateCnt) //有学习成功数据
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
		
		if (LOAD_FULL_STUDY != (g_cyStudyState & LOAD_FULL_STUDY) ) //满载值没有学习成功 不受限制
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
			if (LOAD_STUDY == g_cyStudyState)  //都学习成功 不受限制
			{
				//新满载值如果大于(原满载值-原来空载值）*50% + 原来满载值，那么按(原满载值-原来空载值）*50% + 原来满载值作为新的满载值。
				//新满载值如果小于原来满载值 - (原满载值-原来空载值）* 50%，那么按原来满载值 - (原满载值-原来空载值）* 50%作为新的满载值
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
				
			//if (false == g_cyOverFlag) //不超载
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
// 名称         : LOAD_CheckRunUnload()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 检查运行中卸载
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
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
			
			//小于(空载值 + (满载值 – 空载值) * 30%)
			//大于(空载值 + (满载值 – 空载值) * 20%)
			wTemp = LOAD_GetAverage(g_wUnLoadBuffer, MAX_RUN_UNLOAD_NUM);
			fTemp = g_wFullValue - g_wEmptyValue;
			wUpValue = (u16)(g_wEmptyValue + fTemp * 0.3);
			wDownValue = (u16)(g_wEmptyValue - fTemp * 0.2);
			
			if ( (wDownValue < wTemp) && (wTemp < wUpValue) )
			{
				g_cyRunUnloadCnt++;
				if (5 == g_cyRunUnloadCnt)  //连续5秒钟都符合条件 判断卸载成功
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
// 名称         : LOAD_CheckFullValue()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 检查运行中满载值
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

#define MAX_CHECK_FULL  10
static u16 g_wRunFull[2][MAX_CHECK_FULL];          //检测数据

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
				if (wTemp < fTemp)   //卸载成功，并且不记录铲数
				{
					g_cyExitFlag = true;
					g_cyOverFlag = true;
				}*/
			}
			/*else    //卸载成功，并且不记录铲数
			{
				g_cyExitFlag = true;
				g_cyOverFlag = true;
			}*/
		}
	}
}

//**************************************************************************************************
// 名称         : LOAD_HandleFullTask()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 处理满载任务
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

static void LOAD_HandleFullTask(void)
{
	u16 wTemp;
	float fTemp;
	
	if (CAR_RUN_STATE == g_cyCarState) //开车
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
			
			//if ( (false == g_cyOverFlag) && (g_cyCurrentShovelNum > 5) ) //5铲以下不作记录
			if (g_cyCurrentShovelNum > 8)  //5铲以下不作记录
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
	
	if (true == LOAD_CheckUnload() )  //卸载成功
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
		
		if (10 > g_cyCurrentStateCnt)  //10秒平均值
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
// 名称         : LOAD_HandleInFullTask()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 处理装载到满载任务
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

static void LOAD_HandleInFullTask(void)
{
	u16 wTemp;
	float fTemp;
	
	if (CAR_RUN_STATE == g_cyCarState) //开车
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
		
		if (10 > g_cyCurrentStateCnt)  //10秒平均值
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
// 名称         : LOAD_LoadTask()
// 创建日期     : 2015-09-30
// 作者         : 陈锦
// 功能         : 处理载重任务
// 输入参数     : 无
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

void LOAD_LoadTask(void)
{
    if (MAX_LOAD_AD_NUM > g_wLoadADCnt)  //车机发送车的状态最晚需要1秒(100ms一次，采集600次需要1分钟)
    {
        g_wLoadADBuffer[g_wLoadADCnt] = RunVar.uiAI[0];
        g_wLoadADCnt++;
        return;
    }
    else
    {
        LOAD_FifoData(g_wLoadADBuffer, RunVar.uiAI[0], MAX_LOAD_AD_NUM);        //采集满600个以后采用FIFO方式填数组
    }
    
    if (LOAD_EMPTY_STATE == g_cyWorkState)                                      //空载状态
    {
        LOAD_HandleEmptyTask();                                                 //处理空载任务
    }
    else if (LOAD_INSTALL_STATE == g_cyWorkState)                              //装载状态
    {
        LOAD_HandleInstallTask();                                               //处理装载任务
    }
    else if (LOAD_IN_FULL_STATE == g_cyWorkState)                              //装载到满载状态
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
        LOAD_HandleInFullTask();                                                //处理装载到满载状态
    }
    else if (LOAD_FULL_STATE == g_cyWorkState)                                 //满载状态
    {
        LOAD_HandleFullTask();                                                  //处理满载任务
    }
    else
    {
        g_cyWorkState = LOAD_IDLE_STATE;                                        //空闲状态
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


