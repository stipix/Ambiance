//#include "BOARD.h"
#include "GPIO.h"


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pins : PA8 PA9 PA10 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  //Configure GPIO pins : PB15 PB13
  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);



}

int GPIO_Init(){
	MX_GPIO_Init();
	return INIT_OK;
}

static uint8_t buttons = 0;

//For use in full circuit
//uint8_t GPIO_ReadButtons(void){
//	return ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8 ) << 5) &
//			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9 ) << 4) &
//			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) << 3) &
//			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) << 2) &
//			(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) << 1) &
//			(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) << 0));
//}

//For use before receiving buttons
uint8_t GPIO_ReadButtons(void){
	return ((!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) << 2) |
			(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) << 1) |
			(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) << 0));
}

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

Event_t GPIO_Event_Updater(void){
	Event_t update = {0, 0};
	uint8_t newButtons = GPIO_ReadButtons();
	if(newButtons != buttons){
		update.data = (uint16_t)(buttons ^ newButtons);
		update.status = EVENT_BUTTONS;
		buttons = newButtons;
	}
	return update;
}

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
