/*
 * Name template.c
 * Brief: a template to use when creating event/service modules
 * Author: Caitlin Bonesio
 * Created: 4/15/25
 * Modified: 4/15/25
 */


//----------------------------------------Public Includes----------------------------------------
#include "CONFIG.h"

//----------------------------------------Public Functions---------------------------------------
/*
 * @Function: TEMPLATE_Event_Init
 * @Brief: Provides the initialization function for the events and serviced routine
 * @param: none
 * @return: An 8 bit integer flag reflecting The initialization status
 */
uint8_t TEMPLATE_Event_Init(void){
    return INIT_OK
}

/*
 * @Function: TEMPLATE_Event_Updater
 * @Brief: Provides the event checker that checks and posts the changes in the 
 * @param: none
 * @return: An event 
 */
Event_t TEMPLATE_Event_Updater(void){
    Event_t out = (Event_t){EVENT_NONE, 0};
    return out;
}

/*
 * @Function: TEMPLATE_Event_Handler
 * @Brief: 
 * @param: Event_t event, incoming event for the handler to handle
 * @return: An 8 byte integer success flag, returns 0 if the program should crash
 */
uint8_t TEMPLATE_Event_Handler(Event_t event){
    return 1;
}

