/*
 * TIMERS.c
 *
 *  Created on: Mar 4, 2025
 *      Author: Caitlin Bonesio
 */
#include "BOARD.h"
#include "stm32wb0x_hal_tim.h"

TIM_HandleTypeDef htim2;

static uint32_t milliSeconds = 0;
static uint64_t microSeconds = 0;

uint32_t TIMERS_Init(){
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};

	  /* USER CODE BEGIN TIM2_Init 1 */

	  /* USER CODE END TIM2_Init 1 */
	  htim2.Instance = TIM2;
	  htim2.Init.Prescaler = 32;
	  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim2.Init.Period = 1000;
	  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

uint32_t TIMERS_GetMilliSeconds(){
	return milliSeconds;
}

uint64_t TIMERS_GetMircoSeconds(){
	uint16_t micro = __HAL_TIM_GET_COUNTER(htim2);
	return (milliSeconds*1000+micro&1000);
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
	milliSeconds++;
  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}


