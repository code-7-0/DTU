/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#define USART2_TX_PA2_Pin GPIO_PIN_2
#define USART2_TX_PA2_GPIO_Port GPIOA
#define USART2_RX_PA3_Pin GPIO_PIN_3
#define USART2_RX_PA3_GPIO_Port GPIOA

#define Single_Wire_Pin GPIO_PIN_13
#define Single_Wire_GPIO_Port GPIOB
#define Single_Wire_EXTI_IRQn EXTI4_15_IRQn

#define EC600_RST_Pin GPIO_PIN_14
#define EC600_RST_GPIO_Port GPIOB
#define EC600_PWK_Pin GPIO_PIN_15
#define EC600_PWK_GPIO_Port GPIOB
#define USART1_TX_PA9_Pin GPIO_PIN_9
#define USART1_TX_PA9_GPIO_Port GPIOA
#define USART1_RX_PA10_Pin GPIO_PIN_10
#define USART1_RX_PA10_GPIO_Port GPIOA
#define RUN_LED_Pin GPIO_PIN_3
#define RUN_LED_GPIO_Port GPIOB
#define USART3_TX_PB8_Pin GPIO_PIN_8
#define USART3_TX_PB8_GPIO_Port GPIOB
#define USART3_RX_PB9_Pin GPIO_PIN_9
#define USART3_RX_PB9_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
