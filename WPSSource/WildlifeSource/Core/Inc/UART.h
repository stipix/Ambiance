/*
 * Name UART.h
 * Brief: provides a library to initialize and access a free running timer
 * Author: Caitlin Bonesio
 * Created: 2/5/25
 * Modified: 3/10/25
 */
#ifndef UART_H
#define UART_H

#ifdef __cplusplus//allows code to be ran in a c++ program
extern "C" {
#endif

#include "BOARD.h"


/*
 * @function: UART_Init()
 * @brief: initialize the lpuart module
 * @param: none
 * @return: Init Status, whether the operation failed or succeeded
 */
int UART_Init(void);

/*
 * @function: UART_ReadRx()
 * @brief: reads one received byte from the lpuart
 * @param: none
 * @return: the character received, is 0x00 if no character to read
 */
char UART_ReadRx(void);

/*
 * @function: UART_WriteTx()
 * @brief: sends one byte through the lpuart
 * @param: none
 * @return: none
 */
void UART_WriteTx(char input);

#ifdef __cplusplus//allows code to be ran in a c++ program
}
#endif

#endif
