/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RS485_EN2_Pin GPIO_PIN_1
#define RS485_EN2_GPIO_Port GPIOA
#define RS485_TX2_Pin GPIO_PIN_2
#define RS485_TX2_GPIO_Port GPIOA
#define RS485_RX2_Pin GPIO_PIN_3
#define RS485_RX2_GPIO_Port GPIOA
#define Single_Wire_Pin GPIO_PIN_13
#define Single_Wire_GPIO_Port GPIOB
#define Single_Wire_EXTI_IRQn EXTI4_15_IRQn
#define EC600_RST_Pin GPIO_PIN_14
#define EC600_RST_GPIO_Port GPIOB
#define EC600_PWK_Pin GPIO_PIN_15
#define EC600_PWK_GPIO_Port GPIOB
#define EC600_TX_Pin GPIO_PIN_9
#define EC600_TX_GPIO_Port GPIOA
#define EC600_RX_Pin GPIO_PIN_10
#define EC600_RX_GPIO_Port GPIOA
#define RUN_LED_Pin GPIO_PIN_3
#define RUN_LED_GPIO_Port GPIOB
#define DEBUG_TX3_Pin GPIO_PIN_8
#define DEBUG_TX3_GPIO_Port GPIOB
#define DEBUG_RX3_Pin GPIO_PIN_9
#define DEBUG_RX3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
