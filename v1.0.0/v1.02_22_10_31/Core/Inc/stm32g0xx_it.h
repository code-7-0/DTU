/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g0xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
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
#ifndef __STM32G0xx_IT_H
#define __STM32G0xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

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
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void TIM1_BRK_UP_TRG_COM_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_4_IRQHandler(void);
/* USER CODE BEGIN EFP */

extern unsigned int delaycount;

//����2���
extern unsigned char  En_RS485_Shandle;//����֡�����־
extern unsigned int 	usart2delay;//ͨ�ų�ʱʱ�䣬�������5ms,Ϊͨ�ų�ʱ
extern unsigned char  FLAG_uart2;	//UART2��ʼ�µ�һ֡���ݽ���ǰ��ʹ�ܽ��������־�����ܹ����б���Ϊ1���ڶ�ʱ���������ճ�ʱʱ������


extern unsigned char topic_update_flag;  //ÿ�����һ�α�־	

extern unsigned int live_flag_cunt;//mqtt��Ծ��ʱ

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32G0xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
