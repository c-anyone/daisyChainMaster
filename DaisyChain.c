/*
 * DaisyChain.c
 *
 *  Created on: Jul 6, 2016
 *      Author: faebsn
 */

#include <DAVE.h>
#include "DaisyChain.h"


#define DAISY_BROADCAST			(0xffu)
#define DAISY_RECEIVE_ADDR		(0x7fu)

uint8_t daisy_address = 0xff;
static uint8_t framebuf[64];
static uint8_t frameLength=0;
static uint8_t target;

/*
 * Handles the Reception of Bytes and calls the min protocol receive function
 */
void uartReceiveIRQ() {
	uint16_t rxData = 0;
	rxData = XMC_UART_CH_GetReceivedData(UART_DAISY.channel);
	min_rx_byte((uint8_t) rxData & 0xff);
}

/*
 * Handler for single byte transmission, could be implemented using a ringbuffer
 * and uart transmit interrupt
 */
void min_tx_byte(uint8_t byte) {
	XMC_USIC_CH_TXFIFO_PutData(UART_DAISY.channel,(uint16_t) byte);
}

/*
 * for now only a stub, should return the free space in the transmit buffer
 */
uint8_t min_tx_space(void) {
	return 0xff;
}

/*
 * Handle for frame reception, calls the upper layer functions if packet
 * destination is either this devices address or a broadcast address
 *
 */
void min_frame_received(uint8_t buf[], uint8_t len, uint8_t address) {
	frameLength = len;
	memcpy(framebuf,buf,len);		// copy the received data from the rxBuffer
	switch(address) {

	case DAISY_BROADCAST:			//broadcast, retransmit and ignore for now
		min_tx_frame(address,framebuf,len);
		break;
	case DAISY_RECEIVE_ADDR:			//set address to new counter
		daisy_address = (buf[0])++;		//and retransmit with increased val
		min_tx_frame(address,framebuf,len);
		break;
	}
}

void DaisyChainSendData(uint8_t address,uint8_t length,uint8_t* data) {
	if(length > 0)
		min_tx_frame(address,data,length);
}

void handleFrameReception(void) {
	USBD_VCOM_SendData((int8_t*)framebuf,frameLength);
}

void updateAddress() {

}
