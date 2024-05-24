/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USART1 init function */
void MX_USART1_UART_Init(void)
{
		huart1.Instance = USART1;
		huart1.Init.BaudRate = 115200;
		huart1.Init.WordLength = UART_WORDLENGTH_8B;
		huart1.Init.StopBits = UART_STOPBITS_1;
		huart1.Init.Parity = UART_PARITY_NONE;
		huart1.Init.Mode = UART_MODE_TX_RX;
		huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart1.Init.OverSampling = UART_OVERSAMPLING_16;
		huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
		huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
		huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
		if (HAL_UART_Init(&huart1) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
		{
			Error_Handler();
		}
}


/* USART2 init function */
void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart2, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USART3 init function */
void MX_USART3_UART_Init(void)
{
		huart3.Instance = USART3;
		huart3.Init.BaudRate = 115200;
		huart3.Init.WordLength = UART_WORDLENGTH_8B;
		huart3.Init.StopBits = UART_STOPBITS_1;
		huart3.Init.Parity = UART_PARITY_NONE;
		huart3.Init.Mode = UART_MODE_TX_RX;
		huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart3.Init.OverSampling = UART_OVERSAMPLING_16;
		huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
		huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
		huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
		if (HAL_UART_Init(&huart3) != HAL_OK)
		{
			Error_Handler();
		}
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

		GPIO_InitTypeDef GPIO_InitStruct = {0};
		if(uartHandle->Instance==USART1)
		{
		/* USER CODE BEGIN USART1_MspInit 0 */

		/* USER CODE END USART1_MspInit 0 */
			/* USART1 clock enable */
			__HAL_RCC_USART1_CLK_ENABLE();

			__HAL_RCC_GPIOA_CLK_ENABLE();
			/**USART1 GPIO Configuration
			PA9     ------> USART1_TX
			PA10     ------> USART1_RX
			*/
			GPIO_InitStruct.Pin = USART1_TX_PA9_Pin|USART1_RX_PA10_Pin;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			//GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			/* USART1 interrupt Init */
			HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
			HAL_NVIC_EnableIRQ(USART1_IRQn);
		/* USER CODE BEGIN USART1_MspInit 1 */

		/* USER CODE END USART1_MspInit 1 */
		}
		else if(uartHandle->Instance==USART2)
		{
		/* USER CODE BEGIN USART2_MspInit 0 */

		/* USER CODE END USART2_MspInit 0 */
			/* USART2 clock enable */
			__HAL_RCC_USART2_CLK_ENABLE();

			__HAL_RCC_GPIOA_CLK_ENABLE();
			/**USART2 GPIO Configuration
			PA1     ------> USART2_DE
			PA2     ------> USART2_TX
			PA3     ------> USART2_RX
			*/
			GPIO_InitStruct.Pin = RS485_EN2_Pin|USART2_TX_PA2_Pin|USART2_RX_PA3_Pin;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			/* USART2 interrupt Init */
			HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
			HAL_NVIC_EnableIRQ(USART2_IRQn);
		/* USER CODE BEGIN USART2_MspInit 1 */

		/* USER CODE END USART2_MspInit 1 */
		}
		else if(uartHandle->Instance==USART3)
		{
		/* USER CODE BEGIN USART3_MspInit 0 */

		/* USER CODE END USART3_MspInit 0 */
			/* USART3 clock enable */
			__HAL_RCC_USART3_CLK_ENABLE();

			__HAL_RCC_GPIOB_CLK_ENABLE();
			/**USART3 GPIO Configuration
			PB8     ------> USART3_TX
			PB9     ------> USART3_RX
			*/
			GPIO_InitStruct.Pin = USART3_TX_PB8_Pin|USART3_RX_PB9_Pin;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF4_USART3;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

			/* USART3 interrupt Init */
			HAL_NVIC_SetPriority(USART3_4_IRQn, 3, 0);
			HAL_NVIC_EnableIRQ(USART3_4_IRQn);
		/* USER CODE BEGIN USART3_MspInit 1 */

		/* USER CODE END USART3_MspInit 1 */
		}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, USART1_TX_PA9_Pin|USART1_RX_PA10_Pin);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA1     ------> USART2_DE
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, RS485_EN2_Pin|USART2_TX_PA2_Pin|USART2_RX_PA3_Pin);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB8     ------> USART3_TX
    PB9     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, USART3_TX_PB8_Pin|USART3_RX_PB9_Pin);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_4_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void Delay_sys(__IO unsigned long nCount)
{
  	for(; nCount != 0; nCount--);
}


/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/