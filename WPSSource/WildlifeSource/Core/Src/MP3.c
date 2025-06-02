/*
 * Name MP3.c
 * Brief: a MP3 to use when creating event/service modules
 * Author: Caitlin Bonesio
 * Created: 4/19/25
 * Modified: 4/23/25
 */


//----------------------------------------Private Includes---------------------------------------
#include "CONFIG.h"
#include "MP3.h"
#include "TIMERS.h"
#include "FLASH.h"
#include "UART.h"
#include "Scheduler.h"

#include <stdio.h>
#include <string.h>
//----------------------------------------Private Defines----------------------------------------
#define CYCLELENGTH (10/*minutes*/*60000/*milliseconds/minute*/)//duty cycle time in milliseconds
//----------------------------------------Private Variables--------------------------------------
FIFO MP3queue;
static uint8_t play;//should the mp3 player be playing
static float DC;//how long until the next cycle of the duty cycle
static float volume;
static uint32_t starttime;
static uint32_t inittime;
static uint8_t initialized;
//----------------------------------------Private Functions--------------------------------------
void MP3_SendData(char string[4]){
	uint16_t accumulation = 0;
	LPUART_WriteTx(0x7E);//start flag
	accumulation += 0xFF;
	LPUART_WriteTx(0xFF);//Version?
	accumulation += 0x06;
	LPUART_WriteTx(0x06);//length

	for(int i = 0; i < 4; i++){
		accumulation += string[i];
		LPUART_WriteTx(string[i]);//command and data
	}
	accumulation = -accumulation;//??required for it to work
	LPUART_WriteTx(accumulation>>8);//checksum
	LPUART_WriteTx(accumulation&0xFF);//checksum
	LPUART_WriteTx(0xEF);//start flag

}
//----------------------------------------Public Functions---------------------------------------
/*
 * @Function: MP3_Event_Init
 * @Brief: Provides the initialization function for the events and serviced routine
 * @param: none
 * @return: An 8 bit integer flag reflecting The initialization status
 */
uint8_t MP3_Event_Init(FIFO Queue){
	//somehow need to be sent 3 seconds after the mp3 turns on
    MP3queue = Queue;
    TIMERS_Init();
    play = 1;
    DC = FLASH_GetDutyCycle()/100;
    volume = FLASH_GetVolume()/100;
    starttime = TIMERS_GetMilliSeconds();
    inittime = TIMERS_GetMilliSeconds();
    initialized = 0;
	{
	char send[4] = {0x0C, 0x00, 0x00, 0x00};//reset module
	MP3_SendData(send);
	}
	{
	char send[4] = {0x08, 0x00, 0x00, 0x01};//set source to the TF/microSD card
	MP3_SendData(send);
	}
    return INIT_OK;
}
/*
 * @Function: MP3_Event_Init
 * @Brief: Provides the ability for state machines to interact
 * @param: event to be posted
 * @return: none
 */
void MP3_Event_Post(Event_t event){
    FIFO_Enqueue(MP3queue, event);
}


/*
 * @Function: MP3_Event_Updater
 * @Brief: Provides the event checker that checks and posts the changes in the
 * @param: none
 * @return: An event
 */
Event_t MP3_Event_Updater(void){
    Event_t event = (Event_t){EVENT_NONE, 0};
    uint32_t timer = TIMERS_GetMilliSeconds();
    char rx = LPUART_ReadRx();
	if(rx != UARTFAILED){
		event.status = EVENT_LPUART;
		event.data = rx;
		MP3_Event_Post(event);
	}
    if (play){
		if((timer-starttime) >= CYCLELENGTH*DC){//if the speaker is playing, and time is up
			event.status = EVENT_TIMEOUT;
			event.data = 0;//next state of play
			starttime = timer;
			MP3_Event_Post(event);
		}
    } else {
		if(DC != 0 && ((timer-starttime) >= CYCLELENGTH*(1-DC))){// if the speaker is playing and time is up, DC = 0 don't play
			event.status = EVENT_TIMEOUT;
			event.data = 1;//next state of play
			starttime = timer;
			MP3_Event_Post(event);
		}
    }
    if(((timer-inittime) >= 3000) && !initialized){//if the speaker is playing, and time is up
		event.status = EVENT_INIT;
		event.data = 0;
		MP3_Event_Post(event);
	}
    if(volume != FLASH_GetVolume()/100 || DC != FLASH_GetDutyCycle()/100){
    	event.status = EVENT_SETTINGS;
    	event.data = (volume == FLASH_GetVolume()/100);
		MP3_Event_Post(event);
    }
    return event;
}

/*
 * @Function: MP3_Event_Handler
 * @Brief:
 * @param: Event_t event, incoming event for the handler to handle
 * @return: An 8 byte integer success flag, returns 0 if the program should crash
 */
uint8_t MP3_Event_Handler(Event_t event){
	if(event.status == EVENT_INIT){
		//7E FF 06 08 00 00 01 xx EF //repeat folder
		char send[4] = {0x08, 0x00, 0x00, 0x01};
		MP3_SendData(send);
		initialized = 1;
	}
	if(event.status == EVENT_TIMEOUT){
		//send either play or stop to the mp3 player
		play = event.data;
		if(play){
			char send[4] = {0x0D, 0x00, 0x00, 0x00};
			MP3_SendData(send);
		}else {

			char send[4] = {0x0E, 0x00, 0x00, 0x00};
			MP3_SendData(send);
		}

	}if(event.status == EVENT_PLAY){

		Scheduler_Event_Post(event);
		if(event.data == 0){//0 is not a valid folder or track number
			DC = 0;// immediately cause a timeout to stop the speaker
			char send[4] = {0x0A, 0x00, 0x00, 0x00};//enter low power mode
			MP3_SendData(send);
		} else {

			char send1[4] = {0x0B, 0x00, 0x00, 0x00};//exit low power mode?
			MP3_SendData(send1);
			//7E FF 06 0F 00 FOLDER# TRACK# xx EF

			char send2[4] = {0x0F, 0x00, event.data>>8, event.data&0xFF};
			MP3_SendData(send2);
		}


	}if(event.status == EVENT_SETTINGS){
		DC = FLASH_GetDutyCycle()/100;
		volume = FLASH_GetVolume()/100;
		if(!event.data){//volume == FLASH_GetVolume()/100
			//send new volume to the mp3 player
			char send[4] = {0x06, 0x00, 0x00, (uint8_t)(volume*30)};
			MP3_SendData(send);
		}
	}if (event.status == EVENT_LPUART){
		//don't do anything for now, may parse errors and moving to next track later
	}
    return 1;
}

//#define MP3TESTHARNESS

#ifdef MP3TESTHARNESS
#include "BOARD.h"
#include "discountIO.h"

int main(){
	BOARD_Init();
//	BSP_LED_Init(LED_BLUE);
//	BSP_LED_Init(LED_RED);
	UARTs_Init();
	{
	char send[4] = {0x0C, 0x00, 0x00, 0x00};//reset module
	MP3_SendData(send);
	HAL_Delay(1000);
	}
	{
	char send[4] = {0x08, 0x00, 0x00, 0x01};//set source to the TF/microSD card
	MP3_SendData(send);
	HAL_Delay(1000);
	}

//	char send1[4] = {0x0B, 0x00, 0x00, 0x00};//exit low power mode?
//	MP3_SendData(send1);
	//7E FF 06 0F 00 FOLDER#1 TRACK#1 xx EF
//	HAL_Delay(1000);
//	char send2[4] = {0x0F, 0x00, 0x01, 0x01};
//	MP3_SendData(send2);
	{
	char send[4] = {0x47, 0x00, 0x00, 0x00};//querie the number of files on the micrSD
	MP3_SendData(send);
	}

	while (1){//print response to console
		char rx = LPUART_ReadRx();
		if(rx != UARTFAILED){
			USART_WriteTx(rx);
		}

	}

}
#endif
