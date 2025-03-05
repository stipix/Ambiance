/*
 * BOARD.h
 *
 *  Created on: Feb 7, 2025
 *  Author: Caitlin Bonesio
 */

#ifndef BOARD_H
#define BOARD_H


#include "stm32wb0x_hal.h"
#include "stm32wb0x_nucleo.h"

#define INIT_ERROR 0
#define INIT_OK 1

//Initializes the board
int BOARD_Init(void);

void BOARD_CrashHandler(void);

void Error_Handler(void);

#endif /* BOARD_H */
