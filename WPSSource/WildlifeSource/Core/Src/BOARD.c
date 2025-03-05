/*
 * BOARD.c
 *
 *  Created on: Feb 28, 2025
 *      Author: stipi
 */

#include "BOARD.h"


/**
  * @brief System Clock Configuration
  * @retval None
  */
int SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		return INIT_ERROR;
	}

	/** Configure the SYSCLKSource and SYSCLKDivider
	*/
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_RC64MPLL;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_RC64MPLL_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_WAIT_STATES_1) != HAL_OK)
	{
		return INIT_ERROR;
	}
	return INIT_OK;
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
int PeriphCommonClock_Config(void)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	/** Initializes the peripherals clock
	*/
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
	PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLK_DIV4;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		return INIT_ERROR;
	}
	return INIT_OK;
}


//Initializes the board
int BOARD_Init(void){
	int errorFlag = INIT_OK;
	if(PeriphCommonClock_Config() == INIT_ERROR){
		errorFlag = INIT_ERROR;
	}
	if(SystemClock_Config() == INIT_ERROR){
		errorFlag = INIT_ERROR;
	}
	return errorFlag;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void BOARD_CrashHandler(void)
{
  __disable_irq();
  while (1){

  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1);
}



