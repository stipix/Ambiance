/*
 * Name FLASH.c
 * Brief: provides a library to initialize and perform read/write operation to the flash memory
 * Author: Caitlin Bonesio
 * Created: 4/11/25
 * Modified: 4/23/25
 */
#include "FLASH.h"
#include "BOARD.h"

#define VOLUMEADDRESS  	0x1006F800
#define DCADDRESS 		0x1006F801
#define SCHEDULEADDRESS	0x1006F000
#define LOGSADDRESS 	0x1006E800

#define DCVOLPAGE		96
#define SCHEDULEPAGE	95
#define LOGSPAGE		94

#define FLASHEMPTY 0x1111

#define SCHEDULEEVENTSIZE 8//must be a multiple of 4


static uint16_t ScheduleSize;
static uint16_t LogsSize;
static uint8_t initialized = 0;

/*
 * @function: FLASH_Init()
 * @brief: initialize the flash access library
 * @param: none
 * @return: Init Status, whether the operation failed or succeeded
 */
uint8_t FLASH_Init(){
	if(initialized){return INIT_OK;}
	//find size of schedule
	for(int i = 0; i < 2048/SCHEDULEEVENTSIZE; i++){
		if ((uint8_t)(*(uint8_t*)(SCHEDULEADDRESS+(i*SCHEDULEEVENTSIZE))) == FLASHEMPTY ){
			ScheduleSize = i-1;
			break;
		}
	}
	//find size of logs
	for(int i = 0; i < 2048/SCHEDULEEVENTSIZE; i++){
		if ((uint8_t)(*(uint8_t*)(LOGSADDRESS+(i*SCHEDULEEVENTSIZE))) == FLASHEMPTY ){
			LogsSize = i-1;
			break;
		}
	}
	initialized = 1;
	//load default values into the duty cycle and volume if they are not initialized
	if(FLASH_GetVolume() == FLASHEMPTY ||FLASH_GetDutyCycle() == FLASHEMPTY){
		if(FLASH_GetVolume() == FLASHEMPTY){
			if(FLASH_GetDutyCycle() == FLASHEMPTY){
				FLASH_SetDCVol(50, 40);
			} else {
				FLASH_SetDCVol(50, FLASH_GetDutyCycle());
			}
		} else {
			FLASH_SetDCVol(FLASH_GetVolume(), 40);
		}
	}
	return INIT_OK;
}

/*
 * @function: FLASH_SetVolume();
 * @brief: edits the volume and duty cycle stored in memory
 * @param: none
 * @return: success status
 */
uint8_t FLASH_SetDCVol(uint8_t volume, uint8_t DC){
	FLASH_EraseInitTypeDef erase;
	erase.Page = DCVOLPAGE;
	erase.NbPages = 1;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	uint32_t faultypage;
	HAL_FLASHEx_Erase(&erase, &faultypage);
	HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, VOLUMEADDRESS, ((uint32_t)(DC)<<8)&(uint32_t)(volume));
	return status == HAL_OK;
}

/*
 * @function: FLASH_GetVolume();
 * @brief: read the volume stored in memory
 * @param: none
 * @return: volume
 */
uint8_t FLASH_GetVolume(){
	if(!initialized){return 0;}
	return (uint8_t)(*((uint8_t*)VOLUMEADDRESS));
}



/*
 * @function: FLASH_GetDutyCycle();
 * @brief: read the volume stored in memory
 * @param: none
 * @return: Duty Cycle
 */
uint8_t FLASH_GetDutyCycle(){
	if(!initialized){return 0;}
	return (uint8_t)(*((uint8_t*)DCADDRESS));
}

/*
 * @function: FLASH_AppendLogs();
 * @brief: Adds a new event entry to the logs
 * @param: none
 * @return: success status
 */
uint8_t FLASH_AppendLogs(scheduleEvent event){
	if(!initialized){return 0;}
	LogsSize++;
	uint32_t Data1 = (event.month)&(event.day<<8)&(event.start<<16)&(event.stop<<24);
	uint32_t Data2 = (event.folder)&(event.track<<8);
	//this isn't blocking code officer I swear! (this is blocking code)
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, LOGSADDRESS+LogsSize*SCHEDULEEVENTSIZE, Data1) != HAL_OK ){
		return 0;
	}
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, LOGSADDRESS+LogsSize*SCHEDULEEVENTSIZE, Data2) != HAL_OK ){
		return 0;
	}
	return 1;
}

/*
 * @function: FLASH_GetLogsSize();
 * @brief: returns the size of the logs
 * @param: none
 * @return: Logs size
 */
uint16_t FLASH_GetLogsSize(){
	if(!initialized){return 0;}
	return LogsSize;
}

/*
 * @function: FLASH_ReadLogs();
 * @brief: allows reading the logs that have been stored in flash memory
 * @param: index to read at
 * @return: an schedule event struct at index, null if invalid index
 */
scheduleEvent FLASH_ReadLogs(uint16_t index){
	scheduleEvent event = (scheduleEvent){0, 0, 0, 0, 0, 0};
	if(!initialized){return event;}
	if(index >= 0 && index <= LogsSize){
		event.month = *(uint8_t*)(LOGSADDRESS+index*SCHEDULEEVENTSIZE);
		event.day = *(uint8_t*)(LOGSADDRESS+index*SCHEDULEEVENTSIZE+1);
		event.start = *(uint8_t*)(LOGSADDRESS+index*SCHEDULEEVENTSIZE+2);
		event.stop = *(uint8_t*)(LOGSADDRESS+index*SCHEDULEEVENTSIZE+3);
		event.folder = *(uint8_t*)(LOGSADDRESS+index*SCHEDULEEVENTSIZE+4);
		event.track = *(uint8_t*)(LOGSADDRESS+index*SCHEDULEEVENTSIZE+5);

	}
	return event;
}

/*
 * @function: FLASH_ClearLogs();
 * @brief: Clears all log entries
 * @param: none
 * @return: success status
 */
uint8_t FLASH_ClearLogs(){
	FLASH_EraseInitTypeDef erase;
	erase.Page = LOGSPAGE;
	erase.NbPages = 1;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	uint32_t faultypage;
	HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &faultypage);
	return status == HAL_OK;

}


/*
 * @function: FLASH_AppendSchedule();
 * @brief: Adds a new event entry to the schedule
 * @param: none
 * @return: success status
 */
uint8_t FLASH_AppendSchedule(scheduleEvent event){
	if(!initialized){return 0;}
	LogsSize++;
	uint32_t Data1 = (event.month)&(event.day<<8)&(event.start<<16)&(event.stop<<24);
	uint32_t Data2 = (event.folder)&(event.track<<8);
	//this isn't blocking code officer I swear! (this is blocking code)
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, SCHEDULEADDRESS+LogsSize*SCHEDULEEVENTSIZE, Data1) != HAL_OK ){
		return 0;
	}
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, SCHEDULEADDRESS+LogsSize*SCHEDULEEVENTSIZE, Data2) != HAL_OK ){
		return 0;
	}
	return 1;
}

/*
 * @function: FLASH_GetScheduleSize();
 * @brief: returns the size of the schedule
 * @param: none
 * @return: Schedule size
 */
uint16_t FLASH_GetScheduleSize(){
	if(!initialized){return 0;}
	return ScheduleSize;
}

/*
 * @function: FLASH_ReadSchedule();
 * @brief: allows reading the schedule that have been stored in flash memory
 * @param: index to read at
 * @return: an schedule event struct at index
 */
scheduleEvent FLASH_ReadSchedule(uint16_t index){
	scheduleEvent event = (scheduleEvent){0, 0, 0, 0, 0, 0};
	if(!initialized){return event;}
		if(index >= 0 && index <= LogsSize){
			event.month = *(uint8_t*)(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE);
			event.day = *(uint8_t*)(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+1);
			event.start = *(uint8_t*)(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+2);
			event.stop = *(uint8_t*)(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+3);
			event.folder = *(uint8_t*)(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+4);
			event.track = *(uint8_t*)(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+5);

		}
		return event;
}

/*
 * @function: FLASH_ClearSchedule();
 * @brief: Clears all Schedule entries
 * @param: none
 * @return: success status
 */
uint8_t FLASH_ClearSchedule(){
	FLASH_EraseInitTypeDef erase;
	erase.Page = SCHEDULEPAGE;
	erase.NbPages = 1;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	uint32_t faultypage;
	HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &faultypage);
	return status == HAL_OK;
}
