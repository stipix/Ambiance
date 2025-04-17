/*
 * Name COMM.c
 * Brief: a module to listen for signals from the GUI interface
 * Author: Caitlin Bonesio
 * Created: 4/15/25
 * Modified: 4/15/25
 */

//----------------------------------------Private Includes---------------------------------------
#include "COMM.h"
#include "FIFO.h"
#include "UART.h"
#include "DiscountIO.h"
#include <stdbool.h>

//----------------------------------------Private Defines----------------------------------------
#define VOLUMECONTROL   0x00
#define FOLDERCONTROL   0x01
#define LOGSREQUEST     0x02
#define LOGSDONE        0x03
#define DCCONTROL       0x04
#define SCHEDULECONTROL 0x05
#define SCHEDULEEND     0x0D

#define MAXSCHEDULEEVENTS 180

#define PLACEHOLDER    (10*6)//ten events

//----------------------------------------Private Typedefs---------------------------------------
 typedef enum COMMSTATES{
	 init = 0,
	 idle,
	 volumecontrol,
	 foldercontrol,
	 folderselected,
	 logsrequest,
	 logsdata,
	 dccontrol,
	 schedulecontrol,
	 schedulemonth,
	 scheduleday,
	 schedulestart,
	 schedulestop,
	 schedulefolder,
	 scheduletrack,
	 scheduleend,
 }COMMSTATES_t;


//----------------------------------------Private Variables--------------------------------------
 COMMSTATES_t commSM;
 FIFO COMMqueue;
 uint8_t sendinglogs;
//----------------------------------------Public Functions---------------------------------------
/*
 * @Function: EVENT_COMM_Event_Init
 * @Brief: Provides the initialization function for the events and serviced routine
 * @param: none
 * @return: An 8 bit integer flag reflecting The initialization status
 */
uint8_t COMM_Event_Init(FIFO Queue){
	commSM = init;
	COMMqueue = Queue;
	COMM_Event_Post((Event_t){EVENT_INIT, 0});
	return INIT_OK;
}

void COMM_Event_Post(Event_t event){
	FIFO_Enqueue(COMMqueue, event);
}

/*
 * @Function: EVENT_COMM_Event_Updater
 * @Brief: Provides the event checker that checks and posts the changes in the 
 * @param: none
 * @return: An event 
 */
Event_t COMM_Event_Updater(void){
	uint8_t event = 0;
    Event_t out = (Event_t){EVENT_NONE, 0};
    char input = USART_ReadRx();
    if(input != UARTFAILED){
    	event = 1;
    	out.status = EVENT_USART;
    	out.data = (uint16_t)input;
    }
    if(sendinglogs == 1){
    	if(USART_TxEmpty()){
    		FIFO_Enqueue(COMMqueue, (Event_t){EVENT_USART_READY, 0});
    	}
    }
	if(event){FIFO_Enqueue(COMMqueue, out);}
    return out;
}

/*
 * @Function: EVENT_COMM_Event_Handler
 * @Brief: 
 * @param: Event_t event, incoming event for the handler to handle
 * @return: An 8 byte integer success flag, returns 0 if the program should crash
 */
uint8_t COMM_Event_Handler(Event_t event){
	bool transition = 0;
	COMMSTATES_t next = commSM;
	char text[50];
	switch(commSM){
	case init:
		if(event.status == EVENT_INIT){
			next = idle;
			transition = true;
		}
		break;
	case idle:
		if(event.status == EVENT_ENTRY){
			sendinglogs = 0;
		}
		if(event.status == EVENT_USART){
			switch (event.data){
			case VOLUMECONTROL:
				discountprintf("received volume control\n");
				next = volumecontrol;
				transition = true;
				break;
			case FOLDERCONTROL:
				discountprintf("received folder control\n");
				next = foldercontrol;
				transition = true;
				break;
			case LOGSREQUEST:
				discountprintf("received logs request\n");
				next = logsrequest;
				transition = true;
				break;
			case DCCONTROL:
				discountprintf("received DC control\n");
				next = dccontrol;
				transition = true;
				break;
			case SCHEDULECONTROL:
				discountprintf("received schedule control\n");
				next = schedulecontrol;
				transition = true;
				break;
			default:
				break;
			}
		}
		break;
	case volumecontrol:
		if(event.status == EVENT_USART){
			discountprintf("Storing volume\n");
			next = idle;
			transition = true;
		}
		break;
	case foldercontrol:
		if(event.status == EVENT_USART){
			discountprintf("Sending folder selector\n");
			next = folderselected;
			transition = true;
		}
		break;
	case folderselected:
		if(event.status == EVENT_USART){
			discountprintf("Sending track selector\n");
			next = idle;
			transition = true;
		}
		break;
	case logsrequest:
		static uint32_t sent;
		if(event.status == EVENT_ENTRY){
			sendinglogs = 1;
			sent = 0;
			uint16_t size = PLACEHOLDER;//placeholder until flash library is complete
			USART_WriteTx((uint8_t)(size>>8));
			USART_WriteTx((uint8_t)(size));
			next = logsdata;
			transition = true;
		}
		break;
	case logsdata:
		if(event.status == EVENT_USART_READY){
			//get logs size
			uint8_t data = 'A';//placeholder until flash library is complete
			uint16_t size = PLACEHOLDER;
			if(size - sent > USARTBUFFERSIZE){
				//send next part of logs here, use sent to index the
				for(int i = 0; i < USARTBUFFERSIZE; i++){
					USART_WriteTx(data);
				}
				sent = sent + USARTBUFFERSIZE;
				next = logsdata;
				transition = true;
				discountprintf("data partly sent\n");
			} else {
				for(int i = 0; i < size - sent; i++){
					USART_WriteTx(data);
				}
				//send last of logs here
				next = idle;
				transition = true;
				discountprintf("data sent\n");
			}
		}
		break;
	case dccontrol:
		if(event.status == EVENT_USART){
			discountprintf("storing duty cycle\n");
			//store duty cycle here
			next = idle;
			transition = true;
		}
		break;
	case schedulecontrol:
		static uint8_t numevents;
		if(event.status == EVENT_ENTRY){
			discountprintf("receiving schedule\n");
			next = schedulemonth;
			transition = true;
			numevents = 0;
		}
		break;
	case schedulemonth:
		if(event.status == EVENT_USART){
			if(event.data == SCHEDULEEND){
				discountprintf("schedule complete\n");
				next = idle;
				transition = true;
			} else if(numevents > MAXSCHEDULEEVENTS){
				discountprintf("schedule forced to complete\n");

				//record a schedule end here
				next = scheduleend;
				transition = true;
				USART_WriteTx(SCHEDULEEND);//please stop sending me the schedule
			}else{
				sprintf(text, "Month: %d\n", event.data);
				discountprintf(text);
				//store month here
				next = scheduleday;
				transition = true;
			}
		}
		break;
	case scheduleday:
		if(event.status == EVENT_USART){
			sprintf(text, "Day: %d\n", event.data);
			discountprintf(text);
			//record day here
			next = schedulestart;
			transition = true;
		}
		break;
	case schedulestart:
		if(event.status == EVENT_USART){
			sprintf(text, "start time: %d:%d\n", event.data&0b11111000>>3, (event.data & 0b011)*15);
			discountprintf(text);
			//record start time here
			next = schedulestop;
			transition = true;
		}
		break;
	case schedulestop:
		if(event.status == EVENT_USART){
			sprintf(text, "start time: %d:%d\n", event.data&0b11111000>>3, (event.data & 0b011)*15);
			discountprintf(text);
			//record end time here
			next = schedulefolder;
			transition = true;
		}
		break;
	case schedulefolder:
		if(event.status == EVENT_USART){
			sprintf(text, "folder#: %d\n", event.data);
			discountprintf(text);
			//record end time here
			next = scheduletrack;
			transition = true;
		}
		break;
	case scheduletrack:
		if(event.status == EVENT_USART){
			sprintf(text, "track#: %d\n", event.data);
			discountprintf(text);
			//record end time here
			next = schedulemonth;
			transition = true;
			numevents++;
		}
		break;
	case scheduleend:
		if(event.status == EVENT_USART){
			if(event.data == SCHEDULEEND){
				next = idle;
				transition = true;
			}
		}
		break;
	default:
		break;
	}
	if(transition){
		COMM_Event_Handler((Event_t){EVENT_EXIT});
		commSM = next;
		COMM_Event_Handler((Event_t){EVENT_ENTRY});
	}
	return 1;
}

