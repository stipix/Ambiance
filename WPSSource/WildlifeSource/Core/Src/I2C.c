/*
 * Name I2C.c
 * Brief: provides a library to initialize and access the MCU's I2C hardware
 * Author: Caitlin Bonesio
 * Created: 3/4/25
 * Modified: 4/23/25
 */

//----------------------------------------Private Includes---------------------------------------
#include "I2C.h"
#include "BOARD.h"
#include "stdbool.h"
#include "I2C-UART_Manager.h"
#include "TIMERS.h"
//----------------------------------------Private Defines----------------------------------------

#define TXCIRCBUFFERSIZE 2048//one full OLED print, plus extra
#define RXCIRCBUFFERSIZE 16
#define MAXREQSIZE 1

#define I2CBUFFERTIMEOUT 1000 // number of millisecond I2C transmit will wait for a spot in the buffer to appear
//----------------------------------------Private Typedefs---------------------------------------
typedef struct MemReq{
	uint8_t targetadr;
	uint8_t registeradr;
	void (*PostTo)(Event_t);
	uint8_t data;
}MemReq_t;
typedef struct MemWrite{
	uint8_t targetadr;
	uint8_t registeradr;
	uint8_t data;
}MemWrite_t;

typedef struct TxReqBuffer{
	MemWrite_t data[TXCIRCBUFFERSIZE];
	uint16_t head;
	uint16_t tail;
	bool full;
}TxReqBuffer_t;

typedef struct RxReqBuffer{
	MemReq_t data[RXCIRCBUFFERSIZE];
	uint16_t head;
	uint16_t tail;
	bool full;
}RxReqBuffer_t;
//----------------------------------------Private Variables--------------------------------------

I2C_HandleTypeDef hi2c1;

static RxReqBuffer_t RxReq;//Receive request
static TxReqBuffer_t TxReq;//Transmit request

static uint8_t initialized = 0;
static uint16_t finalcount = 0;


//----------------------------------------Private functions--------------------------------------
/*
 * @Function: HAL_I2C_MemTxCpltCallback
 * @Brief: Overwrites a weak HAL function. Called at the end of an i2c memory transmit operation
 * @param: hi2c: I2C module that is in use
 * @return: none
 */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c){
	//here is where to implement something to do with the transmitted data

//	BSP_LED_Toggle(LED_RED);
	//if there is more in the buffer
	if(TxReq.tail != TxReq.head){
		I2CUARTtoI2C(1);
		HAL_I2C_Mem_Write_IT(&hi2c1,
							 TxReq.data[TxReq.tail].targetadr<<1,
							 TxReq.data[TxReq.tail].registeradr,
							 I2C_MEMADD_SIZE_8BIT,
							 &(TxReq.data[TxReq.tail].data),
							 1);
		TxReq.tail++;
		TxReq.tail %= TXCIRCBUFFERSIZE;
//		if(TxReq.full){
//			TxReq.full = false;
//		}
	}
}


/*
 * @Function: HAL_I2C_MemRxCpltCallback
 * @Brief: Overwrites a weak HAL function. Called at the end of an i2c memory receive operation
 * @param: hi2c: I2C module that is in use
 * @return: none
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c){

	//use the received data construct and event to post
	Event_t ret = {EVENT_I2C, RxReq.data[RxReq.tail].registeradr << 8 &RxReq.data[RxReq.tail].data};
	//post the event to the function given
	RxReq.data[RxReq.tail].PostTo(ret);
	//decrement the circular buffer
	if(RxReq.tail == RxReq.head){
		RxReq.full = false;
	}
	RxReq.tail++;
	RxReq.tail %= RXCIRCBUFFERSIZE;
	if(RxReq.head != RxReq.tail){//if the buffer is not empty (cannot be full as we just pulled from the buffer)
		I2CUARTtoI2C(1);
		HAL_I2C_Mem_Read_IT(&hi2c1,
							RxReq.data[RxReq.head].targetadr<<1,
							RxReq.data[RxReq.head].registeradr,
							I2C_MEMADD_SIZE_8BIT,
							&(RxReq.data[RxReq.head].data),
							1);
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
		HAL_I2C_ER_IRQHandler(&hi2c1);//error handler
	}
	else
	{
		HAL_I2C_EV_IRQHandler(&hi2c1);//event handler
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
	if(initialized){return INIT_OK;}
	TIMERS_Init();
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00303D5B;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_StatusTypeDef ret = HAL_I2C_Init(&hi2c1);
	if (ret != HAL_OK)
	{
		return INIT_ERROR;
	}
	// Configure Analog filter
	ret = HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);
	if (ret != HAL_OK)
	{
		return INIT_ERROR;
	}

	// Configure Digital filter
	ret = HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);
	if (ret != HAL_OK)
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
	initialized = 1;
	I2CUARTtoI2C(1);
	//enables interrupts
	//hi2c1.Instance->CR1 = hi2c1.Instance->CR1 | I2C_CR1_RXIE_Msk | I2C_CR1_TXIE_Msk;
	HAL_NVIC_SetPriority(I2C1_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(I2C1_IRQn);
//	__HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_RXI);
//	__HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_TXI);
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
HAL_StatusTypeDef I2C_Transmit(uint8_t targetadr, uint8_t registeradr, uint8_t data){
	if(!initialized){return HAL_ERROR;}
	HAL_StatusTypeDef status = HAL_OK;
	//will not attempt to receive if the buffer is full
	static uint16_t count =0;
	if(finalcount == 0){
		count++;
	}
	if(TxReq.tail == (TxReq.head+1)%TXCIRCBUFFERSIZE){
		finalcount = count;
		BSP_LED_On(LED_RED);
	}
	uint32_t start = TIMERS_GetMilliSeconds();
	while(TxReq.tail == (TxReq.head+1)%TXCIRCBUFFERSIZE && ((start + I2CBUFFERTIMEOUT) > TIMERS_GetMilliSeconds()));
	if(TxReq.tail == (TxReq.head+1)%TXCIRCBUFFERSIZE){
		return HAL_ERROR;
		BSP_LED_On(LED_RED);
	}

	//place the transmit request into the buffer
	static MemWrite_t Post;
	Post.targetadr = targetadr;
	Post.registeradr = registeradr;
//	Post.PostTo = 0;//null pointer, no function is posted to after the transmit is complete
	Post.data = data;

	if(TxReq.tail == TxReq.head && hi2c1.State == HAL_I2C_STATE_READY){
		I2CUARTtoI2C(1);
		status = HAL_I2C_Mem_Write_IT(&hi2c1,
									 Post.targetadr<<1,
									 Post.registeradr,
									 I2C_MEMADD_SIZE_8BIT,
									 &(Post.data),
									 1);
		if(status == HAL_BUSY){
			TxReq.data[TxReq.head] = Post;
			TxReq.head++;
			TxReq.head %= TXCIRCBUFFERSIZE;
		}
	} else {
		TxReq.data[TxReq.head] = Post;
		TxReq.head++;
		TxReq.head %= TXCIRCBUFFERSIZE;
//		if(TxReq.tail == TxReq.head){
//			TxReq.full = true;
//		}
	}
	return status;
}

/*
 * @Function: I2C_Recieve
 * @Brief: transmits requesting data. when the data is received it posts an event with the data to the PostTo function
 * @param: uint8_t targetadr: Address of the device to communicate
 * 		   uint8_t registeradr: Address of the register to read from
 * 		   void (*PostTo)(Event_t): which service to post to when the data is received
 * @return: -1 if error, 1 if success
 */
HAL_StatusTypeDef I2C_Recieve(uint8_t targetadr, uint8_t registeradr,void (*PostTo)(Event_t)){
	if(!initialized){return HAL_ERROR;}
	HAL_StatusTypeDef status = HAL_OK;
	//will not attempt to receive if the buffer is full
	if(RxReq.full){
		return HAL_ERROR;
	}

	//place the receive request into the buffer
	static MemReq_t Post;
	Post.targetadr = targetadr;
	Post.registeradr = registeradr;
	Post.PostTo = PostTo;
	uint8_t first = TxReq.tail == TxReq.head;
	RxReq.data[RxReq.head] = Post;
	RxReq.head++;
	RxReq.head %= RXCIRCBUFFERSIZE;
	if(RxReq.tail == RxReq.head){
		RxReq.full = true;
	}
	if(first){//if the buffer was empty
		I2CUARTtoI2C(1);
		status = HAL_I2C_Mem_Read_IT(&hi2c1,
							RxReq.data[RxReq.head].targetadr<<1,
							RxReq.data[RxReq.head].registeradr,
							I2C_MEMADD_SIZE_8BIT,
							&(RxReq.data[RxReq.head].data),
							1);
		if(status != HAL_OK){
			RxReq.head--;
			RxReq.head %= RXCIRCBUFFERSIZE;
		}
	}
	return status;

}

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
)
{
    HAL_StatusTypeDef ret;
//    I2CAddress = I2CAddress << 1; // Use 8-bit address.
//
//    ret = HAL_I2C_Mem_Write(
//        &hi2c1,
//        I2CAddress,
//        deviceRegisterAddress,
//        I2C_MEMADD_SIZE_8BIT,
//        &data,
//        1,
//        HAL_MAX_DELAY
//    );
    ret = I2C_Transmit(I2CAddress, deviceRegisterAddress, data);
    if (ret != HAL_OK)
    {
        return ERROR;
    }

    return SUCCESS;
}


//----------------------------------------Private Test Harness-----------------------------------
//#define I2CTESTHARNESS

#ifdef I2CTESTHARNESS
#include "BOARD.h"
#include "Oled.h"
#include "DiscountIO.h"
#include "UART.h"

HAL_StatusTypeDef status;
uint8_t month;
uint8_t day;
uint8_t hour;
uint8_t minute

void fakeposter(Event_t event){
	if(event.status == EVENT_I2C){
		switch (event.data>>8){
		case RTCMNTHADDR:
			month = ((event.data & 0x10)>>4)*10 + (event.data & 0x0F);
			break;
		case RTCDAYADDR:
			day = ((event.data & 0x30)>>4)*10 + (event.data & 0x0F);
			break;
		case RTCHOURADDR:
			if(event.data & 0x40){//AM/PM
				hour = 12*((event.data &0x20)>>5)+ 10*((event.data &0x10)>>4) + ((event.data &0x0F));
			}else {//24Hr
				hour = 10*((event.data &0x30)>>4) + ((event.data &0x0F));
			}
			break;
		case RTCMINADDR:
			//MINTEN2 MINTEN1 MINTEN0 MINONE3 MINONE2 MINONE1 MINONE0
			minute = 10*((event.data & 0x30)>>4) +((event.data &0x0F));

			break;
		}
	}
	return;
}

int main(){

	BOARD_Init();
	TIMERS_Init();
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Init(LED_RED);
	HAL_Delay(100);
	OledInit();
	HAL_Delay(1000);
	while (1){
		char text[88];
		I2C_Recieve(RTCADDRESS, RTCMNTHADDR, fakeposter);
		I2C_Recieve(RTCADDRESS, RTCDAYADDR, fakeposter);
		I2C_Recieve(RTCADDRESS, RTCHOURADDR, fakeposter);
		I2C_Recieve(RTCADDRESS, RTCMINADDR, fakeposter);
		sprintf(text, "current time: %d/%d %d:%d", month, day, hour, minute);
		OledDrawString(text);
		OledUpdate();

		HAL_Delay(1000);

	}

}
#endif
