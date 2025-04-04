/*
 * Name GPIO.c
 * Brief: provides a library to initialize and read from the external buttons.
 * 		  Also provides the functions to implement the buttons into the events and services main
 * Author: Caitlin Bonesio
 * Created: 2/5/25
 * Modified: 3/7/25
 */


//----------------------------------------Private Includes---------------------------------------
#include "GPIO.h"

//----------------------------------------Private Defines----------------------------------------
#define BUTTON_0_PIN GPIO_PIN_10
#define BUTTON_1_PIN GPIO_PIN_2
#define BUTTON_2_PIN GPIO_PIN_3
#define BUTTON_3_PIN GPIO_PIN_4
#define BUTTON_4_PIN GPIO_PIN_12
#define BUTTON_5_PIN GPIO_PIN_13

#define BUTTON_0 (GPIOA, BUTTON_0_PIN)
#define BUTTON_1 (GPIOA, BUTTON_1_PIN)
#define BUTTON_2 (GPIOA, BUTTON_2_PIN)
#define BUTTON_3 (GPIOB, BUTTON_3_PIN)
#define BUTTON_4 (GPIOB, BUTTON_4_PIN)
#define BUTTON_5 (GPIOB, BUTTON_5_PIN)

//----------------------------------------Private Variable---------------------------------------
static uint8_t buttons = 0;

//----------------------------------------Public Functions---------------------------------------

/*
 * @function: GPIO_Init()
 * @brief: initialized GPIO pins to receive 6 external buttons
 * @param: none
 * @return: Init Status, whether the operation failed or succeeded
 */
int GPIO_Init(){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//Configure GPIO pins on port A
	GPIO_InitStruct.Pin = BUTTON_0_PIN | BUTTON_1_PIN | BUTTON_2_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//Configure GPIO pins : PB15 PB13
	GPIO_InitStruct.Pin = BUTTON_3_PIN | BUTTON_4_PIN | BUTTON_5_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	return INIT_OK;
}


/*
 * @Function: GPIO_ReadButtons
 * @Brief: Allows an easy way to read the external button states
 * @param: none
 * @return: An 8 bit integer where the LSB is button 0, and the 6th bit is button 5
 */
//uint8_t GPIO_ReadButtons(void){
//	return ((HAL_GPIO_ReadPin(BUTTON_0) << 5) &
//			(HAL_GPIO_ReadPin(BUTTON_0) << 4) &
//			(HAL_GPIO_ReadPin(BUTTON_0) << 3) &
//			(HAL_GPIO_ReadPin(BUTTON_0) << 2) &
//			(HAL_GPIO_ReadPin(BUTTON_0) << 1) &
//			(HAL_GPIO_ReadPin(BUTTON_0) << 0));
//}

//For use before receiving buttons
/*
 * @Function: GPIO_ReadButtons
 * @Brief: Allows an easy way to read the internal button states
 * @param: none
 * @return: An 8 bit integer where the LSB is button 0, and the 3rd bit is button 3
 */
uint8_t GPIO_ReadButtons(void){
	return ((!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) << 2) |
			(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) << 1) |
			(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) << 0));
}

/*
 * @Function: GPIO_Event_Init
 * @Brief: Provides the initialization function for the events and serviced routine
 * @param: none
 * @return: An 8 bit integer flag reflecting The initialization status
 */
uint8_t GPIO_Event_Init(void){
	buttons = 0;
	//Initialize dev board leds, for debug purposes
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Init(LED_GREEN);
	BSP_LED_Init(LED_RED);

	/* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
	BSP_PB_Init(B1, BUTTON_MODE_GPIO);
	BSP_PB_Init(B2, BUTTON_MODE_GPIO);
	BSP_PB_Init(B3, BUTTON_MODE_GPIO);
	return GPIO_Init();

}

/*
 * @Function: GPIO_Event_Updater
 * @Brief: Provides the event checker that checks and posts the changes in the buttons and the current buttons states
 * @param: none
 * @return: An event where the 16bit data is formatted at the 8 most significant bits representing the current states
 * 		 	and the least significant 8 bits representing the changes in the buttons states
 */
Event_t GPIO_Event_Updater(void){
	Event_t update = {0, 0};//Initialize to no event
	uint8_t newButtons = GPIO_ReadButtons();
	if(newButtons != buttons){
		update.data = (uint16_t)(buttons ^ newButtons) | (((uint16_t)newButtons) << 8);
		update.status = EVENT_BUTTONS;
		buttons = newButtons;
	}
	return update;
}

/*
 * @Function: GPIO_Event_Handler
 * @Brief: Currently changes the LEDs on the devboard based on the updates
 * @param: Event_t event, incoming event for the handler to handle
 * @return: An 8 byte integer success flag, returns 0 if the program should crash
 */
uint8_t GPIO_Event_Handler(Event_t event){
	if(event.status == EVENT_BUTTONS){
		if((event.data>>0)&0x01){
			BSP_LED_Toggle(LED_BLUE);
		}
		if((event.data>>1)&0x01){
			BSP_LED_Toggle(LED_GREEN);
		}
		if((event.data>>2)&0x01){
			BSP_LED_Toggle(LED_RED);
		}
	}
	return 1;
}
