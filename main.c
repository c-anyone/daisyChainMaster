/*
 * main.c
 *
 *  Created on: 2016 Jun 29 18:28:54
 *  Author: Fabio Pungg
 */

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include "./xmc_daisyChain/DaisyChain.h"
#include "led_commands.h"

#define BUF_SIZE	(64u)
/**

 * @brief main() - Application entry point
 *
 * <b>Details of function</b><br>
 * This routine is the application entry point. It is invoked by the device startup code.It is
 * responsible for invoking the APP initialization dispatcher routine - DAVE_Init() and hosting
 * the place-holder for user application
 * code.
 */

uint8_t RxBuffer[BUF_SIZE] = { 0 };
uint8_t TxBuffer[BUF_SIZE] = { 0 };
uint8_t Bytes;

static void usbCallback(void);
static inline void parseAndAct(size_t bytes);

int main(void) {
	DAVE_STATUS_t status;

	status = DAVE_Init();
	if (status == DAVE_STATUS_FAILURE) {
		XMC_DEBUG(("DAVE Apps initialization failed with status %d\n", status));
		while (1U) {
			// Error handling code here
		}
	}

	if (USBD_VCOM_Connect() != USBD_VCOM_STATUS_SUCCESS) {
		return -1;
	}
	while (!USBD_VCOM_IsEnumDone())
		;

	daisyInit(&UART_DAISY);

	while (1U) {

		usbCallback();

		daisyWorker();

		CDC_Device_USBTask(&USBD_VCOM_cdc_interface);
	}
	return 1;
}

static inline int printPWMSettings(char *mesBuf, int free, uint8_t *buf,
		size_t length) {
	PWM_SETTINGS_t *ptr;
//	if (length != sizeof(PWM_SETTINGS_t) || buf == NULL)
//		return 0;
	ptr = (PWM_SETTINGS_t*) buf;

	return snprintf(mesBuf, free, "%d %d %d\n", ptr->led1, ptr->led2, ptr->led3);
}

static inline int printLEDTypes(char *mesBuf, int free, uint8_t *buf,
		size_t length) {
	ledtype_t *ptr;
	if (length != sizeof(ledtype_t) || buf == NULL)
		return 0;
	ptr = (ledtype_t*) buf;
	return snprintf(mesBuf, free, "WL %d  #LEDS %d  Imax %d\n", ptr->wavelength,
			ptr->leds, ptr->amps);
}

static inline int PrintCmd(char *mesBuf, uint8_t sender_address, uint8_t *packet,
		size_t length) {
	led_command_t cmd;
	int retval = 0;
	cmd = (led_command_t) packet[0];
	retval = snprintf(mesBuf, 200, "Device %d ", sender_address);
	switch (cmd) {
	case LED_COMMAND_GET_TEMP:
		// unpack temperature and put into mesBuf here
		retval = snprintf(mesBuf+retval,200-retval,"Temp: %f\n",(float)*(packet+1));
		break;
	case LED_COMMAND_GET_TYPES:
		// unpack LED Settings and put into mesBuf here
		retval += printLEDTypes(mesBuf+retval,200-retval,packet+1,length-1);
		break;
	case LED_COMMAND_GET_PWM_SETTINGS:
		// unpack PWM settings and put into mesBuf here
		retval += printPWMSettings(mesBuf+retval,200-retval,&packet[1],length-1);
		break;
	default:
		return 0;
	}
	return retval;
}

void daisyPacketReceived(uint8_t receive_address, uint8_t sender_address,
		uint8_t *packet_ptr, size_t length) {
	int cnt = 5;
	char mesBuf[200] = { "empty" };
	switch (receive_address) {
	case DAISY_ADDR_COUNT:
		if (length > 0)
			cnt = snprintf(mesBuf, 200, "Devices found: %d\n", sender_address);
		break;
	case DAISY_ADDR_BROADCAST:
		//cnt = snprintf(mesBuf, 200, "Broadcast Command: %d\n", packet_ptr[0]);
		cnt = PrintCmd(mesBuf,sender_address,packet_ptr,length);
		break;
	case DAISY_ADDR_ERROR:
		cnt = snprintf(mesBuf, 200, "An Error has occurred\n");
		break;
	case DAISY_ADDR_MASTER:
		cnt = PrintCmd(mesBuf,sender_address,packet_ptr,length);
		break;
	}

	USBD_VCOM_SendData((int8_t*) mesBuf, cnt);
}

void usbCallback(void) {
	static uint8_t bytes = 0;
	uint8_t bytesReceived = 0;

	bytesReceived = USBD_VCOM_BytesReceived();

	if (bytesReceived) {
		while (bytes < BUF_SIZE && bytesReceived > 0) {
			USBD_VCOM_ReceiveByte((int8_t*) &RxBuffer[bytes]);
			--bytesReceived;
			++bytes;

			// if the string ends in LF or CR, evaluate and act accordingly
			if (RxBuffer[bytes - 1] == '\n' || RxBuffer[bytes - 1] == '\r') {
				RxBuffer[bytes - 1] = '\0';
				parseAndAct(bytes);
			}
			return;
		}
	}
bytes = 0;
}

typedef enum {
	DAISY_NONE,
	DAISY_AUTO_DISCOVER,
	DAISY_PING,
	DAISY_SET_ALL,
	DAISY_RESET_ALL,
	DAISY_GET_TEMPS,
	DAISY_GET_TYPES,
	DAISY_GET_PWM_SETTINGS
} DAISY_CHAIN_COMMANDS_t;

static void daisyBroadcastCommand(DAISY_CHAIN_COMMANDS_t command) {
	uint8_t data, addr, sender;
	sender = DAISY_ADDR_MASTER;
	switch (command) {
	case DAISY_AUTO_DISCOVER:
		addr = (uint8_t) DAISY_ADDR_COUNT;
		data = 0;
		break;
	case DAISY_SET_ALL:
		addr = DAISY_ADDR_BROADCAST;
		data = (uint8_t) LED_COMMAND_ON;
		break;
	case DAISY_RESET_ALL:
		addr = DAISY_ADDR_BROADCAST;
		data = (uint8_t) LED_COMMAND_OFF;
		break;
	case DAISY_GET_TEMPS:
		addr = DAISY_ADDR_BROADCAST;
		data = (uint8_t) LED_COMMAND_GET_TEMP;
		break;
	case DAISY_GET_TYPES:
		addr = DAISY_ADDR_BROADCAST;
		data = (uint8_t) LED_COMMAND_GET_TYPES;
		break;
	case DAISY_GET_PWM_SETTINGS:
		addr = DAISY_ADDR_BROADCAST;
		data = (uint8_t) LED_COMMAND_GET_PWM_SETTINGS;
		break;
	default:
		return;
	}
	daisySendData(addr, sender, &data, 1);
}

static inline void parseAndAct(size_t bytes) {
	char *str;
	char *saveptr = NULL;
	PWM_SETTINGS_t leds;
	str = strtok_r((char*) RxBuffer, " ", &saveptr);
	if (str == NULL) {
		return;
	}
	if (strncmp("discover", str, bytes) == 0) {
		daisyBroadcastCommand(DAISY_AUTO_DISCOVER);
	} else if (strncmp("ping", str, bytes) == 0) {
		daisyBroadcastCommand(DAISY_PING);
	} else if (strncmp("setall", str, bytes) == 0) {
		daisyBroadcastCommand(DAISY_SET_ALL);
	} else if (strncmp("resetall", str, bytes) == 0) {
		daisyBroadcastCommand(DAISY_RESET_ALL);
	} else if (strncmp("set", str, bytes) == 0) {
		uint8_t address;

		str = strtok_r(NULL, " ", &saveptr);
		address = (uint8_t) atoi(str);
		leds.led1 = (uint16_t) atoi(strtok_r(NULL, " ", &saveptr));
		leds.led2 = (uint16_t) atoi(strtok_r(NULL, " ", &saveptr));
		leds.led3 = (uint16_t) atoi(strtok_r(NULL, " ", &saveptr));
		bytes = 0;
		TxBuffer[0] = (uint8_t) LED_COMMAND_SET;
		memcpy(TxBuffer + 1, &leds, sizeof(leds));
		daisySendData(address, DAISY_ADDR_MASTER, TxBuffer, sizeof(leds) + 1);

	} else if (strncmp("temperature", str, bytes) == 0) {
		daisyBroadcastCommand(DAISY_GET_TEMPS);
	} else if (strncmp("types", str, bytes) == 0) {
		daisyBroadcastCommand(DAISY_GET_TYPES);
	} else if (strncmp("getpwm",str,bytes) == 0) {
		daisyBroadcastCommand(DAISY_GET_PWM_SETTINGS);
	}
}

