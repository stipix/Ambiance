/*
 * Name I2C.h
 * Brief: provides a library to initialize and recieve and transmit using the I2C.
 * 		  Also provides the functions to implement the I2C into the events and services main
 * Author: Caitlin Bonesio
 * Created: 3/4/25
 * Modified: 3/7/25
 */


#ifndef I2C_H
#define I2C_H


#ifdef __cplusplus//allows code to be ran in a c++ program
extern "C" {
#endif


//----------------------------------------Public Includes----------------------------------------
#include "CONFIG.h"

//----------------------------------------Public Functions---------------------------------------

/*
 * @function: I2C_Init()
 * @brief: initialize the I2C peripheral to receive and transmit
 * @param: none
 * @return: Init Status, whether the operation failed or succeeded
 */
int I2C_Init(void);


/*
 * @Function: I2C_Transmit
 * @Brief: Gives access to transmit a byte
 * @param: uint8_t targetadr: Address of the device to communicate
 * 		   uint8_t registeradr: Address of the register to write to
 * 		   uint8_t data: to to be transmitted
 * @return: a flag if the data has been transmitted
 */
uint8_t I2C_Transmit(uint8_t targetadr, uint8_t registeradr, uint8_t registersize, uint8_t* data);


/*
 * @Function: I2C_Recieve
 * @Brief: transmits requesting data. when the data is received it posts an event with the data to the PostTo function
 * @param: uint8_t targetadr: Address of the device to communicate
 * 		   uint8_t registeradr: Address of the register to read from
 * 		   uint8_t registersize: size of address of the register to read from, keep 2 bytes and under
 * 		   void (*PostTo)(Event_t): which service to post to when the data is received
 * @return: -1 if error, 1 if success
 */
int8_t I2C_Recieve(uint8_t targetadr, uint8_t registeradr, uint8_t registersize ,void (*PostTo)(Event_t));

/*
 * @Function: I2C_Event_Init
 * @Brief: Provides the initialization function for the events and serviced routine
 * @param: none
 * @return: An 8 bit integer flag reflecting the initialization status
 */
uint8_t I2C_Event_Init(void);

/*
 * @Function: I2C_Event_Updater
 * @Brief: Provides the event checker that checks and posts the changes in the I2C
 * @param: none
 * @return: An event with 8 bits of recieved data
 */
Event_t I2C_Event_Updater(void);

/*
 * @Function: I2C_Event_Handler
 * @Brief: stores recieved bytes for later recall
 * @param: Event_t event, incoming event for the handler to handle
 * @return: An 8 byte integer success flag, returns 0 if the program should crash
 */
uint8_t I2C_Event_Handler(Event_t event);



#ifdef __cplusplus//allows code to be ran in a c++ program
}
#endif


#endif /* INC_I2C_H_ */
