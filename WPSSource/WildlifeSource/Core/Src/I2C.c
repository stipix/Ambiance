/*
 * Name I2C.c
 * Brief: provides a library to initialize and access the MCU's I2C hardware
 * Author: Caitlin Bonesio
 * Created: 3/4/25
 * Modified: 3/31/25
 */

#include "I2C.h"
#include "BOARD.h"
#include "stdbool.h"

#define CIRCBUFFERSIZE 32
#define MAXREQSIZE 2

//typedef struct circBufffer{
//	uint8_t data[CIRCBUFFERSIZE];
//	uint8_t* head = data;
//	uint8_t* tail = data;
//	bool full = false;
//}circBuffer_t;

typedef struct MemReq{
	uint8_t targetadr;
	uint8_t registeradr;
	uint8_t registersize;
	void (*PostTo)(Event_t);
	uint8_t data[MAXREQSIZE];
}MemReq_t;

typedef struct ReqBuffer{
	MemReq_t data[CIRCBUFFERSIZE];
	uint8_t head;
	uint8_t tail;
	bool full;
}ReqBuffer_t;


I2C_HandleTypeDef hi2c1;

ReqBuffer_t RxReq;//Receive request
ReqBuffer_t TxReq;//Transmit request
//circBuffer_t Tx;
//circBuffer_t Rx;


/*
 * @Function: HAL_I2C_MemRxCpltCallback
 * @Brief: Overwrites a weak HAL function. Called at the end of an i2c memory receive operation
 * @param: hi2c: I2C module that is in use
 * @return: none */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c){
	//here is where to implement something to do with the transmitted data

	//decrement the circular buffer
	if(TxReq.tail == TxReq.head){
		TxReq.full = false;
	}
	TxReq.head--;
	if(RxReq.head != RxReq.tail){//if the buffer is not empty (cannot be full as we just pulled from the buffer)
			HAL_I2C_Mem_Write_IT(&hi2c1,
								 TxReq.data[TxReq.head].targetadr<<1,
								 TxReq.data[TxReq.head].registeradr,
								 TxReq.data[TxReq.head].registersize,
								 TxReq.data[TxReq.head].data,
								 TxReq.data[TxReq.head].registersize);
		}
}


/*
 * @Function: HAL_I2C_MemRxCpltCallback
 * @Brief: Overwrites a weak HAL function. Called at the end of an i2c memory receive operation
 * @param: hi2c: I2C module that is in use
 * @return: none */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c){
	uint16_t data;
	//construct the received data into a uint16_t
	if(RxReq.data[RxReq.head].registersize == 2){
		data = (RxReq.data[RxReq.head].data[0] << 8) + (RxReq.data[RxReq.head].data[1]);
	}else if(RxReq.data[RxReq.head].registersize == 1){
		data = RxReq.data[RxReq.head].data[0];
	}else {//error recovery
		data = 0;
	}
	//use the received data construct and event to post
	Event_t ret = {data, EVENT_I2C};
	//post the event to the function given
	RxReq.data[RxReq.head].PostTo(ret);
	//decrement the circular buffer
	if(RxReq.tail == RxReq.head){
		RxReq.full = false;
	}
	RxReq.head--;
	if(RxReq.head != RxReq.tail){//if the buffer is not empty (cannot be full as we just pulled from the buffer)

		HAL_I2C_Mem_Read_IT(&hi2c1,
							RxReq.data[RxReq.head].targetadr<<1,
							RxReq.data[RxReq.head].registeradr,
							RxReq.data[RxReq.head].registersize,
							RxReq.data[RxReq.head].data,
							RxReq.data[RxReq.head].registersize);
	}
}

/*
 * @function: I2C_Init()
 * @brief: initialize the I2C peripheral to receive and transmit
 * @param: none
 * @return: Init Status, whether the operation failed or succeeded
 */
int I2C_Init(void){
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00303D5B;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		return INIT_ERROR;
	}

	// Configure Analog filter
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		return INIT_ERROR;
	}

	// Configure Digital filter
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		return INIT_ERROR;
	}
	//set up circular buffers
	RxReq.full = false;
	RxReq.tail = 0;
	RxReq.head = 0;

	TxReq.full = false;
	TxReq.tail = 0;
	TxReq.head = 0;
	//enables interrupts
	//hi2c1.Instance->CR1 = hi2c1.Instance->CR1 | I2C_CR1_RXIE_Msk | I2C_CR1_TXIE_Msk;
	return INIT_OK;
}

/*
 * @Function: I2C_Transmit
 * @Brief: Gives access to transmit a byte
 * @param: uint8_t targetadr: Address of the device to communicate
 * 		   uint8_t registeradr: Address of the register to write to
 * 		   uint8_t data: to to be transmitted
 * @return: a flag if the data has been transmitted
 */
uint8_t I2C_Transmit(uint8_t targetadr, uint8_t registeradr, uint8_t registersize, uint8_t* data){
	//will not attempt to receive if the buffer is full
		if(RxReq.full){
			return -1;
		}
		//will not try to receive if the register is larger than can be posted with an event
		if(registersize > MAXREQSIZE){
			return -1;
		}
		//place the transmit request into the buffer
		MemReq_t Post;
		Post.targetadr = targetadr;
		Post.registeradr = registeradr;
		Post.registersize = registersize;
		Post.PostTo = 0;//null pointer, no function is posted to after the transmit is complete
		for(int i = 0; i < registersize; i++){
			Post.data[i] = *(data+i);//make use of pointer arithmetic where data + i = data + sizeof(uint8_t)*i
		}
		TxReq.data[TxReq.tail] = Post;
		TxReq.tail++;
		if(TxReq.tail == TxReq.head){
			TxReq.full = true;
		}
		if(TxReq.head+1 == TxReq.tail){//if the buffer was empty

			HAL_I2C_Mem_Write_IT(&hi2c1,
								TxReq.data[TxReq.head].targetadr<<1,
								TxReq.data[TxReq.head].registeradr,
								TxReq.data[TxReq.head].registersize,
								TxReq.data[TxReq.head].data,
								TxReq.data[TxReq.head].registersize);
		}
		return 1;
	return 0;
}

/*
 * @Function: I2C_Recieve
 * @Brief: transmits requesting data. when the data is received it posts an event with the data to the PostTo function
 * @param: uint8_t targetadr: Address of the device to communicate
 * 		   uint8_t registeradr: Address of the register to read from
 * 		   uint8_t registersize: size of address of the register to read from, keep 2 bytes and under
 * 		   void (*PostTo)(Event_t): which service to post to when the data is received
 * @return: -1 if error, 1 if success
 */
int8_t I2C_Recieve(uint8_t targetadr, uint8_t registeradr, uint8_t registersize ,void (*PostTo)(Event_t)){
	//will not attempt to receive if the buffer is full
	if(RxReq.full){
		return -1;
	}
	//will not try to receive if the register is larger than can be posted with an event
	if(registersize > MAXREQSIZE){
		return -1;
	}
	//place the receive request into the buffer
	MemReq_t Post;
	Post.targetadr = targetadr;
	Post.registeradr = registeradr;
	Post.registersize = registersize;
	Post.PostTo = PostTo;
	RxReq.data[RxReq.tail] = Post;
	RxReq.tail++;
	if(RxReq.tail == RxReq.head){
		RxReq.full = true;
	}
	if(RxReq.head+1 == RxReq.tail){//if the buffer was empty

		HAL_I2C_Mem_Read_IT(&hi2c1,
							RxReq.data[RxReq.head].targetadr<<1,
							RxReq.data[RxReq.head].registeradr,
							RxReq.data[RxReq.head].registersize,
							RxReq.data[RxReq.head].data,
							RxReq.data[RxReq.head].registersize);
	}
	return 1;

}
void I2C1_IRQHandler(void)
{
	//possibly unused
//	if(hi2c1.Instance->ISR & I2C_FLAG_RXNE){
//		if(!Rx.full){
//			Rx.data[Rx.tail] = hi2c1.Instance->RXDR;
//			Rx.tail++;
//			if(Rx.tail == Rx.head){
//				Rx.full = true;
//			}
//		}
//		hi2c1.Instance->ISR = hi2c1.Instance->ISR & !(uint32_t)I2C_FLAG_RXNE;
//	}
//	if(hi2c1.Instance->ISR & I2C_FLAG_TXE){
//		if(Tx.full||(Tx.head != Tx.tail)){//if the transmit buffer is not empty
//			hi2c1.Instance->TXDR = Tx.data[Tx.tail];
//			if(Tx.tail == Tx.head){
//				Tx.full = false;
//			}
//			Tx.tail--;
//
//		}
//		hi2c1.Instance->ISR = hi2c1.Instance->ISR & !(uint32_t)I2C_FLAG_TXE;
//	}
	if (hi2c1.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR))
	{
		HAL_I2C_ER_IRQHandler(&hi2c1);
	}
	else
	{
		HAL_I2C_EV_IRQHandler(&hi2c1);
	}
}

/*
 * @Function: I2C_Event_Init
 * @Brief: Provides the initialization function for the events and serviced routine
 * @param: none
 * @return: An 8 bit integer flag reflecting the initialization status
 */
uint8_t I2C_Event_Init(void){
	return I2C_Init();
}

/*
 * @Function: I2C_Event_Updater
 * @Brief: Provides the event checker that checks and posts the changes in the I2C
 * @param: none
 * @return: An event with 8 bits of recieved data
 */
Event_t I2C_Event_Updater(void){
	Event_t update = {0, 0};
	return update;
}

/*
 * @Function: I2C_Event_Handler
 * @Brief: stores recieved bytes for later recall
 * @param: Event_t event, incoming event for the handler to handle
 * @return: An 8 byte integer success flag, returns 0 if the program should crash
 */
uint8_t I2C_Event_Handler(Event_t event){
	return 1;
}

