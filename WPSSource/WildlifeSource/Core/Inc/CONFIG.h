/*
 * Name CONFIG.h
 * Brief: provides a way to easily configure the main loop and add more modules
 * Author: Caitlin Bonesio
 * Created: 2/7/25
 * Modified: 3/10/25
 */

#ifndef CONFIG_H
#define CONFIG_H


#ifdef __cplusplus//allows code to be ran in a c++ program
extern "C" {
#endif

//----------------------------------------Public Includes----------------------------------------
#include "BOARD.h"

//----------------------------------------Public Typedefs----------------------------------------

typedef enum Event_Type{
	EVENT_NONE = 0,
	EVENT_BUTTONS,
	EVENT_I2C,
}Event_Type;

typedef struct EventReturntype{
	uint16_t data;
	Event_Type status;
}Event_t;

//----------------------------------------Public Defines-----------------------------------------


#define EVENT_ERROR -1

//Include event module header files here
#include "GPIO.h"

//Add the initialization, Updater, and handler functions to these lists
#define EVENTLISTSIZE 1

#define EVENT_INITLIST {GPIO_Event_Init}
#define EVENT_UPDATELIST {GPIO_Event_Updater}
#define EVENT_HANDLERLIST {GPIO_Event_Handler}


#ifdef __cplusplus//allows code to be ran in a c++ program
}
#endif


#endif /* CONFIG_H */
