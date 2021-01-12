#include "modbus.h"
#include "Picocap.h"
#include "spi_bsp.h"
#include "PCapDistance.h"
uint8_t  cnt;   //复位次数计数
uint8_t temp;

void SystemClockConfiguration(void);

uint8_t optimize = 0;          
uint32_t PCap_buf[3];
uint32_t  altit;
uint32_t SegmentValue2[3]={0};		//122-》0 -》(-0)	
//计算液位高度的一些参数初始化
void ArgumentInitialization(void)
{
    uint32_t SegmentValue1[19]={ 0};
    uint16_t altitude[9]={5,15,30,50,70,90,105,115,122};
    
    uint8_t AddrOffset;
    AddrOffset = 0;                //地址随液体类型偏移
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

    //122-》0 -》(-0)	
    AddrOffset = (UserParam.LiquidType - 1) * 12;                //地址随液体类型偏移
    Eeprom_Read_Multi4Bytes(CAPMAX1 + AddrOffset,&SegmentValue2[0],4);                      //满量程电容    
    Eeprom_Read_Multi4Bytes(CAPLOW1 + AddrOffset,&SegmentValue2[1],4);                      //满量程到零点电容      
    Eeprom_Read_Multi4Bytes(CAPMIN1 + AddrOffset,&SegmentValue2[2],4);                     //  -0 离开水面到零点电容
    PCap_EffectiveLength_init(SegmentValue1,SegmentValue2,altitude);
    
    Eeprom_Read_Multi4Bytes(ULRIPLEUP,&SegmentValue1[0],4); 
    Eeprom_Read_Multi4Bytes(ULRIPLEDE,&SegmentValue1[1],4);   
    
    SetSensitivity_UP(SegmentValue1[0]);                //方向切换灵敏度
    SetSensitivity_DE(SegmentValue1[1]);
    
    Eeprom_Read_Multi4Bytes(FILTERING,&SegmentValue1[2],4);  
    SetFilteringLevel(SegmentValue1[2]);                 //滤波级别   
    
    Eeprom_Read_Multi4Bytes(OPTIMIZ,&SegmentValue1[2],4);                     //  优化状态
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
    Read_Device_Param();                                                        //读取设备参数
    gUart1_Config();                                                            //串口1初始化                                                               
    HAL_Delay(10);
    PCap_Init();                                                                      //PCap芯片初始化
    gTim2Config();                                                               //定时器2配置
    ArgumentInitialization();                                                   //计算液位高度的一些参数初始化
  //  ADC_Chn_Config();                                                           //ADC多通道配置
  // DAC_Chn_Selection();                                                        //DAC通道选择配置
   User_Iwdg_Init();                                                         //初始化看门狗        6.4s
    
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
     
        User_Iwdg_Feed();                                                       //喂看门狗                                                         
      
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
          RunVarParam.ErrStatus &= 0xfff7;                                          //无采集故障，bit3置0
       }
       else
       {
           RunVarParam.ErrStatus = 0x0008;                                         //采集出现故障，bit3置1(采集出现故障则不存在超出范围)
           altit = 0;
       }
       
       if(!((PCap_buf[1] >= SegmentValue2[1]) && (PCap_buf[1] <= SegmentValue2[0])))
       {
            RunVarParam.ErrStatus |= 0x0080;                                        //超出测量范围，bit7置1   
    
            if(SegmentValue2[1] >= PCap_buf[1])                            //限制PCap值在量程范围
            {
                RunVarParam.ErrStatus &= 0xffbf;                                    //低于最小，把bit5位置0 
                RunVarParam.ErrStatus |= 0x0010;                                    //低于最小，把bit4位置1
            }
            else if(SegmentValue2[0] <= PCap_buf[1])
            {
                RunVarParam.ErrStatus &= 0xffef;                                    //高于最大，把bit4位置0 
                RunVarParam.ErrStatus |= 0x0020;                                    //高于最大，把bit5位置1
            }
        }
        else
        {
            RunVarParam.ErrStatus &= 0xff4f;                                        //在测量范围内，bit7置0    
        }

      //Modbus通信处理
        if(UsartRecvOk == USART_RECV_TIMER_OK)
        {
            MBASCII_Function();
            UsartRecvOk = 0;                                                    //发送完接收标志置0            
        }
        else
        {
            SensorEvent();                                                      //传感器事件自动上报
        }

     //   GetPCapResult();                                                        //获取PCap采集数据
     //   DataFilterAndOutput();                                                  //PCap数据滤波和输出
     //   ADC_GetChn_Value();                                                     //ADC通道数据获取
    //    DAC_Chn_Handle();                                                       //DAC通道数据处理
      
    }

}

//**************************************************************************************************
// 名称         	: SystemClockConfiguration()
// 创建日期     	: 2017-04-26
// 作者        	 	: 庄明群
// 功能         	: 系统时钟配置
// 输入参数     	: 无
// 输出参数     	: 无
// 返回结果     	: 无
// 注意和说明   	:
// 修改内容     	:
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