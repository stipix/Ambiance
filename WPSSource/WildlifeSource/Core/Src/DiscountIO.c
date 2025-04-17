/*
 * DiscountIO.c
 *
 *  Created on: Apr 17, 2025
 *      Author: stipi
 */
#include <stdio.h>
#include <string.h>
#include "UART.h"
#include "DiscountIO.h"

int	discountprintf (const char * text){
	int length = strlen(text);
	for(int i = 0; i < length; i++){
		USART_WriteTx(text[i]);
	}
	return 1;
}


