/*
 * CONFIG.h
 *
 *  Created on: Feb 7, 2025
 *      Author: stipi
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

typedef struct UpdateReturntype{
	uint16_t data;
	uint8_t status;
}Update_t;
#define ERROR -1

//Include Update module header files here

//Add the initialization, Updater, and handler functions to these lists
#define EVENTLISTSIZE 0
uint8_t (*InitList[EVENTLISTSIZE])(void) = {};
Update_t (*UpdateList[EVENTLISTSIZE])(void) = {};
uint8_t (*HandlerList[EVENTLISTSIZE])(Update_t update) = {};

#endif /* INC_CONFIG_H_ */
