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
		//ʹ��IAP���¹��ܣ�	VTOR�Ĵ�����ŵ����µ��ж����������ʼ��ַ��λ��0x8005000��ʼ  
		//ǰ20KΪIAP����
	SCB->VTOR = FLASH_BASE | 0x5000; 
	FLASH->SR = FLASH_SR_CLEAR;	
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	#ifndef DEBUG
	MX_IWDG_Init();
	#endif
	MX_TIM1_Init();//10ms��ʱ��
	MX_USART1_UART_Init();//EC600N
	MX_USART2_UART_Init();//MODBUS
	MX_USART3_UART_Init();//DEBUG
	MX_TIM3_Init();//������
	
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim1);//������ʱ��TIM1
	HAL_TIM_Base_Start(&htim3);//������ʱ��TIM3
  USART_DMAstart();//���ڳ�ʼ�� 
	Init_Flash(); 

///**********����
		//OTA�洢��ַ
	OTA_store_addr=*(uint32_t*)UPGRADEaddr;
	if(OTA_store_addr==APP3adress)
		OTA_store_addr=APP2adress;
	else  
		OTA_store_addr=APP3adress;

	debug_printf("�̼��汾��%d \r\n",VER_MAJOR);	
//***********/
	initLinkedList(&Linked_List);//��ʼ������
  while (1)
  {
    /* USER CODE END WHILE */
		//1��ѭ������С��1MS
		Handle_Debug();//���Դ���
		Handle_Flash();//��дflash
//		if(update_flag==0)//ota�����н�ֹ
		{
			Handle_Gaspower();//�ڹ�¯
			Handle_GasCost(); //����
			Handle_topic();//����
		}
//		Handle_Gaspower();
			Handle_Lte();//ec600���� mqtt���� ���� 
		
			Handle_Temper_Crontrol();//�¿���
			updateTime();//ʱ��
		#ifndef DEBUG
		//ι��
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
