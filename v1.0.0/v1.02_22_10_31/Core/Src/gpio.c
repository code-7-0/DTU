/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
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
#include "gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();



  /*Configure GPIO pins : PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = EC600_RST_Pin|EC600_PWK_Pin|RUN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	//HAL_GPIO_WritePin(GPIOB, EC600_PWK_Pin,GPIO_PIN_SET|EC600_RST_Pin);//Ä£¿é¿ª»ú
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, EC600_RST_Pin|EC600_PWK_Pin|RUN_LED_Pin, GPIO_PIN_RESET);
	
	
	
	/*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin =  Single_Wire_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Single_Wire_GPIO_Port, &GPIO_InitStruct);
	
	/* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	
}


/* USER CODE BEGIN 2 */
void Single_Wire_output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0}; 
	/*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin =  Single_Wire_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Single_Wire_GPIO_Port, &GPIO_InitStruct);
	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
}

void Single_Wire_input(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0}; 
	/*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin =  Single_Wire_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Single_Wire_GPIO_Port, &GPIO_InitStruct);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
