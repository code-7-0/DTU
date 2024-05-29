/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"


#include "timer.h"
#include "debug.h"
#include "lte_demo.h"
#include "temper_control.h"
void SystemClock_Config(void);
int func_b(void)
{
    return 0;
}
int main(void)
{
  /* USER CODE BEGIN 1 */
		//使用IAP更新功能，	VTOR寄存器存放的是新的中断向量表的起始地址复位从0x8005000开始  
		//前20K为IAP程序
	SCB->VTOR = FLASH_BASE | 0x5000; 
	FLASH->SR = FLASH_SR_CLEAR;	
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	#ifndef DEBUG
	MX_IWDG_Init();
	#endif
	MX_TIM1_Init();//10ms定时器
	MX_USART1_UART_Init();//EC600N
	MX_USART2_UART_Init();//MODBUS
	MX_USART3_UART_Init();//DEBUG
	MX_TIM3_Init();//计数器
	
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim1);//启动定时器TIM1
	HAL_TIM_Base_Start(&htim3);//启动定时器TIM3
  USART_DMAstart();//串口初始化 
	Init_Flash(); 

///**********测试
		//OTA存储地址
	OTA_store_addr=*(uint32_t*)UPGRADEaddr;
	if(OTA_store_addr==APP3adress)
		OTA_store_addr=APP2adress;
	else  
		OTA_store_addr=APP3adress;

	debug_printf("固件版本：%d \r\n",VER_MAJOR);	
//***********/
	initLinkedList(&Linked_List);//初始化链表
  while (1)
  {
    /* USER CODE END WHILE */
		//1个循环周期小于1MS
		Handle_Debug();//调试串口
		Handle_Flash();//改写flash
//		if(update_flag==0)//ota过程中禁止
		{
			Handle_Gaspower();//壁挂炉
			Handle_GasCost(); //积分
			Handle_topic();//主题
		}
//		Handle_Gaspower();
			Handle_Lte();//ec600连接 mqtt连接 发送 
		
			Handle_Temper_Crontrol();//温控器
			updateTime();//时钟
		#ifndef DEBUG
		//喂狗
		if(iwdg_flag==1){
			iwdg_flag=0;
			HAL_IWDG_Refresh(&hiwdg);	
		}
		#endif 

  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
