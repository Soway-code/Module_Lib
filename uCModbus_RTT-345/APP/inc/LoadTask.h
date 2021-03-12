/************************************************************************************************************************************************************************
** 版权：   2015-2025, 深圳市信为科技发展有限公司
** 文件名:  LoadTask.h
** 作者:    陈锦
** 版本:    V1.0.0
** 日期:    2015-07-28
** 描述:    载重任务处理
** 功能:         
*************************************************************************************************************************************************************************
** 修改者:          No
** 版本:  		
** 修改内容:    No 
** 日期:            No
*************************************************************************************************************************************************************************/

#include "bsp.h"
#include "type.h"

//#ifndef _LOAD_TASK_H_
//#define _LOAD_TASK_H_

#define LOAD_DATA_FIFO_NUM  10

extern u16 g_wRatedValue;                      //额定满量程
extern float g_fOverValueK;                    //超载值系数
extern u8  g_cyOverFlag;                	   //超载标志;
extern u8 g_cyShovelDTime;                     //铲与铲之间延迟时间    
extern u16 g_wCurrentOutputValue;              //当前输出值
extern u8  g_cyRecordShovelNum;                //记录铲数
extern u8  g_cyCarState;                       //车状态
extern u16 g_wEmptyBuffer[LOAD_DATA_FIFO_NUM]; //空载学习保存FIFO
extern u16 g_wFullBuffer[LOAD_DATA_FIFO_NUM];  //满载学习保存FIFO
extern u8  g_cyWorkState;                      //工作状态
extern float g_fVarianceValue;                 //方差值
extern u8  g_cyCurrentShovelNum;
extern u16 g_wEmptyValue;                      //空载值
extern u16 g_wFullValue;                       //满载值 

//工作状态
#define LOAD_EMPTY_STATE    0  //空载状态
#define LOAD_INSTALL_STATE  1  //装载状态
#define LOAD_IN_FULL_STATE  2  //装载到满载状态
#define LOAD_FULL_STATE     3  //满载状态
#define LOAD_IDLE_STATE     4  //休闲状态

//车状态
#define CAR_RUN_STATE       0  //开车
#define CAR_STOP_STATE      1  //停车

//学习标志
#define LOAD_NO_STUDY        0x00      //无学习过
#define LOAD_EMPTY_STUDY  0x01      //空载没有学习
#define LOAD_FULL_STUDY   0x02      //满载没有学习
#define LOAD_STUDY           0x03      //都学习成功

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

extern u16 LOAD_GetAverage(u16 *pWBuffer, u8 cyLen);

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

extern void LOAD_GetPar(void);

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

extern void LOAD_LoadTask(void);

//#endif //_LOAD_TASK_H_
