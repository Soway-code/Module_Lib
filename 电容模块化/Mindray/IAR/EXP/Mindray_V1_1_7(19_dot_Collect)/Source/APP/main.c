#include "modbus.h"
#include "Picocap.h"
#include "spi_bsp.h"
#include "PCapDistance.h"
uint8_t  cnt;   //��λ��������
uint8_t temp;

void SystemClockConfiguration(void);

uint8_t optimize = 0;          
uint32_t PCap_buf[3];
uint32_t  altit;
uint32_t SegmentValue2[3]={0};		//122-��0 -��(-0)	
//����Һλ�߶ȵ�һЩ������ʼ��
void ArgumentInitialization(void)
{
    uint32_t SegmentValue1[19]={ 0};
    uint16_t altitude[9]={5,15,30,50,70,90,105,115,122};
    
    uint8_t AddrOffset;
    AddrOffset = 0;                //��ַ��Һ������ƫ��
    Eeprom_Read_Multi4Bytes(CAP1_0  + AddrOffset,&SegmentValue1[0],4);
    Eeprom_Read_Multi4Bytes(CAP1_1  + AddrOffset,&SegmentValue1[1],4);
    Eeprom_Read_Multi4Bytes(CAP1_2  + AddrOffset,&SegmentValue1[2],4);
    Eeprom_Read_Multi4Bytes(CAP1_3  + AddrOffset,&SegmentValue1[3],4);
    Eeprom_Read_Multi4Bytes(CAP1_4  + AddrOffset,&SegmentValue1[4],4);
    Eeprom_Read_Multi4Bytes(CAP1_5  + AddrOffset,&SegmentValue1[5],4);
    Eeprom_Read_Multi4Bytes(CAP1_6  + AddrOffset,&SegmentValue1[6],4);
    Eeprom_Read_Multi4Bytes(CAP1_7  + AddrOffset,&SegmentValue1[7],4);
    Eeprom_Read_Multi4Bytes(CAP1_8  + AddrOffset,&SegmentValue1[8],4);
    Eeprom_Read_Multi4Bytes(CAP1_9  + AddrOffset,&SegmentValue1[9],4);
    Eeprom_Read_Multi4Bytes(CAP1_10 + AddrOffset,&SegmentValue1[10],4);
    Eeprom_Read_Multi4Bytes(CAP1_11 + AddrOffset,&SegmentValue1[11],4);
    Eeprom_Read_Multi4Bytes(CAP1_12 + AddrOffset,&SegmentValue1[12],4);
    Eeprom_Read_Multi4Bytes(CAP1_13 + AddrOffset,&SegmentValue1[13],4);
    Eeprom_Read_Multi4Bytes(CAP1_14 + AddrOffset,&SegmentValue1[14],4);
    Eeprom_Read_Multi4Bytes(CAP1_15 + AddrOffset,&SegmentValue1[15],4);
    Eeprom_Read_Multi4Bytes(CAP1_16 + AddrOffset,&SegmentValue1[16],4);
    Eeprom_Read_Multi4Bytes(CAP1_17 + AddrOffset,&SegmentValue1[17],4);
    Eeprom_Read_Multi4Bytes(CAP1_18 + AddrOffset,&SegmentValue1[18],4);     

    //122-��0 -��(-0)	
    AddrOffset = (UserParam.LiquidType - 1) * 12;                //��ַ��Һ������ƫ��
    Eeprom_Read_Multi4Bytes(CAPMAX1 + AddrOffset,&SegmentValue2[0],4);                      //�����̵���    
    Eeprom_Read_Multi4Bytes(CAPLOW1 + AddrOffset,&SegmentValue2[1],4);                      //�����̵�������      
    Eeprom_Read_Multi4Bytes(CAPMIN1 + AddrOffset,&SegmentValue2[2],4);                     //  -0 �뿪ˮ�浽������
    PCap_EffectiveLength_init(SegmentValue1,SegmentValue2,altitude);
    
    Eeprom_Read_Multi4Bytes(ULRIPLEUP,&SegmentValue1[0],4); 
    Eeprom_Read_Multi4Bytes(ULRIPLEDE,&SegmentValue1[1],4);   
    
    SetSensitivity_UP(SegmentValue1[0]);                //�����л�������
    SetSensitivity_DE(SegmentValue1[1]);
    
    Eeprom_Read_Multi4Bytes(FILTERING,&SegmentValue1[2],4);  
    SetFilteringLevel(SegmentValue1[2]);                 //�˲�����   
    
    Eeprom_Read_Multi4Bytes(OPTIMIZ,&SegmentValue1[2],4);                     //  �Ż�״̬
    optimize = (uint8_t)SegmentValue1[2];
    CapacitanceValue(optimize,&PCap_buf[2]);
    
       
}
int main(void)
{
    //SysTick_Config(SystemCoreClock / 1000);

    HAL_Init(); 
    HAL_DeInit();
     
    HAL_RCC_DeInit();
    SystemClockConfiguration();
    
    cnt = Eeprom_Read_OneByte(RESET_CNT);
    Read_Device_Param();                                                        //��ȡ�豸����
    gUart1_Config();                                                            //����1��ʼ��                                                               
    HAL_Delay(10);
    PCap_Init();                                                                      //PCapоƬ��ʼ��
    gTim2Config();                                                               //��ʱ��2����
    ArgumentInitialization();                                                   //����Һλ�߶ȵ�һЩ������ʼ��
  //  ADC_Chn_Config();                                                           //ADC��ͨ������
  // DAC_Chn_Selection();                                                        //DACͨ��ѡ������
   User_Iwdg_Init();                                                         //��ʼ�����Ź�        6.4s
    
  // EepInitFlg = Eeprom_Read_OneByte(RUN_ADDR_BASE);
   
    {
        uint8_t i=0;
        for(i = 0;i < 14; i++)
        {
              Sensor_PCap_GetResult(RESULT_REG1_ADDR,&PCap_buf[0],1);
              PCap_EffectiveLength(PCap_buf[0],&altit,&PCap_buf[1]);
              HAL_Delay(120);     
          }
    }
    
    while(1)
    {
     
        User_Iwdg_Feed();                                                       //ι���Ź�                                                         
      
        __HAL_TIM_CLEAR_IT(&TIM2_HandleInitStruct, TIM_IT_UPDATE);
        HAL_TIM_Base_Start_IT(&TIM2_HandleInitStruct);
               
       Sensor_PCap_GetResult(RESULT_REG1_ADDR,&PCap_buf[0],1);
       if(optimize == 0x55)
       { 
              PCap_EffectiveLength(PCap_buf[0],&altit,&PCap_buf[1]);
        }
       else
       {     PCap_EffectiveLength(PCap_buf[0],&altit,&PCap_buf[2]);
             CapacitanceValue(optimize,&PCap_buf[1]);
             if((PCap_buf[1] == 0) || (altit == 0))
               PCap_buf[1] = PCap_buf[2];
       }
          HAL_Delay(150);
      if((0x00FFFFFF != PCap_buf[1]) && (0x0 != PCap_buf[1]))	
       {
          RunVarParam.ErrStatus &= 0xfff7;                                          //�޲ɼ����ϣ�bit3��0
       }
       else
       {
           RunVarParam.ErrStatus = 0x0008;                                         //�ɼ����ֹ��ϣ�bit3��1(�ɼ����ֹ����򲻴��ڳ�����Χ)
           altit = 0;
       }
       
       if(!((PCap_buf[1] >= SegmentValue2[1]) && (PCap_buf[1] <= SegmentValue2[0])))
       {
            RunVarParam.ErrStatus |= 0x0080;                                        //����������Χ��bit7��1   
    
            if(SegmentValue2[1] >= PCap_buf[1])                            //����PCapֵ�����̷�Χ
            {
                RunVarParam.ErrStatus &= 0xffbf;                                    //������С����bit5λ��0 
                RunVarParam.ErrStatus |= 0x0010;                                    //������С����bit4λ��1
            }
            else if(SegmentValue2[0] <= PCap_buf[1])
            {
                RunVarParam.ErrStatus &= 0xffef;                                    //������󣬰�bit4λ��0 
                RunVarParam.ErrStatus |= 0x0020;                                    //������󣬰�bit5λ��1
            }
        }
        else
        {
            RunVarParam.ErrStatus &= 0xff4f;                                        //�ڲ�����Χ�ڣ�bit7��0    
        }

      //Modbusͨ�Ŵ���
        if(UsartRecvOk == USART_RECV_TIMER_OK)
        {
            MBASCII_Function();
            UsartRecvOk = 0;                                                    //��������ձ�־��0            
        }
        else
        {
            SensorEvent();                                                      //�������¼��Զ��ϱ�
        }

     //   GetPCapResult();                                                        //��ȡPCap�ɼ�����
     //   DataFilterAndOutput();                                                  //PCap�����˲������
     //   ADC_GetChn_Value();                                                     //ADCͨ�����ݻ�ȡ
    //    DAC_Chn_Handle();                                                       //DACͨ�����ݴ���
      
    }

}

//**************************************************************************************************
// ����         	: SystemClockConfiguration()
// ��������     	: 2017-04-26
// ����        	 	: ׯ��Ⱥ
// ����         	: ϵͳʱ������
// �������     	: ��
// �������     	: ��
// ���ؽ��     	: ��
// ע���˵��   	:
// �޸�����     	:
//**************************************************************************************************

void SystemClockConfiguration(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
//    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
//    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
//    Error_Handler();
  }
}