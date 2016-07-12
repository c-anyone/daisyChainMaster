/*
 * DaisyChain.c
 *
 *  Created on: Jul 6, 2016
 *      Author: faebsn
 */

#include <DAVE.h>
#include "DaisyChain.h"

/*
 *
 */
static uint8_t daisy_address = 0xef;
static uint8_t framebuf[64];
static uint8_t frameLength=0;

/*
 *
 */

static daisyRxCallback_t rxCallback = NULL;
static UART_CONFIG_t *daisyUart;


/*
 * 	static function definitions
 */
void handleFrameReception(void);
static void updateAddress(uint8_t newAddress);


/*
 * Set the callback handler for received data
 */
void daisySetRxCallback(daisyRxCallback_t cbPtr) {
	rxCallback = cbPtr;
}

/*
 * Initialize daisy chain handler
 */
void daisyInit(UART_CONFIG_t *uartConfig) {
	daisyUart = uartConfig;
	XMC_UART_CH_Start(daisyUart->channel);
}

/*
 * Handles the Reception of Bytes and calls the min protocol receive function
 */
void uartReceiveIRQ() {
	uint16_t rxData = 0;
	while(!XMC_USIC_CH_RXFIFO_IsEmpty(daisyUart->channel)) {
		rxData = XMC_UART_CH_GetReceivedData(daisyUart->channel);
		min_rx_byte((uint8_t) rxData & 0xff);
	}
}

/*
 * Handler for single byte transmission, could be implemented using a ringbuffer
 * and uart transmit interrupt
 */
void min_tx_byte(uint8_t byte) {
	XMC_USIC_CH_TXFIFO_PutData(daisyUart->channel,(uint16_t) byte);
}

/*
 * for now only a stub, should return the free space in the transmit buffer
 * will be used once a transmit ringbuffer is implemented
 */
uint8_t min_tx_space(void) {
	return 0xff;
}

/*
 * Handle for frame reception, calls the upper layer functions if packet
 * destination is either this devices address or a broadcast address
 */
void min_frame_received(uint8_t buf[], uint8_t len, uint8_t address) {
	frameLength = len;
	memcpy(framebuf,buf,len);		// copy the received data from the rxBuffer

	if(address != daisy_address) {
		switch(address) {
		case DAISY_ERROR:				// in case of error, retransmit to master to handle
			min_tx_frame(address,framebuf,len);
			break;
		case DAISY_BROADCAST:			//broadcast, retransmit and ignore for now
			handleFrameReception();
			min_tx_frame(address,framebuf,len);
			break;
		case DAISY_ADDR_COUNT:
			//set address to new counter
			updateAddress(++buf[0]);
			//and retransmit with increased counter value
			min_tx_frame(address,framebuf,len);
			break;
		}
	} else {
		// call data handler callback here
		if(rxCallback != NULL)
			rxCallback(len,framebuf);
	}
}

void daisySendData(uint8_t address,uint8_t length,uint8_t* data) {
	if(length > 0)
		min_tx_frame(address,data,length);
}

void handleFrameReception(void) {
	USBD_VCOM_SendData((int8_t*)framebuf,frameLength);
}

static void updateAddress(uint8_t newAddress) {
	daisy_address = newAddress;
}
