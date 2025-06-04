/*
 * Name Scheduler.c
 * Brief: the moudle that reads the schedule from the flash memory and the time from the rtc clock
 * 		  to control what to play and when.
 * 		  Also creates system logs
 * Author: Caitlin Bonesio
 * Created: 4/21/25
 * Modified: 4/23/25
 */


//----------------------------------------Private Includes---------------------------------------
#include "CONFIG.h"
#include "Scheduler.h"
#include "I2C.h"
#include "FLASH.h"
#include "TIMERS.h"
#include "discountIO.h"
//----------------------------------------Private Defines----------------------------------------
#define REFRESHRATE (1/*minutes*/*60000/*milliseconds/minute*/)
#define NULLDATE    0xFF
//----------------------------------------Private Variables--------------------------------------
FIFO Schedulerqueue;
static uint32_t starttime;//timer

static uint8_t month;//I2C data
static uint8_t day;
static uint8_t hour;
static uint8_t minute;

static uint8_t logging;//logging
static uint8_t playdata;
//----------------------------------------Private Functions--------------------------------------
void CompareTime(){
	if(month == NULLDATE || day == NULLDATE || hour == NULLDATE || minute == NULLDATE){
		return;
	}
	discountprintf("Date received");
	if(logging){
		scheduleEvent event;

		event.month = month;
		event.day = day;
		if(playdata){
			event.start = ((hour & 0b11111) << 3) & ((uint8_t)(minute/15));
			event.stop = 0;
			event.folder = (playdata>>8)&0xFF;
			event.track = playdata&0xFF;
		} else {
			scheduleEvent prevevent = FLASH_ReadLogs(FLASH_GetLogsSize()-1);
			event.start = 0;
			event.stop = ((hour & 0b11111) << 3) & ((uint8_t)(minute/15));
			event.folder = prevevent.folder;
			event.track = prevevent.track;
		}
		FLASH_AppendLogs(event);
		month = NULLDATE;
		day = NULLDATE;
		hour = NULLDATE;
		minute = NULLDATE;
	} else {
		scheduleEvent event;
		for(int i = 0; i < FLASH_GetScheduleSize(); i++){
			event = FLASH_ReadSchedule(i);
			if(event.month == month && event.day == day){
				if((event.start&0b11111000)>>3 == hour && (event.start & 0b011)*15 == minute){
					Event_t play = (Event_t){EVENT_PLAY, (event.folder<<8) + event.track};
					MP3_Event_Post(play);
					month = NULLDATE;
					day = NULLDATE;
					hour = NULLDATE;
					minute = NULLDATE;
				}
				if((event.stop&0b11111000)>>3 == hour && (event.stop & 0b011)*15 == minute){
					Event_t play = (Event_t){EVENT_PLAY, 0};
					event.start = 0;
					MP3_Event_Post(play);
					month = NULLDATE;
					day = NULLDATE;
					hour = NULLDATE;
					minute = NULLDATE;
				}
			}
		}
	}
}
//----------------------------------------Public Functions---------------------------------------
/*
 * @Function: Scheduler_Event_Init
 * @Brief: Provides the initialization function for the events and serviced routine
 * @param: none
 * @return: An 8 bit integer flag reflecting The initialization status
 */
uint8_t Scheduler_Event_Init(FIFO Queue){
    Schedulerqueue = Queue;
    I2C_Init();
    TIMERS_Init();
    starttime = TIMERS_GetMilliSeconds();
    return INIT_OK;
}
/*
 * @Function: Scheduler_Event_Init
 * @Brief: Provides the ability for state machines to interact
 * @param: event to be posted
 * @return: none
 */
void Scheduler_Event_Post(Event_t event){
    FIFO_Enqueue(Schedulerqueue, event);
}


/*
 * @Function: Scheduler_Event_Updater
 * @Brief: Provides the event checker that checks and posts the changes in the
 * @param: none
 * @return: An event
 */
Event_t Scheduler_Event_Updater(void){
    Event_t event = (Event_t){EVENT_NONE, 0};
    uint32_t timer = TIMERS_GetMilliSeconds();
	if((timer-starttime) >= REFRESHRATE){
		event.status = EVENT_TIMEOUT;
		event.data = 0;
		starttime = timer;
		Scheduler_Event_Post(event);
	}
    return event;
}

/*
 * @Function: Scheduler_Event_Handler
 * @Brief:
 * @param: Event_t event, incoming event for the handler to handle
 * @return: An 8 byte integer success flag, returns 0 if the program should crash
 */
uint8_t Scheduler_Event_Handler(Event_t event){
	if(event.status == EVENT_PLAY){
		I2C_Recieve(RTCADDRESS, RTCMNTHADDR, 1);
		I2C_Recieve(RTCADDRESS, RTCDAYADDR, 1);
		I2C_Recieve(RTCADDRESS, RTCHOURADDR, 1);
		I2C_Recieve(RTCADDRESS, RTCMINADDR, 1);
		logging = 1;
		playdata = event.data;
	}
	if(event.status == EVENT_TIMEOUT){
		I2C_Recieve(RTCADDRESS, RTCMNTHADDR, 1);
		I2C_Recieve(RTCADDRESS, RTCDAYADDR, 1);
		I2C_Recieve(RTCADDRESS, RTCHOURADDR, 1);
		I2C_Recieve(RTCADDRESS, RTCMINADDR, 1);
	}
	if(event.status == EVENT_I2C){
		switch (event.data>>8){
		case RTCMNTHADDR:
			month = ((event.data & 0x10)>>4)*10 + (event.data & 0x0F);
			CompareTime();
			break;
		case RTCDAYADDR:
			day = ((event.data & 0x30)>>4)*10 + (event.data & 0x0F);
			CompareTime();
			break;
		case RTCHOURADDR:
			if(event.data & 0x40){//AM/PM
				hour = 12*((event.data &0x20)>>5)+ 10*((event.data &0x10)>>4) + ((event.data &0x0F));
			}else {//24Hr
				hour = 10*((event.data &0x30)>>4) + ((event.data &0x0F));
			}
			CompareTime();
			break;
		case RTCMINADDR:
			//MINTEN2 MINTEN1 MINTEN0 MINONE3 MINONE2 MINONE1 MINONE0
			minute = 10*((event.data & 0x30)>>4) +((event.data &0x0F));
			CompareTime();

			break;
		}
	}
    return 1;
}

