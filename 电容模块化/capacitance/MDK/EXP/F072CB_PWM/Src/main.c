/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Picocap.h"
#include "Calculation.h"
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern TIM_HandleTypeDef htim3;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t PCap_buf[2];
uint32_t Period=9000;
uint8_t UART_BUF[200],i=0;
uint32_t  altit;

#define APPLICATION_ADDRESS     (uint32_t)0x08003000

#define VECTOR_TABLE_SIZE       48 * 4

#if   (defined ( __CC_ARM ))
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
__no_init __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )
__IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#endif // (defined ( __CC_ARM ))


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
    /* F0系列没有中断向量表重映射寄存器，所以，这里使用另一种方法：拷贝中断向量表到
    0x20000000开始的内存空间中，再将这个内存地址到0x00000000，完成中断向量表的重映射 */
    memcpy((void*)VectorTable, (void*)APPLICATION_ADDRESS, VECTOR_TABLE_SIZE); 
    /* 将系统内存映射到0x00000000 */
    __HAL_SYSCFG_REMAPMEMORY_SRAM();
	

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	PCap_Init();
	uint32_t SegmentValue[11]={1071364,1259417,1386342,1480923,1588943,1678879,1778879,1878879,1978879,2078879,2178879};	//11个段点对应的电容值
	uint16_t altitude[11]={0,167,335,435,535,635,735,835,935,1023,1112};	//11段点的高度
	uint16_t frequency[11]={5910,5457,5000,4690,4380,4110,3840,3630,3420,3152,2881};	//11个点频率
	parameter_init(SegmentValue,altitude,frequency);
	
	HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin,GPIO_PIN_SET);
	
	FrequencyCalibration(4800);		 //pwm频率校准

	//等待稳定
	{						
        uint8_t i=0;
        for(i = 0;i < 22; i++)
        {
			Sensor_PCap_GetResult(RESULT_REG1_ADDR,&PCap_buf[0],1);
            PCap_ChangeTheFrequency(PCap_buf[0],&Period,&PCap_buf[1],&altit);
            HAL_Delay(120);     
        }
    }
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2,Period/2);
	__HAL_TIM_SET_AUTORELOAD(&htim3,Period);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */	  
	Sensor_PCap_GetResult(RESULT_REG1_ADDR,&PCap_buf[0],1);				//读取电容值
	altitude[0] =   PCap_ChangeTheFrequency(PCap_buf[0],&Period,&PCap_buf[1],&altit);		//滤波计算
	HAL_Delay(200);
	altitude[1] = get_hz();

	//输出pwm
	if(i % 2 == 0)									
	{
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2,Period/2);			
		__HAL_TIM_SET_AUTORELOAD(&htim3,Period);
	}
	
	if(i > 5)
	{
		sprintf((char*)UART_BUF,"高度：%d     滤波后电容值：%d    hz=%d\n",altit,PCap_buf[1],altitude[1]); 
		HAL_UART_Transmit_DMA(&huart1,UART_BUF, 80); 
		i = 0;
	}
	i++;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
