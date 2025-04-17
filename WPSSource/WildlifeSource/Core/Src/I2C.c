/*
 * Name I2C.c
 * Brief: provides a library to initialize and access the MCU's I2C hardware
 * Author: Caitlin Bonesio
 * Created: 3/4/25
 * Modified: 3/31/25
 */

//----------------------------------------Private Includes---------------------------------------
#include "I2C.h"
#include "BOARD.h"
#include "stdbool.h"
//----------------------------------------Private Defines----------------------------------------

#define CIRCBUFFERSIZE 32
#define MAXREQSIZE 2

//----------------------------------------Private Typedefs---------------------------------------
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

//----------------------------------------Private Variables--------------------------------------

I2C_HandleTypeDef hi2c1;

ReqBuffer_t RxReq;//Receive request
ReqBuffer_t TxReq;//Transmit request


//----------------------------------------Private functions--------------------------------------
/*
 * @Function: HAL_I2C_MemTxCpltCallback
 * @Brief: Overwrites a weak HAL function. Called at the end of an i2c memory transmit operation
 * @param: hi2c: I2C module that is in use
 * @return: none
 */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c){
	//here is where to implement something to do with the transmitted data

	//decrement the circular buffer
	if(TxReq.tail == TxReq.head){
		TxReq.full = false;
	}
	TxReq.head++;
	TxReq.head %= CIRCBUFFERSIZE;
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
 * @return: none
 */
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
	Event_t ret = {EVENT_I2C, data};
	//post the event to the function given
	RxReq.data[RxReq.head].PostTo(ret);
	//decrement the circular buffer
	if(RxReq.tail == RxReq.head){
		RxReq.full = false;
	}
	RxReq.head++;
	RxReq.head %= CIRCBUFFERSIZE;
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
 * @Function:I2C1_IRQHandler
 * @Brief: Overwrites a weak HAL function. handles I2C1 interrupts
 * @param: none
 * @return: none
 */
void I2C1_IRQHandler(void)
{
	if (hi2c1.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR))
	{
		HAL_I2C_ER_IRQHandler(&hi2c1);
	}
	else
	{
		HAL_I2C_EV_IRQHandler(&hi2c1);
	}
}

//----------------------------------------Public Functions---------------------------------------

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
	__HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_RXI);
	__HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_TXI);
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
HAL_StatusTypeDef I2C_Transmit(uint8_t targetadr, uint8_t registeradr, uint8_t registersize, uint8_t* data){
	HAL_StatusTypeDef status = HAL_OK;
	//will not attempt to receive if the buffer is full
	if(RxReq.full){
		return HAL_ERROR;
	}
	//will not try to receive if the register is larger than can be posted with an event
	if(registersize > MAXREQSIZE){
		return HAL_ERROR;
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
	TxReq.tail %= CIRCBUFFERSIZE;
	if(TxReq.tail == TxReq.head){
		TxReq.full = true;
	}
	if((TxReq.head+1)%CIRCBUFFERSIZE == TxReq.tail){//if the buffer was empty

		status = HAL_I2C_Mem_Write_IT(&hi2c1,
							TxReq.data[TxReq.head].targetadr<<1,
							TxReq.data[TxReq.head].registeradr,
							TxReq.data[TxReq.head].registersize,
							TxReq.data[TxReq.head].data,
							TxReq.data[TxReq.head].registersize);
		if(status != HAL_OK){
			TxReq.tail--;
			TxReq.tail %= CIRCBUFFERSIZE;
		}
	}
	return status;
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
HAL_StatusTypeDef I2C_Recieve(uint8_t targetadr, uint8_t registeradr, uint8_t registersize ,void (*PostTo)(Event_t)){
	HAL_StatusTypeDef status = HAL_OK;
	//will not attempt to receive if the buffer is full
	if(RxReq.full){
		return HAL_ERROR;
	}
	//will not try to receive if the register is larger than can be posted with an event
	if(registersize > MAXREQSIZE){
		return HAL_ERROR;
	}
	//place the receive request into the buffer
	MemReq_t Post;
	Post.targetadr = targetadr;
	Post.registeradr = registeradr;
	Post.registersize = registersize;
	Post.PostTo = PostTo;
	RxReq.data[RxReq.tail] = Post;
	RxReq.tail++;
	RxReq.tail %= CIRCBUFFERSIZE;
	if(RxReq.tail == RxReq.head){
		RxReq.full = true;
	}
	if((RxReq.head+1)%CIRCBUFFERSIZE == RxReq.tail){//if the buffer was empty
		status = HAL_I2C_Mem_Read_IT(&hi2c1,
							RxReq.data[RxReq.head].targetadr<<1,
							RxReq.data[RxReq.head].registeradr,
							RxReq.data[RxReq.head].registersize,
							RxReq.data[RxReq.head].data,
							RxReq.data[RxReq.head].registersize);
		if(status != HAL_OK){
			RxReq.tail--;
			RxReq.tail %= CIRCBUFFERSIZE;
		}
	}
	return status;

}


//----------------------------------------Private Test Harness-----------------------------------
//#define I2CTESTHARNESS

#ifdef I2CTESTHARNESS
#include "BOARD.h"
#include <stdio.h>

COM_InitTypeDef BspCOMInit;
int8_t status;

void fakeposter(Event_t event){
	printf("data received: %d", event.data);
	status = 1;
}

int main(){
	HAL_Init();
	BOARD_Init();
	BSP_LED_Init(LED_BLUE);
	I2C_Init();
	BspCOMInit.BaudRate   = 115200;
	BspCOMInit.WordLength = COM_WORDLENGTH_8B;
	BspCOMInit.StopBits   = COM_STOPBITS_1;
	BspCOMInit.Parity     = COM_PARITY_NONE;
	BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
	status = 1;
	if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
	{
		Error_Handler();
	}
	while (1){
		uint8_t address = RTCADDRESS;
		uint8_t registersize = 1;
		uint8_t registeraddress = 00;
		if(status == 1){
			status = I2C_Recieve(address, registeraddress, registersize , fakeposter);
		    BSP_LED_Toggle(LED_BLUE);
		}


		HAL_Delay(500);

	}

}
#endif
