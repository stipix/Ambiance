/*
 * CONFIG.h
 *
 *  Created on: Feb 7, 2025
 *      Author: Caitlin Bonesio
 */

#ifndef CONFIG_H
#define CONFIG_H
#include "BOARD.h"

typedef enum Event_Type{
	EVENT_NONE = 0,
	EVENT_BUTTONS,
}Event_Type;

typedef struct EventReturntype{
	uint16_t data;
	Event_Type status;
}Event_t;


#define EVENT_ERROR -1

//Include event module header files here
//#include "GPIO.h"

//Add the initialization, Updater, and handler functions to these lists
#define EVENTLISTSIZE 1
#include "GPIO.h"

#define EVENT_INITLIST {GPIO_Event_Init}
#define EVENT_UPDATELIST {GPIO_Event_Updater}
#define EVENT_HANDLERLIST {GPIO_Event_Handler}

#endif /* CONFIG_H */
