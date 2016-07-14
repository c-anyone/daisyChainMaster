/*
 * DaisyChain.h
 *
 *  Created on: Jul 6, 2016
 *      Author: Fabio Pungg
 */

#ifndef DAISYCHAIN_H_
#define DAISYCHAIN_H_

#include "min.h"

/*
 * special addresses for different purposes
 */
#define DAISY_BROADCAST			(0xffu)
#define DAISY_ADDR_COUNT		(0x7fu)
#define DAISY_ERROR				(0x8fu)

// typedef for data receive callback function
 typedef void (*daisyRxCallback_t)(uint8_t,uint8_t,uint8_t*);

void daisyInit(UART_CONFIG_t *uartConfig);

void daisySendData(uint8_t address,uint8_t length,uint8_t* data);

void daisySetRxCallback(daisyRxCallback_t);

#endif /* DAISYCHAIN_H_ */
