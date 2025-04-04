/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


// USER CODE END Header
// Includes ------------------------------------------------------------------
//#include "BOARD.h"
//
#include "BOARD.h"
#include "CONFIG.h"
#include "UART.h"
#include "GPIO.h"

// Private includes ----------------------------------------------------------

// Private typedef -----------------------------------------------------------

// Private define ------------------------------------------------------------
//#define TESTHARNESSACTIVE// Define this when you are using a test harness for a module, disables main
// Private macro -------------------------------------------------------------

// Private variables ---------------------------------------------------------
//Provides a list of all initialization, updater, and handler functions for each modules, functions are defined in CONFIG.h
uint8_t (*InitList[EVENTLISTSIZE])(void) = EVENT_INITLIST;
Event_t (*UpdateList[EVENTLISTSIZE])(void) = EVENT_UPDATELIST;
uint8_t (*HandlerList[EVENTLISTSIZE])(Event_t update) = EVENT_HANDLERLIST;
//I2C_HandleTypeDef hi2c1;

#ifndef TESTHARNESSACTIVE
// Private function prototypes -----------------------------------------------

/**
 *
 * @brief  The application entry point. Implements a simple events and services routine. the event checkers are called updaters
 * `	   the services are called handlers
 * @author Caitlin Bonesio
 *
 */

int main(void)
{

	//MCU Configuration--------------------------------------------------------
	if( BOARD_Init() != INIT_OK){
		BOARD_CrashHandler();
	}

	// Initialize all configured peripherals
	if(UART_Init() != INIT_OK){
		BOARD_CrashHandler();
	}
	//list of all updates from the updaters to be passed to handlers
	Event_t updates[EVENTLISTSIZE];
	//Initialize all modules
	for(int i = 0; i < EVENTLISTSIZE; i++){
		if ((*InitList[i])() == EVENT_ERROR){
			return 0;//We've crashed
		}
	}
	while(1){
		//run all module event checkers
		for(int i = 0; i < EVENTLISTSIZE; i++){
			updates[i] = (*UpdateList[i])();//collect updates from the updaters
			if (updates[i].status == EVENT_ERROR){
				BOARD_CrashHandler();//We've crashed
			}
		}
		//run all module event handlers
		for(int i = 0; i < EVENTLISTSIZE; i++){
			if(updates[i].status != EVENT_NONE){//If there is an update
				if ((*HandlerList[i])(updates[i]) == EVENT_ERROR){//pass the updates to the handlers
					BOARD_CrashHandler();//We've crashed
				}
			}
		}

	}
}
#endif





/* USER CODE BEGIN 4 */

/* USER CODE END 4 */



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
