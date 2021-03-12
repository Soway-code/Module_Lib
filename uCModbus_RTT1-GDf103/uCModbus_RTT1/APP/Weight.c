#include "weight.h"
#include "math.h"
   
struct Empirical_Data EmpiricalData;

struct OutData tempOutData;                                                    //�������
extern struct OutData myOutData;

#define  LEARNDATA_BUF_LEN  100                                                 //ѧϰ���ݻ��泤��
UInt16 LearnDataCounter = 0;
UInt16 LearnData1 = 0;                                                          //��һ��ѧϰ����
UInt16 LearnData2 = 0;                                                          //�ڶ���ѧϰ����
UInt16 LearnData_Buf[LEARNDATA_BUF_LEN];                                        //ѧϰ���ݻ���

#define  FULLLOADDATA_BUF_LEN  180                                              //���ػ��泤��
#define  EMPTYLOADDATA_BUF_LEN  180                                             //���ػ��泤��
UInt16 FullLoadData_Counter = 0, EmptyLoadData_Counter = 0;                     //������
UInt16 FullLoadData_Buf[FULLLOADDATA_BUF_LEN];                                  //���ػ���
UInt16 EmptyLoadData_Buf[EMPTYLOADDATA_BUF_LEN];                                //���ػ���
UInt16 EmptyLoad=0, FullLoad=0;                                                 //��ʱ������غ�����ֵ
UInt16 filterData=0;                                                            //AD�˲�ֵ

UInt16 Speed;

void ResetFlashPara()                                                           //�������ݳ�ʼ��
{
    EmpiricalData.Counter =0;                                                   //���������EEPROM        
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
{                                                                               //��ȡEEPROM�д洢������
    I2C1_ReadNBytes(EEPROM_ADDRESS,WEIGHTPARAADDR,sizeof(EmpiricalData),(u8*)&EmpiricalData); 

    tempOutData.Weight =1;                                                      //��������ʼ��Ϊ1

    if((EmpiricalData.Counter >6)||(EmpiricalData.Counter ==0))
    {
      // �������ݳ�ʼ��
      ResetFlashPara();
    }
}



		
void UpdateEmpiricalData(struct S_Weight NewData)                              //���¾�������
{
    float Sum_EmptyLoad = 0.0f, Sum_FullLoad = 0.0f, Sum_Weight = 0.0f;
    UInt16 i;
    EmpiricalData.Counter++;                                                    //����ֵ��һ
    if (EmpiricalData.Counter >= EMPIRICAL_BUF_LEN)  //#define EMPIRICAL_BUF_LEN  5    //��ʷ/�������ݸ���
    {
        EmpiricalData.Counter = EMPIRICAL_BUF_LEN;
    }
    
    for (i = 0; i < EMPIRICAL_BUF_LEN - 1; i++)                                 //�Ƚ��ȳ����ӵ�һ���ϵģ��½�һ���µ�(FIFO)
    {
        EmpiricalData.Data[i].EmptyLoad = EmpiricalData.Data[i + 1].EmptyLoad;
        EmpiricalData.Data[i].FullLoad = EmpiricalData.Data[i + 1].FullLoad;
        EmpiricalData.Data[i].Weight = EmpiricalData.Data[i + 1].Weight;
    }
    EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1].EmptyLoad = NewData.EmptyLoad;    //��β������������
    EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1].FullLoad = NewData.FullLoad;
    EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1].Weight = NewData.Weight;

    for (i = 0; i < EmpiricalData.Counter; i++)                                 //���
    {
        Sum_EmptyLoad += EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1 - i].EmptyLoad;
        Sum_FullLoad += EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1 - i].FullLoad;
        Sum_Weight += EmpiricalData.Data[EMPIRICAL_BUF_LEN - 1 - i].Weight;
    }                                                                           //�õ�ƽ��ֵ(��������)
    EmpiricalData.AverageEmptyLoad = (UInt16)(Sum_EmptyLoad / EmpiricalData.Counter + 0.5f);
    EmpiricalData.AverageFullLoad = (UInt16)(Sum_FullLoad / EmpiricalData.Counter + 0.5f);
    EmpiricalData.AverageWeight = (UInt16)(Sum_Weight / EmpiricalData.Counter + 0.5f);
}


 

UInt16 BubbleSort_Filter(UInt16 buf[], UInt16 len)                              //ð�������˲���ȡ�м�ֵ��
{
    UInt16 i, j;
    UInt16 temp;
    for (j = 0; j < len - 1; j++)                                               //����ֵ��С�������У�ð�ݷ���
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
    return (buf[len / 2]);                                                      //ȡ�м�ֵ
}




#define  FILTER_N  30                                                           //30��ֵ���˲����
UInt16 filter_buf[FILTER_N + 1];
UInt16 Filter(UInt16 NewData)
{
    int i, j;
    UInt16 filter_temp;
    float filter_sum = 0;

    for (j = 0; j < FILTER_N - 1; j++)                                          //����ֵ��С�������У�ð�ݷ���
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
    for (i = 2; i < FILTER_N - 2; i++)                                          //ȥ��������С��2�����ֵ
    {
        filter_sum += filter_buf[i];
    }
    filter_sum = filter_sum / (FILTER_N - 4);
    return (UInt16) (filter_sum+0.5f);                                         //��������

}



#define  SPEEDFILTER_N  10                                                      //10��ֵ���ٶ��˲�
UInt16 SpeedFilter_buf[SPEEDFILTER_N + 1];
UInt16 SpeedFilter(UInt16 NewData)                                              //�ٶ��˲�(����)
{
    UInt16 i;
    float sum = 0;
    for (i = 0; i < SPEEDFILTER_N - 1; i++)                                     //FIFO
    {
        SpeedFilter_buf[i] = SpeedFilter_buf[i + 1];
    }
    SpeedFilter_buf[SPEEDFILTER_N - 1] = NewData;                               //���µ����ݸ����һ��Ԫ��

    for (i = 0; i < SPEEDFILTER_N; i++) 
    {
        sum += SpeedFilter_buf[i];
    }

    sum = sum / SPEEDFILTER_N;                                                  //���ֵ

    return (UInt16)(sum + 0.5f);                                               //��������
}


 
struct OutData Runing(UInt16 NewData)
{
    struct S_Weight temp_Weight;
    Speed = SpeedFilter(Speed);                                                 //�õ��ٶ��źŲ��˲�
    if(EmpiricalData.Counter > 0)                                               //�о���ֵ֮����ܴ�������
    {
        if (Speed > 20)                                                         //�ٶȴ���20km/h
        {
           filterData = Filter(NewData);                                        //�����˲�(�ɼ�����ADֵ)
           tempOutData.FilterOutData = filterData;
           if (filterData > EmpiricalData.AverageEmptyLoad + EmpiricalData.AverageWeight * .4f)
           {                                                                    //����ֵ+����ֵ*0.4
               FullLoadData_Buf[FullLoadData_Counter++] = filterData;           //�ɼ��������ݵ�����
               if (FullLoadData_Counter == FULLLOADDATA_BUF_LEN)                //�ɼ���
               {
                   FullLoadData_Counter = 0;                                    //ȡ��λ��ֵ
                   FullLoad = BubbleSort_Filter(FullLoadData_Buf, FULLLOADDATA_BUF_LEN);
                   if(EmptyLoad>0)                                              //����ֵ����������(���غ��������ݶ��вſ���дEEPROM)
                   {                                                      
                       temp_Weight.FullLoad = FullLoad;
                       temp_Weight.EmptyLoad = EmptyLoad;
                       temp_Weight.Weight =(UInt16)( FullLoad - EmptyLoad);
                       UpdateEmpiricalData(temp_Weight);                        //���¾�������
                       I2C1_WriteNBytes(EEPROM_ADDRESS,WEIGHTPARAADDR,sizeof(EmpiricalData),(u8*)&EmpiricalData); //�������
                       EmptyLoad = 0;
                   }
               }
               tempOutData.Weight = (UInt16)( filterData - EmpiricalData.AverageEmptyLoad);
               if(tempOutData.Weight>600) 
               {
                   tempOutData.Weight = 600;                                    //����ֵ������ 600����
               }
           }
           else  //(�ɼ�����ADֵ <= ����ֵ+����ֵ*0.4
           {
               EmptyLoadData_Buf[EmptyLoadData_Counter++] = filterData;         //�ɼ��������ݵ�����
               if (EmptyLoadData_Counter == EMPTYLOADDATA_BUF_LEN)              //��
               {
                   EmptyLoadData_Counter = 0;
                   EmptyLoad = BubbleSort_Filter(EmptyLoadData_Buf, EMPTYLOADDATA_BUF_LEN);
               }                                                                               
               tempOutData.Weight =1;                                           //�������
           }
        }
    }
    
    if (EmpiricalData.Counter == 0)                                             //û��ѧϰ����
    {
        if (Speed > 30)                                                         //�ٶȴ���30km/h
        {
            filterData = Filter(NewData);                                       //�����˲�            
            tempOutData.FilterOutData = filterData;
            if (LearnDataCounter < LEARNDATA_BUF_LEN)                           //30km/h �ĳ��ٱ��뱣�� 100s �������ݲ���ѧϰ�ɹ�
            {
                LearnData_Buf[LearnDataCounter++] = filterData;
            }
            else                                                                //�����������ﵽ LEN ,ѧϰ���ݲɼ����
            {   
                if (LearnData1==0)                                              //ѧϰ�ɹ���һ������
                {                                                                           
                    LearnData1 = BubbleSort_Filter(LearnData_Buf, LEARNDATA_BUF_LEN); 
                    LearnDataCounter = 0;                                       //��ֵ�˲�
                }
                else                                                            //ѧϰ�ɹ��ڶ�������
                {   
                    LearnData2 = BubbleSort_Filter(LearnData_Buf, LEARNDATA_BUF_LEN); 
                    temp_Weight.Weight = (UInt16)abs(LearnData1 - LearnData2);  //�������������һ��ѧϰ�͵ڶ���ѧϰ�Ĳ�ֵ��
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
                        UpdateEmpiricalData(temp_Weight);                       //���¾�������
                        I2C1_WriteNBytes(EEPROM_ADDRESS,WEIGHTPARAADDR,sizeof(EmpiricalData),(u8*)&EmpiricalData); // �������                       
                    }
                }
            }
        }
        else
        {                                                                       //�ٶȵ���30�����¿�ʼ�ɼ�����                                                                                
            LearnDataCounter = 0;                                               //���¿�ʼѧϰ
        }
    }               
    return tempOutData;
}   

