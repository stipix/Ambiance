/*
 * Name ButtonsMenuSM.c
 * Brief: a module that allows the user to change setting through the pcb's push buttons
 * Author: Caitlin Bonesio
 * Created: 4/23/25
 * Modified: 4/23/25
 */


//----------------------------------------Private Includes---------------------------------------
#include "CONFIG.h"
#include "ButtonsMenuSM.h"
#include "MP3.h"
#include "GPIO.h"
#include "I2C.h"
#include "TIMERS.h"
#include "FLASH.h"
//----------------------------------------Private Defines----------------------------------------
#define DEBOUNCETIME 100//milliseconds
#define SLEEPTIMER   30000
//----------------------------------------Private Typedefs---------------------------------------
typedef enum ButtonsMenuStates{
	main,
	folderControl,
	options,
	scheduleDay,
	scheduleStart,
	scheduleStop,
	scheduleFolder,
}ButtonsMenuStates_t;
//----------------------------------------Private Variables--------------------------------------
FIFO ButtonsMenuSMqueue;

static ButtonsMenuStates_t BMState;
static uint8_t displayoff;

static uint8_t cursorpos;//display cursors
static uint8_t folderselect;
static uint8_t trackselect;
static uint8_t dayselect;
static uint8_t Stimeselect;
static uint8_t Etimeselect;
//----------------------------------------Public Functions---------------------------------------
/*
 * @Function: ButtonsMenuSM_Event_Init
 * @Brief: Provides the initialization function for the events and serviced routine
 * @param: none
 * @return: An 8 bit integer flag reflecting The initialization status
 */
uint8_t ButtonsMenuSM_Event_Init(FIFO Queue){
    ButtonsMenuSMqueue = Queue;
    BMState = main;
    GPIO_Init();
    I2C_Init();
	TIMERS_Init();
    return INIT_OK;
}
/*
 * @Function: ButtonsMenuSM_Event_Init
 * @Brief: Provides the ability for state machines to interact
 * @param: event to be posted
 * @return: none
 */
void ButtonsMenuSM_Event_Post(Event_t event){
    FIFO_Enqueue(ButtonsMenuSMqueue, event);
}



/*
 *sample every type of button Event

if(event.status == EVENT_BUTTONS){
	if(event.data & B1XORMASK && event.data & B1MASK){
		nextstate = main;
		transition = 1;
	} else
	if(event.data & B2XORMASK && event.data & B2MASK){

	} else
	if(event.data & B3XORMASK && event.data & B3MASK){

	} else
	if(event.data & B4XORMASK && event.data & B4MASK){
	} else
	if(event.data & B5XORMASK && event.data & B5MASK){
	} else
	if(event.data & B6XORMASK && event.data & B6MASK){
	}
}
 */

/*
 * @Function: ButtonsMenuSM_Event_Updater
 * @Brief: Provides the event checker that checks and posts the changes in the
 * @param: none
 * @return: An event
 */
Event_t ButtonsMenuSM_Event_Updater(void){

    static uint32_t debounce = 0;
    static uint8_t lastbuttons = 0;
    static uint32_t timer = 0;

    Event_t event = (Event_t){EVENT_NONE, 0};

    uint8_t currbuttons = GPIO_ReadButtons();
    if(curbuttons != lastbuttons && debounce >= TIMERS_GetMilliSeconds() + DEBOUNCETIME){
    	event.status = EVENT_BUTTONS;
    	event.data = (currbuttons ^ lastbuttons) << 8 + currbuttons;
    	lastbuttons = currbuttons;
    	debounce = TIMERS_GetMilliSeconds();
    	timer = TIMERS_GetMilliSeconds();//delay the sleep timer
    	ButtonsMenuSM_Event_Post(event);
    }
    if(timer & (timer >= TIMERS_GetMilliSeconds() + SLEEPTIMER)){
    	event.status = EVENT_TIMEOUT;
    	event.data = 0;
    	timer = 0;// disable sleep timer until next button press
    	ButtonsMenuSM_Event_Post(event);

    }
    return event;
}

/*
 * @Function: ButtonsMenuSM_Event_Handler
 * @Brief:
 * @param: Event_t event, incoming event for the handler to handle
 * @return: An 8 byte integer success flag, returns 0 if the program should crash
 */
uint8_t ButtonsMenuSM_Event_Handler(Event_t event){
	if(event.status == EVENT_TIMEOUT){
		displayoff = 1;
		BMState = main;
	}
	if(displayoff){
		if(event.status = EVENT_BUTTONS){
			displayoff = 0;
		}
	} else {

		uint8_t transition = 0;
		ButtonsMenuStates_t nextstate = BMState;
		switch (BMState){
		case main:
			if(event.status == EVENT_BUTTONS){
				if(event.data & B1XORMASK && event.data & B1MASK){
					nextstate = folderControl;
					transition = 1;
				} else
				if(event.data & B2XORMASK && event.data & B2MASK){
					nextstate = scheduleDay;
					transition = 1;

				} else
				if(event.data & B3XORMASK && event.data & B3MASK){
					nextstate = options;
					transition = 1;

				} else
				if(event.data & B5XORMASK && event.data & B5MASK){
					FLASH_SetDCVol(FLASH_GetVolume()+1, FLASH_GetDutyCycle());
				} else
				if(event.data & B6XORMASK && event.data & B6MASK){
					FLASH_SetDCVol(FLASH_GetVolume()-1, FLASH_GetDutyCycle());
				}

			}
			break;
		case folderControl:
			if(event.status == EVENT_ENTRY){
				cursorpos = 0;
				folderselect = 0;
				trackselect = 0;
			}
			if(event.status == EVENT_BUTTONS){
				if(event.data & B1XORMASK && event.data & B1MASK){
					nextstate = main;
					transition = 1;
				} else
				if(event.data & B2XORMASK && event.data & B2MASK){
					MP3_Event_Post(((Event_t){EVENT_PLAY, ((folderselect+1)<<8) + (trackselect+1)}));
					nextstate = main;
					transition = 1;

				} else
				if(event.data & B3XORMASK && event.data & B3MASK){
					if(cursorpos){
						folderselect--;
						folderselect %= 256;
					} else {
						trackselect--;
						trackselect %= 256;
					}

				} else
				if(event.data & B4XORMASK && event.data & B4MASK){
					if(cursorpos){
						folderselect++;
						folderselect %= 256;
					} else {
						trackselect++;
						trackselect %= 256;
					}
					break;
				} else
				if(event.data & B5XORMASK && event.data & B5MASK){
					cursorpos = !cursorpos;
				} else
				if(event.data & B6XORMASK && event.data & B6MASK){
					cursorpos = !cursorpos;
				}
			}
			break;
		case options:
		if(event.status == EVENT_ENTRY){
			cursorpos = 0;
			folderselect = 0;//left right control value
			trackselect = 0;//up down select option
		}
		if(event.status == EVENT_BUTTONS){
			if(event.data & B1XORMASK && event.data & B1MASK){
				nextstate = main;
				transition = 1;
			} else
			if(event.data & B3XORMASK && event.data & B3MASK){
				cursorpos = !cursorpos;
			} else
			if(event.data & B4XORMASK && event.data & B4MASK){
				cursorpos = !cursorpos;
				break;
			} else
			if(event.data & B5XORMASK && event.data & B5MASK){
				if(cursorpos){
					uint8_t accumulation = 0;
					LPUART_WriteTx(0x7E);//start flag
					accumulation += 0xFF;
					LPUART_WriteTx(0xFF);//Version?
					accumulation += 0x06;
					LPUART_WriteTx(0x06);//length
					LPUART_WriteTx(0x02);//Previous track command
					accumulation += 0x02;
					LPUART_WriteTx(0x00);
					LPUART_WriteTx(0x00);
					LPUART_WriteTx(0x00);
					LPUART_WriteTx(accumulation);//checksum
					LPUART_WriteTx(0xEF);//start flag
				} else {
					FLASH_SetDCVol(FLASH_GetVolume(), FLASH_GetDutyCycle()-1);
				}
			} else
			if(event.data & B6XORMASK && event.data & B6MASK){
				if(cursospos){

					uint8_t accumulation = 0;
					LPUART_WriteTx(0x7E);//start flag
					accumulation += 0xFF;
					LPUART_WriteTx(0xFF);//Version?
					accumulation += 0x06;
					LPUART_WriteTx(0x06);//length
					LPUART_WriteTx(0x01);//Next track command
					accumulation += 0x01;
					LPUART_WriteTx(0x00);
					LPUART_WriteTx(0x00);
					LPUART_WriteTx(0x00);
					LPUART_WriteTx(accumulation);//checksum
					LPUART_WriteTx(0xEF);//start flag
				}else {
					FLASH_SetDCVol(FLASH_GetVolume(), FLASH_GetDutyCycle()+1);
				}
			}
		}

			break;
		case scheduleDay:
			if(event.status == EVENT_ENTRY){
				dayselect = 0;
			}
			if(event.status == EVENT_BUTTONS){
				if(event.data & B1XORMASK && event.data & B1MASK){
					nextstate = main;
					transition = 1;
				} else
				if(event.data & B2XORMASK && event.data & B2MASK){
					dayselect++;
					nextstate = scheduleStart;
					transition = 1;
				} else
				if(event.data & B3XORMASK && event.data & B3MASK){
					dayselect--;
					dayselect%31;
				} else
				if(event.data & B4XORMASK && event.data & B4MASK){
					dayselect++;
					dayselect%31;
				}
			}
			break;
		case scheduleStart:
			if(event.status == EVENT_ENTRY){
				Stimeselect == 0;
			}
			if(event.status == EVENT_BUTTONS){
				if(event.data & B1XORMASK && event.data & B1MASK){
					nextstate = scheduleDay;
					transition = 1;
				} else
				if(event.data & B2XORMASK && event.data & B2MASK){
					nextstate = scheduleStop;
					transition = 1;
				} else
				if(event.data & B3XORMASK && event.data & B3MASK){
					if(Stimeselect == 0){
						Stimeselect = 0b10111011;//23:45
					} else {
						Stimeselect--;
						Stimeselct &= 0b00000100;
					}

				} else
				if(event.data & B4XORMASK && event.data & B4MASK){
					if(Stimeselect == 0b10111011){//23:45
						Stimeselect = 0;//0:00
					} else {
						Stimeselect++;
						if(Stimeselect & 0b00000100){
							Stimeselect += 0b00000100;
						}
					}
				}
			}
			break;
		case scheduleStop:
			if(event.status == EVENT_ENTRY){
				Etimeselect == 0;
			}
			if(event.status == EVENT_BUTTONS){
				if(event.data & B1XORMASK && event.data & B1MASK){
					nextstate = scheduleStart;
					transition = 1;
				} else
				if(event.data & B2XORMASK && event.data & B2MASK){
					nextstate = scheduleFolder;
					transition = 1;
				} else
				if(event.data & B3XORMASK && event.data & B3MASK){
					if(Etimeselect == 0){
						Etimeselect = 0b10111011;//23:45
					} else {
						Etimeselect--;
						Etimeselct &= 0b00000100;
					}

				} else
				if(event.data & B4XORMASK && event.data & B4MASK){
					if(Etimeselect == 0b10111011){//23:45
						Etimeselect = 0;//0:00
					} else {
						Etimeselect++;
						if(Etimeselect & 0b00000100){
							Etimeselect += 0b00000100;
						}
					}
				}
			}
			break;
		case scheduleFolder:
			if(event.status == EVENT_ENTRY){
				cursorpos = 0;
				folderselect = 0;
				trackselect = 0;
			}
			if(event.status == EVENT_BUTTONS){
				if(event.data & B1XORMASK && event.data & B1MASK){
					nextstate = scheduleStop;
					transition = 1;
				} else
				if(event.data & B2XORMASK && event.data & B2MASK){
					scheduleEvent sevent;
					sevent.month = 0;
					sevent.day = dayselect;
					sevent.start = Stimeselect;
					sevent.stop = Etimeselect;
					sevent.track = trackselect+1;
					sevent.folder = folderselect+1;
					FLASH_ApeendSchedule(sevent);
					nextstate = main;
					transition = 1;

				} else
				if(event.data & B3XORMASK && event.data & B3MASK){
					if(cursorpos){
						folderselect--;
						folderselect %= 256;
					} else {
						trackselect--;
						trackselect %= 256;
					}

				} else
				if(event.data & B4XORMASK && event.data & B4MASK){
					if(cursorpos){
						folderselect++;
						folderselect %= 256;
					} else {
						trackselect++;
						trackselect %= 256;
					}
					break;
				} else
				if(event.data & B5XORMASK && event.data & B5MASK){
					cursorpos = !cursorpos;
				} else
				if(event.data & B6XORMASK && event.data & B6MASK){
					cursorpos = !cursorpos;
				}
			}
			break;
		default:
			break;
		}
		if(transition){
			ButtonsMenuSM_Event_Handler((Event_t){EVENT_EXIT});
			BMState = next;
			ButtonsMenuSM_Event_Handler((Event_t){EVENT_ENTRY});
		}
	}
    return 1;
}


