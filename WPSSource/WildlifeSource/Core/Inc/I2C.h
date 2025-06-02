/*
 * Name I2C.h
 * Brief: provides a library to initialize and recieve and transmit using the I2C.
 * 		  Also provides the functions to implement the I2C into the events and services main
 * Author: Caitlin Bonesio
 * Created: 3/4/25
 * Modified: 4/21/25
 */


#ifndef I2C_H
#define I2C_H


#ifdef __cplusplus//allows code to be ran in a c++ program
extern "C" {
#endif


//----------------------------------------Public Includes----------------------------------------
#include "CONFIG.h"


//----------------------------------------Public Defines-----------------------------------------
#define SCREENADDRESS   0b00111100
#define RTCADDRESS      0b01101111
#define RTCYEARADDR     0x05
#define RTCMNTHADDR     0x04
#define RTCDAYADDR      0x03
#define RTCHOURADDR     0x02
#define RTCMINADDR      0x01
#define RTCSECADDR 		0x00
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
HAL_StatusTypeDef I2C_Transmit(uint8_t targetadr, uint8_t registeradr, uint8_t data);


/*
 * @Function: I2C_Recieve
 * @Brief: transmits requesting data. when the data is received it posts an event with the data to the PostTo function
 * @param: uint8_t targetadr: Address of the device to communicate
 * 		   uint8_t registeradr: Address of the register to read from
 * 		   void (*PostTo)(Event_t): which service to post to when the data is received
 * @return: -1 if error, 1 if success
 */
HAL_StatusTypeDef I2C_Recieve(uint8_t targetadr, uint8_t registeradr, void (*PostTo)(Event_t));


void I2C_USARTtoI2C();

/** I2C_WriteReg(I2CAddress, deviceRegisterAddress, data)
 *
 * Writes one device register on chosen I2C device.
 *
 * @param   I2CAddress              (unsigned char) 7-bit address of I2C device
 *                                                  wished to interact with.
 * @param   deviceRegisterAddress   (unsigned char) 8-bit address of register on
 *                                                  device.
 * @param   data                    (uint8_t)       Data wished to be written to
 *                                                  register.
 * @return                          (unsigned char) [SUCCESS, ERROR]
 */
unsigned char I2C_WriteReg(
    unsigned char I2CAddress,
    unsigned char deviceRegisterAddress,
    uint8_t data
);


#ifdef __cplusplus//allows code to be ran in a c++ program
}
#endif


#endif /* INC_I2C_H_ */
