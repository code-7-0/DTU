/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g0xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ec600n.h"
#include "modbus.h"
#include "debug.h"
#include "Gaspower.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */
unsigned int delaycount=0;
unsigned int delaycount_enable=0;
unsigned int delay100mscount=0;
/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim1;



/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line 4 to 15 interrupts.
  */
void EXTI4_15_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_15_IRQn 0 */
	if(HAL_GPIO_ReadPin(Single_Wire_GPIO_Port,Single_Wire_Pin)==GPIO_PIN_RESET) //判断为高电平
    {
       EXTI4_15_Single_Wire_Pin_flag = 1;
    }
		
  /* USER CODE END EXTI4_15_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
  /* USER CODE BEGIN EXTI4_15_IRQn 1 */

  /* USER CODE END EXTI4_15_IRQn 1 */
}
/**
  * @brief This function handles TIM1 break, update, trigger and commutation interrupts.
  */

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_IRQn 0 */
  unsigned int dat;
  /* USER CODE END TIM1_IRQn 0 */
	HAL_TIM_IRQHandler(&htim1);
  
	/* USER CODE BEGIN TIM1_IRQn 1 */
	//1ms延时
	if(delay100mscount > 0)	 delay100mscount--;
		else  
		{
			delay100mscount = 9; 
			delaycount_enable=1;	
		}

	Gaspsend_delaycount++;	 //发送计时
		
	if(HAL_GPIO_ReadPin(Single_Wire_GPIO_Port,Single_Wire_Pin)==GPIO_PIN_RESET)
		Gasprev_delaycount++;  //接收计时
	
	//HAL_GPIO_TogglePin(GPIOB, RUN_LED_Pin);//测试基本时钟

	if(delaycount_enable)
	{
			delaycount_enable=0;
				if(delaycount > 0)	 delaycount--;
				else  //秒级延时
				{
					HAL_GPIO_TogglePin(GPIOB, RUN_LED_Pin);
					delaycount = 499; 
					topic_update_flag = 1; 
					
					
					if(IntegrationDelay>0)  IntegrationDelay--;
					else IntegrationFlag = 1;
					
					if(live_flag_cunt>0) live_flag_cunt--;
				}
				
		fat_tim_proc();
				
		//判断Uart1通信超时
		if(FLAG_uart1)
		{
			if(usart1delay >= delaycount)  //计数未溢出
			{
				dat = usart1delay - delaycount;
			}else
			{
				dat = usart1delay+1000;
				dat =  dat -  delaycount;
			}
			if(dat > 3)  //通信超时
			{
				 FLAG_uart1 = 0;   //帧接收标志清零
				 Ec600_Shandle = 1;	  //帧处理标志置位
			}
		}		
		
		if(send_waittime_cunt)  send_waittime_cunt--;
	
		//判断Uart2通信超时
		if(FLAG_uart2)
		{
			if(usart2delay >= delaycount)  //计数未溢出
			{
				dat = usart2delay - delaycount;
			}else
			{
				dat = usart2delay+1000;
				dat =  dat -  delaycount;
			}
			if(dat > 3)  //通信超时
			{
				 FLAG_uart2 = 0;   //帧接收标志清零
				 En_RS485_Shandle = 1;	  //帧处理标志置位
			}
		}
		
		//判断Uart3通信超时
		if(FLAG_uart3)
		{
			if(usart3delay >= delaycount)  //计数未溢出
			{
				dat = usart3delay - delaycount;
			}else
			{
				dat = usart3delay+1000;
				dat =  dat -  delaycount;
			}
			if(dat > 3)  //通信超时
			{
				 FLAG_uart3 = 0;   //帧接收标志清零
				 Debug_Shandle = 1;	  //帧处理标志置位
			}
		}
	}
	/* USER CODE END TIM1_IRQn 1 */
}


/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
