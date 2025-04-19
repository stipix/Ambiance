/*
 * Name FLASH.c
 * Brief: provides a library to initialize and perform read/write operation to the flash memory
 * Author: Caitlin Bonesio
 * Created: 4/11/25
 * Modified: 4/18/25
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

/*
 * @function: FLASH_Init()
 * @brief: initialize the flash access library
 * @param: none
 * @return: Init Status, whether the operation failed or succeeded
 */
uint8_t FLASH_Init(){
	//init flash controller?
	//find size of schedule
	for(int i = 0; i < 2048/SCHEDULEEVENTSIZE; i++){
		if ((uint8_t)(*(SCHEDULEADDRESS+(i*SCHEDULEEVENTSIZE))) == FLASHEMPTY ){
			ScheduleSize = i-1;
			break;
		}
	}
	//find size of logs
	for(int i = 0; i < 2048/SCHEDULEEVENTSIZE; i++){
		if ((uint8_t)(*(LOGSADDRESS+(i*SCHEDULEEVENTSIZE))) == FLASHEMPTY ){
			LogsSize = i-1;
			break;
		}
	}
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
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, VOLUMEADDRESS, ((uint32_t)(DC)<<8)&(uint32_t)(volume));
}

/*
 * @function: FLASH_GetVolume();
 * @brief: read the volume stored in memory
 * @param: none
 * @return: volume
 */
uint8_t FLASH_GetVolume(){
	return (uint8_t)(*(VOLUMEADDRESS));
}



/*
 * @function: FLASH_GetDutyCycle();
 * @brief: read the volume stored in memory
 * @param: none
 * @return: Duty Cycle
 */
uint8_t FLASH_GetDutyCycle(){
	return (uint8_t)(*(DCADDRESS));
}

/*
 * @function: FLASH_AppendLogs();
 * @brief: Adds a new event entry to the logs
 * @param: none
 * @return: success status
 */
uint8_t FLASH_AppendLogs(scheduleEvent event){
	LogsSize++;
	uint32_t Data1 = (event.month)&(event.day<<8)&(event.start<<16)&(event.end<<24);
	uint32_t Data2 = (event.folder)&(event.track<<8);
	//this isn't blocking code officer I swear! (this is blocking code)
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, LOGSADDRESS+LogsSize*SCHEDULEEVENTSIZE, data1);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, LOGSADDRESS+LogsSize*SCHEDULEEVENTSIZE, data2);
}

/*
 * @function: FLASH_GetLogsSize();
 * @brief: returns the size of the logs
 * @param: none
 * @return: Logs size
 */
uint16_t FLASH_GetLogsSize(){
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
	if(index >= 0 && index <= LogsSize){
		event.month = *(LOGSADDRESS+index*SCHEDULEEVENTSIZE);
		event.day = *(LOGSADDRESS+index*SCHEDULEEVENTSIZE+1);
		event.start = *(LOGSADDRESS+index*SCHEDULEEVENTSIZE+2);
		event.stop = *(LOGSADDRESS+index*SCHEDULEEVENTSIZE+3);
		event.folder = *(LOGSADDRESS+index*SCHEDULEEVENTSIZE+4);
		event.track = *(LOGSADDRESS+index*SCHEDULEEVENTSIZE+5);

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
	HAL_FLASHEx_Erase(&erase, &faultypage);
}


/*
 * @function: FLASH_AppendSchedule();
 * @brief: Adds a new event entry to the schedule
 * @param: none
 * @return: success status
 */
uint8_t FLASH_AppendSchedule(scheduleEvent event){
	LogsSize++;
	uint32_t Data1 = (event.month)&(event.day<<8)&(event.start<<16)&(event.end<<24);
	uint32_t Data2 = (event.folder)&(event.track<<8);
	//this isn't blocking code officer I swear! (this is blocking code)
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, SCHEDULEADDRESS+LogsSize*SCHEDULEEVENTSIZE, data1) == HAL_ERROR ){
		return 0;
	}
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, SCHEDULEADDRESS+LogsSize*SCHEDULEEVENTSIZE, data2) == HAL_ERROR ){
		return 0;
	}
}

/*
 * @function: FLASH_GetScheduleSize();
 * @brief: returns the size of the schedule
 * @param: none
 * @return: Schedule size
 */
uint16_t FLASH_GetScheduleSize(){
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
		if(index >= 0 && index <= LogsSize){
			event.month = *(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE);
			event.day = *(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+1);
			event.start = *(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+2);
			event.stop = *(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+3);
			event.folder = *(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+4);
			event.track = *(SCHEDULEADDRESS+index*SCHEDULEEVENTSIZE+5);

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
	HAL_FLASHEx_Erase(&erase, &faultypage);
}
