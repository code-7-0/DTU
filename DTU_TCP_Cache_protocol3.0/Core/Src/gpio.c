/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, EC600_RST_Pin|EC600_PWK_Pin|RUN_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = Single_Wire_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Single_Wire_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = EC600_RST_Pin|EC600_PWK_Pin|RUN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 2 */

// 实现函数
void SetPinMode(PinMode mode) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
	    // 首先反初始化引脚
    HAL_GPIO_DeInit(Single_Wire_GPIO_Port, Single_Wire_Pin);	
		
    // 共通配置
    GPIO_InitStruct.Pin = Single_Wire_Pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//		GPIO_InitStruct.Pull = GPIO_NOPULL; 
		GPIO_InitStruct.Pull = GPIO_PULLUP;  
    // 根据模式配置
    if (mode == INPUT_MODE) {
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; // 配置为中断模式，上升沿和下降沿触发  
				HAL_GPIO_Init(Single_Wire_GPIO_Port, &GPIO_InitStruct);        // 重新初始化引脚
        // 启用中断
        HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);			
    } else if (mode == OUTPUT_MODE) {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // 开漏输出 
        HAL_GPIO_Init(Single_Wire_GPIO_Port, &GPIO_InitStruct);
        // 禁用中断
        HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
    }
	
}
/* USER CODE END 2 */
