/*
 * Name UART.c
 * Brief: provides a library to initialize and use the low power UART (LPUART) and USART
 * Author: Caitlin Bonesio
 * Created: 2/5/25
 * Modified: 4/15/25
 */

//----------------------------------------Private Includes---------------------------------------
#include "UART.h"

#include "stm32wb05.h"
#include "stm32wb0x_ll_lpuart.h"

//----------------------------------------Private Defines----------------------------------------
#define UARTCIRCBUFFSIZE USARTBUFFERSIZE+1


//----------------------------------------Private Typedefs---------------------------------------
typedef struct UARTcb{
	uint8_t data[UARTCIRCBUFFSIZE];
	uint16_t head;
	uint16_t tail;
	bool full;
}UARTcb_t;
//----------------------------------------Private Variables--------------------------------------
UART_HandleTypeDef hlpuart1;
USART_HandleTypeDef husart1;

COM_InitTypeDef BspCOMInit;

UARTcb_t USARTtx;
UARTcb_t USARTrx;



//----------------------------------------Private Functions--------------------------------------

/**
  * @brief This function handles LPUART1 Interrupt.
  */
void LPUART1_IRQHandler(void)
{
  /* USER CODE BEGIN LPUART1_IRQn 0 */

  /* USER CODE END LPUART1_IRQn 0 */
  HAL_UART_IRQHandler(&hlpuart1);
  /* USER CODE BEGIN LPUART1_IRQn 1 */

  /* USER CODE END LPUART1_IRQn 1 */
}

void USART1_IRQHandler(void)
{

	if(husart1.Instance->ISR & USART_ISR_RXNE_RXFNE_Msk){
		if(!USARTrx.full){
			USARTrx.data[USARTrx.head] = husart1.Instance->RDR;
			USARTrx.head++;
			USARTrx.head %= UARTCIRCBUFFSIZE;
			if(USARTrx.tail == USARTrx.head){
				USARTrx.full = true;
			}
		} else {
			uint8_t discard = husart1.Instance->RDR;
			discard++;
		}
	}
	if((husart1.Instance->ISR & USART_ISR_TC_Msk)){
		if(USARTtx.head != USARTtx.tail || USARTtx.full){
			husart1.Instance->TDR = USARTtx.data[USARTtx.tail];
			USARTtx.tail++;
			USARTtx.tail %= UARTCIRCBUFFSIZE;
			if(USARTtx.full){
				USARTrx.full = false;
			}

		} else {
			__HAL_USART_CLEAR_FLAG(&husart1, UART_CLEAR_TCF);
		}
	}
	BSP_LED_Toggle(LED_RED);
	HAL_USART_IRQHandler(&husart1);

}

//----------------------------------------Public Functions---------------------------------------
/*
 * @function: UART_Init()
 * @brief: initialize the lpuart module
 * @param: none
 * @return: Init Status, whether the operation failed or succeeded
 */
int UART_Init(void){

	hlpuart1.Instance = LPUART1;
	hlpuart1.Init.BaudRate = 9600;
	hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
	hlpuart1.Init.StopBits = UART_STOPBITS_1;
	hlpuart1.Init.Parity = UART_PARITY_NONE;
	hlpuart1.Init.Mode = UART_MODE_TX_RX;
	hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	hlpuart1.FifoMode = UART_FIFOMODE_ENABLE;
	if (HAL_UART_Init(&hlpuart1) != HAL_OK)
	{
	return INIT_ERROR;
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
	  return INIT_ERROR;
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
	  return INIT_ERROR;
	}
	husart1.Instance = USART1;
//	husart1.Init.BaudRate = 9600;
//	husart1.Init.WordLength = UART_WORDLENGTH_8B;
//	husart1.Init.StopBits = UART_STOPBITS_1;
//	husart1.Init.Parity = UART_PARITY_NONE;
//	husart1.Init.Mode = UART_MODE_TX_RX;
//	husart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
//	husart1.FifoMode = UART_FIFOMODE_DISABLE;
//	if (HAL_USART_Init(&husart1) != HAL_OK)
//	{
//	return INIT_ERROR;
//	}

	BspCOMInit.BaudRate   = 9600;
	BspCOMInit.WordLength = COM_WORDLENGTH_8B;
	BspCOMInit.StopBits   = COM_STOPBITS_1;
	BspCOMInit.Parity     = COM_PARITY_NONE;
	BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
	if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
	{
		Error_Handler();
	}
	HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	husart1.Instance->CR1 |= (USART_CR1_RXNEIE_RXFNEIE_Msk | USART_CR1_TCIE_Msk);

	USARTtx.tail = 0;
	USARTtx.head = 0;
	USARTtx.full = false;

	USARTrx.tail = 0;
	USARTrx.head = 0;
	USARTrx.full = false;

//	__HAL_USART_ENABLE_IT(&husart1, UART_IT_TXE);
//	__HAL_USART_ENABLE_IT(&husart1, UART_IT_RXNE);
	return INIT_OK;
}

/*
 * @function: UART_ReadRx()
 * @brief: reads one received byte from the lpuart
 * @param: none
 * @return: the character received, is 0x00 if no character to read
 */
char LPUART_ReadRx(void){
	if(LL_LPUART_IsActiveFlag_RXNE_RXFNE(LPUART1)){
		return LL_LPUART_ReadReg(LPUART1, RDR);
	}else {
		return UARTFAILED;
	}

}

/*
 * @function: UART_WriteTx()
 * @brief: sends one byte through the lpuart
 * @param: none
 * @return: none
 */
char LPUART_WriteTx(char input){
	if (LL_LPUART_IsActiveFlag_TXE_TXFNF(LPUART1)){
		LL_LPUART_WriteReg(LPUART1, TDR, input);
		return UARTSUCCESS;
	}else {
		return UARTFAILED;
	}
}
/*
 * @function: USART_ReadRx()
 * @brief: reads one received byte from the usart
 * @param: none
 * @return: the character received, 0x25 (NAK) if no character to read
 */
char USART_ReadRx(void){
	if(USARTrx.head != USARTrx.tail || USARTrx.full){
		char data = USARTrx.data[USARTrx.tail];
		USARTrx.tail++;
		USARTrx.tail %= UARTCIRCBUFFSIZE;
		return data;
	} else {
		return UARTFAILED;
	}
}

/*
 * @function: USART_WriteTx()
 * @brief: sends one byte through the usart
 * @param: input, character to be sent
 * @return: status, 0x00 if success,  0x25 (NAK) if failed,
 */
char USART_WriteTx(char input){
	if(!USARTtx.full){
		if(USARTtx.head == USARTtx.tail && (husart1.Instance->ISR & USART_ISR_TXE_TXFNF_Msk)){
			husart1.Instance->TDR = input;
		} else {
			USARTtx.data[USARTtx.head] = input;
			USARTtx.head++;
			USARTtx.head %= UARTCIRCBUFFSIZE;
		}
		return UARTSUCCESS;
	} else {
		return UARTFAILED;
	}
}

/*
 * @function: USART_TxEmpty()
 * @brief: returns if the Tx  buffer is empty
 * @param: none
 * @return: status, 0x00 if not empty,  0x01 if empty,
 */
uint8_t USART_TxEmpty(void){
	return (USARTtx.head == USARTtx.tail) && !USARTtx.full;
}


//----------------------------------------Private Test Harness-----------------------------------
//#define UARTTESTHARNESS

#ifdef UARTTESTHARNESS
#include "BOARD.h"


int main(){
	HAL_Init();
	BOARD_Init();
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Init(LED_RED);
	UART_Init();
	while (1){
		char rx = USART_ReadRx();
		if(rx != UARTFAILED){
			USART_WriteTx(rx);
		}

	}

}
#endif




