#include "weight.h"
#include "math.h"
   
struct Empirical_Data EmpiricalData;

struct OutData tempOutData;                                                    //输出数据
extern struct OutData myOutData;

#define  LEARNDATA_BUF_LEN  100                                                 //学习数据缓存长度
UInt16 LearnDataCounter = 0;
UInt16 LearnData1 = 0;                                                          //第一次学习数据
UInt16 LearnData2 = 0;                                                          //第二次学习数据
UInt16 LearnData_Buf[LEARNDATA_BUF_LEN];                                        //学习数据缓存

#define  FULLLOADDATA_BUF_LEN  180                                              //满载缓存长度
#define  EMPTYLOADDATA_BUF_LEN  180                                             //空载缓存长度
UInt16 FullLoadData_Counter = 0, EmptyLoadData_Counter = 0;                     //计数器
UInt16 FullLoadData_Buf[FULLLOADDATA_BUF_LEN];                                  //满载缓存
UInt16 EmptyLoadData_Buf[EMPTYLOADDATA_BUF_LEN];                                //空载缓存
UInt16 EmptyLoad=0, FullLoad=0;                                                 //临时保存空载和满载值
UInt16 filterData=0;                                                            //AD滤波值

UInt16 Speed;

void ResetFlashPara()                                                           //经验数据初始化
{
    EmpiricalData.Counter =0;                                                   //保存参数到EEPROM        
    I2C1_WriteNBytes(EEPROM_ADDRESS,WEIGHTPARAADDR,sizeof(EmpiricalData),(u8*)&EmpiricalData); 
    EmptyLoad=0;
    FullLoad=0;
    FullLoadData_Counter = 0;
    EmptyLoadData_Counter = 0;
    LearnDataCounter = 0;
    LearnData1 = 0;
    LearnData2 = 0;
}
       
void Init_xx()
{                                                                               //读取EEPROM中存储的数据
    I2C1_ReadNBytes(EEPROM_ADDRESS,WEIGHTPARAADDR,sizeof(EmpiricalData),(u8*)&EmpiricalData); 

    tempOutData.Weight =1;                                                      //浮动零点初始化为1

    if((EmpiricalData.Counter >6)||(EmpiricalData.Counter ==0))
    {
      // 经验数据初始化
      ResetFlashPara();
    }
}



		
void UpdateEmpiricalData(struct S_Weight NewData)                              //更新经验数据
{
    float Sum_EmptyLoad = 0.0f, Sum_FullLoad = 0.0f, Sum_Weight = 0.0f;
    UInt16 i;
    EmpiricalData.Counter++;                                                    //经验值加一
    if (EmpiricalData.Counter >= EMPIRICAL_BUF_LEN)  //#define EMPIRICAL_BUF_LEN  5    //历史/经验数据个数
    {
        EmpiricalData.Counter = EMPIRICAL_BUF_LEN;
    }
    
    for (i = 0; i < EMPIRICAL_BUF_LEN - 1; i++)                                 //先进先出，扔掉一个老的，新进一个新的(FIFO)
    {
        EmpiricalData.Data[i].EmptyLoad = EmpiricalData.Data[i + 1].EmptyLoad;
        EmpiricalData.Data[i].FullLoad = EmpiricalData.Data[i + 1].FullLoad;
        EmpiricalData.Data[i].Weight = EmpiricalData.Data[i + 1].Weight;
    }
    EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1].EmptyLoad = NewData.EmptyLoad;    //队尾放入最新数据
    EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1].FullLoad = NewData.FullLoad;
    EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1].Weight = NewData.Weight;

    for (i = 0; i < EmpiricalData.Counter; i++)                                 //求和
    {
        Sum_EmptyLoad += EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1 - i].EmptyLoad;
        Sum_FullLoad += EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1 - i].FullLoad;
        Sum_Weight += EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1 - i].Weight;
    }                                                                           //得到平均值(四舍五入)
    EmpiricalData.AverageEmptyLoad = (UInt16)(Sum_EmptyLoad / EmpiricalData.Counter + 0.5f);
    EmpiricalData.AverageFullLoad = (UInt16)(Sum_FullLoad / EmpiricalData.Counter + 0.5f);
    EmpiricalData.AverageWeight = (UInt16)(Sum_Weight / EmpiricalData.Counter + 0.5f);
}


 

UInt16 BubbleSort_Filter(UInt16 buf[], UInt16 len)                              //冒泡排序滤波（取中间值）
{
    UInt16 i, j;
    UInt16 temp;
    for (j = 0; j < len - 1; j++)                                               //采样值从小到大排列（冒泡法）
    {
        for (i = 0; i < len - 1 - j; i++)
        {
            if (buf[i] > buf[i + 1])
            {
                temp = buf[i];
                buf[i] = buf[i + 1];
                buf[i + 1] = temp;
            }
        }
    }
    return (buf[len / 2]);                                                      //取中间值
}




#define  FILTER_N  30                                                           //30个值，滤波深度
UInt16 filter_buf[FILTER_N + 1];
UInt16 Filter(UInt16 NewData)
{
    int i, j;
    UInt16 filter_temp;
    float filter_sum = 0;

    for (j = 0; j < FILTER_N - 1; j++)                                          //采样值从小到大排列（冒泡法）
    {
        for (i = 0; i < FILTER_N - 1 - j; i++)
        {
            if (filter_buf[i] > filter_buf[i + 1])
            {
                filter_temp = filter_buf[i];
                filter_buf[i] = filter_buf[i + 1];
                filter_buf[i + 1] = filter_temp;
            }
        }
    }
    filter_buf[0] = NewData;                                                    //FIFO
    for (i = FILTER_N; i > 0; i--)
    {
        filter_buf[i] = filter_buf[i - 1];
    }
    for (i = 2; i < FILTER_N - 2; i++)                                          //去掉最大和最小各2个求均值
    {
        filter_sum += filter_buf[i];
    }
    filter_sum = filter_sum / (FILTER_N - 4);
    return (UInt16) (filter_sum+0.5f);                                         //四舍五入

}



#define  SPEEDFILTER_N  10                                                      //10个值，速度滤波
UInt16 SpeedFilter_buf[SPEEDFILTER_N + 1];
UInt16 SpeedFilter(UInt16 NewData)                                              //速度滤波(递推)
{
    UInt16 i;
    float sum = 0;
    for (i = 0; i < SPEEDFILTER_N - 1; i++)                                     //FIFO
    {
        SpeedFilter_buf[i] = SpeedFilter_buf[i + 1];
    }
    SpeedFilter_buf[SPEEDFILTER_N - 1] = NewData;                               //最新的数据给最后一个元素

    for (i = 0; i < SPEEDFILTER_N; i++) 
    {
        sum += SpeedFilter_buf[i];
    }

    sum = sum / SPEEDFILTER_N;                                                  //求均值

    return (UInt16)(sum + 0.5f);                                               //四舍五入
}


 
struct OutData Runing(UInt16 NewData)
{
    struct S_Weight temp_Weight;
    Speed = SpeedFilter(Speed);                                                 //得到速度信号并滤波
    if(EmpiricalData.Counter > 0)                                               //有经验值之后才能处理数据
    {
        if (Speed > 20)                                                         //速度大于20km/h
        {
           filterData = Filter(NewData);                                        //递推滤波(采集到的AD值)
           tempOutData.FilterOutData = filterData;
           if (filterData > EmpiricalData.AverageEmptyLoad + EmpiricalData.AverageWeight * .4f)
           {                                                                    //空载值+载重值*0.4
               FullLoadData_Buf[FullLoadData_Counter++] = filterData;           //采集满载数据到缓冲
               if (FullLoadData_Counter == FULLLOADDATA_BUF_LEN)                //采集满
               {
                   FullLoadData_Counter = 0;                                    //取中位均值
                   FullLoad = BubbleSort_Filter(FullLoadData_Buf, FULLLOADDATA_BUF_LEN);
                   if(EmptyLoad>0)                                              //空载值必须有数据(空载和满载数据都有才可以写EEPROM)
                   {                                                      
                       temp_Weight.FullLoad = FullLoad;
                       temp_Weight.EmptyLoad = EmptyLoad;
                       temp_Weight.Weight =(UInt16)( FullLoad - EmptyLoad);
                       UpdateEmpiricalData(temp_Weight);                        //更新经验数据
                       I2C1_WriteNBytes(EEPROM_ADDRESS,WEIGHTPARAADDR,sizeof(EmpiricalData),(u8*)&EmpiricalData); //保存参数
                       EmptyLoad = 0;
                   }
               }
               tempOutData.Weight = (UInt16)( filterData - EmpiricalData.AverageEmptyLoad);
               if(tempOutData.Weight>600) 
               {
                   tempOutData.Weight = 600;                                    //重量值限制在 600以内
               }
           }
           else  //(采集到的AD值 <= 空载值+载重值*0.4
           {
               EmptyLoadData_Buf[EmptyLoadData_Counter++] = filterData;         //采集空载数据到缓冲
               if (EmptyLoadData_Counter == EMPTYLOADDATA_BUF_LEN)              //满
               {
                   EmptyLoadData_Counter = 0;
                   EmptyLoad = BubbleSort_Filter(EmptyLoadData_Buf, EMPTYLOADDATA_BUF_LEN);
               }                                                                               
               tempOutData.Weight =1;                                           //输出数据
           }
        }
    }
    
    if (EmpiricalData.Counter == 0)                                             //没有学习经验
    {
        if (Speed > 30)                                                         //速度大于30km/h
        {
            filterData = Filter(NewData);                                       //递推滤波            
            tempOutData.FilterOutData = filterData;
            if (LearnDataCounter < LEARNDATA_BUF_LEN)                           //30km/h 的车速必须保持 100s 以上数据才能学习成功
            {
                LearnData_Buf[LearnDataCounter++] = filterData;
            }
            else                                                                //当数据数量达到 LEN ,学习数据采集完成
            {   
                if (LearnData1==0)                                              //学习成功第一个数据
                {                                                                           
                    LearnData1 = BubbleSort_Filter(LearnData_Buf, LEARNDATA_BUF_LEN); 
                    LearnDataCounter = 0;                                       //中值滤波
                }
                else                                                            //学习成功第二个数据
                {   
                    LearnData2 = BubbleSort_Filter(LearnData_Buf, LEARNDATA_BUF_LEN); 
                    temp_Weight.Weight = (UInt16)abs(LearnData1 - LearnData2);  //分离出重量（第一次学习和第二次学习的差值）
                    LearnDataCounter = 0;
                    if (temp_Weight.Weight > 200)
                    {
                        if (LearnData1 > LearnData2)
                        {
                            temp_Weight.EmptyLoad = LearnData2;
                            temp_Weight.FullLoad = LearnData1;
                        }
                        else
                        {
                            temp_Weight.EmptyLoad = LearnData1;
                            temp_Weight.FullLoad = LearnData2;
                        }
                        UpdateEmpiricalData(temp_Weight);                       //更新经验数据
                        I2C1_WriteNBytes(EEPROM_ADDRESS,WEIGHTPARAADDR,sizeof(EmpiricalData),(u8*)&EmpiricalData); // 保存参数                       
                    }
                }
            }
        }
        else
        {                                                                       //速度低于30，重新开始采集数据                                                                                
            LearnDataCounter = 0;                                               //重新开始学习
        }
    }               
    return tempOutData;
}   

