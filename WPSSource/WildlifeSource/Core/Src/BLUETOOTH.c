/*
 * BLUETOOTH.c
 *
 *  Created on: Mar 4, 2025
 *      Author: stipi
 */

#include "BLUETOOTH.h"
#include "BOARD.h"

#define BTBUFFERSIZE 256

typedef struct BTBuff {
	uint8_t data[BTBUFFERSIZE];
	uint8_t head;
	uint8_t tail;
	uint8_t full;
}BTBuff_t;

static BTBuff_t buffer;

void BLUETOOTH_BufferInit(){
	buffer.head = 0;
	buffer.tail = 0;
	buffer.full = 0;
}

uint8_t BLUETOOTH_WriteBuffer(uint8_t input){
	if(!buffer.full){
		buffer.data[buffer.head] = input;
		buffer.head++;
		buffer.head %= BTBUFFERSIZE;
		buffer.full = ((buffer.head+1)%BTBUFFERSIZE == buffer.tail);

		return 1;
	} else {
		return 0;
	}
}

uint8_t BLUETOOTH_ReadBuffer(){
	uint8_t ret = 0;
	if(buffer.head != buffer.tail || buffer.full){

		ret = buffer.data[buffer.tail];
		buffer.tail++;
		buffer.tail %= BTBUFFERSIZE;
		buffer.full = ((buffer.head+1)%BTBUFFERSIZE == buffer.tail);
	}
	return ret;
}
