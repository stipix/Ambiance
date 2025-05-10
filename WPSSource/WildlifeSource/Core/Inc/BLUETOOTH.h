/*
 * Name BLUETOOTH.h
 * Brief: Provides a TX buffer for the virtual USART held over the bluetooth connection
 * Author: Caitlin Bonesio
 * Created: 4/12/25
 * Modified: 5/9/25
 */

 #ifdef __cplusplus//allows code to be ran in a c++ program
 extern "C" {
 #endif

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "BOARD.h"

void BLUETOOTH_BufferInit();

uint8_t BLUETOOTH_WriteBuffer(uint8_t input);

uint8_t BLUETOOTH_ReadBuffer();

#endif /* INC_BLUETOOTH_H_ */
#ifdef __cplusplus//allows code to be ran in a c++ program
 }
#endif
