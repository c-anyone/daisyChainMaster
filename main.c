/*
 * main.c
 *
 *  Created on: 2016 Jun 29 18:28:54
 *  Author: Fabio Pungg
 */




#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include "./xmc_daisyChain/DaisyChain.h"
/**

 * @brief main() - Application entry point
 *
 * <b>Details of function</b><br>
 * This routine is the application entry point. It is invoked by the device startup code. It is responsible for
 * invoking the APP initialization dispatcher routine - DAVE_Init() and hosting the place-holder for user application
 * code.
 */
#define		ONESEC 		1000000U
#define		ONEMSEC		1000U

#define BUF_SIZE	(64u)
int8_t RxBuffer[BUF_SIZE] = { 0 };
int8_t TxBuffer[BUF_SIZE] = { 0 };
uint8_t Bytes;

static void usbCallback(void);
void receiveCallback(uint8_t address,uint8_t length,uint8_t *buf);

void sendPing() {
	DIGITAL_IO_ToggleOutput(&LED1);
	DIGITAL_IO_ToggleOutput(&LED2);
}

int main(void)
{
	DAVE_STATUS_t status;

	status = DAVE_Init();
	if (status == DAVE_STATUS_FAILURE)
	{
		XMC_DEBUG(("DAVE Apps initialization failed with status %d\n", status));
		while (1U)
		{
			// Error handling code here
		}
	}

	if(USBD_VCOM_Connect() != USBD_VCOM_STATUS_SUCCESS)
	{
		return -1;
	}
	while(!USBD_VCOM_IsEnumDone());


	uint32_t TimerId = SYSTIMER_CreateTimer(ONEMSEC,SYSTIMER_MODE_PERIODIC,(void*)usbCallback,NULL);
	if(TimerId != 0U)
	{
		// Timer is created successfully
		// Start/Run Software Timer
		uint32_t status = SYSTIMER_StartTimer(TimerId);
		if(status == SYSTIMER_STATUS_SUCCESS)
		{
			// Timer is running
		}
	}

	daisyInit(&UART_DAISY);		// init the daisy chain layer and start uart
	daisySetRxCallback(receiveCallback);	// set the receive callback function

	while (1U)
	{
		CDC_Device_USBTask(&USBD_VCOM_cdc_interface);


	}
	return 1;
}



typedef struct {
	uint16_t identifier;
	uint16_t led1;		// zero equals off
	uint16_t led2;
	uint16_t led3;
} PWM_SETTINGS_t;



void receiveCallback(uint8_t address,uint8_t length,uint8_t *buf) {
	uint8_t cnt = 5;
	char mesBuf[200] = {"empty"};
	PWM_SETTINGS_t* ptr;
	switch(address) {
	case DAISY_ADDR_COUNT:
		if(length>0)
			cnt = snprintf(mesBuf,200,"Devices found: %d\n",buf[0]);
		break;
	case DAISY_BROADCAST:
		if(length == sizeof(PWM_SETTINGS_t)) {
			ptr = (PWM_SETTINGS_t*) buf;
			cnt = snprintf(mesBuf,200,"LEDs set to %d %d %d\n",ptr->led1,ptr->led2,ptr->led3);
		}
		break;
	case DAISY_ERROR:
		cnt = snprintf(mesBuf,200,"An Error has occured\n");
		break;
	}

	USBD_VCOM_SendData((int8_t*)mesBuf,cnt);
}

typedef enum {
	DAISY_NONE,
	DAISY_AUTO_DISCOVER,
	DAISY_PING,
	DAISY_SET_ALL,
	DAISY_RESET_ALL
} DAISY_CHAIN_COMMANDS_t;
PWM_SETTINGS_t leds;
void send_commands(DAISY_CHAIN_COMMANDS_t com) {
//	PWM_SETTINGS_t leds = { 0x11,0x22,0x33,0x44 };
	uint8_t tmp = 0;
	switch(com) {
	case DAISY_AUTO_DISCOVER:
		daisySendData(DAISY_ADDR_COUNT,1,&tmp);
		break;
	case DAISY_PING:
		daisySendData(DAISY_BROADCAST,0,&tmp);
		break;
	case DAISY_SET_ALL:
		leds.led1 = leds.led2 = leds.led3 = 0;
		daisySendData(DAISY_BROADCAST,sizeof(leds),(uint8_t*) &leds);
		break;
	case DAISY_RESET_ALL:
		leds.led1 = leds.led2 = leds.led3 = 10000;
		daisySendData(DAISY_BROADCAST,sizeof(leds),(uint8_t*) &leds);
		break;
	case DAISY_NONE:
		return;
	}
}

void usbCallback(void) {
	static uint8_t bytes = 0;
	uint8_t bytesReceived = 0;
	DAISY_CHAIN_COMMANDS_t command = DAISY_NONE;
	char *str;
	char *saveptr = NULL;
	bytesReceived = USBD_VCOM_BytesReceived();

	if(bytesReceived) {
		while(bytes < BUF_SIZE && bytesReceived > 0) {
			USBD_VCOM_ReceiveByte(&RxBuffer[bytes]);
			--bytesReceived; ++bytes;

			// if the string ends in LF or CR, evaluate and act accordingly
			if(RxBuffer[bytes-1]=='\n' || RxBuffer[bytes-1] == '\r') {
				RxBuffer[bytes-1] = '\0';
				str = strtok_r((char*) RxBuffer," ",&saveptr);
				if(str == NULL) {
					break;
				}
				if(strncmp("discover",str,bytes) == 0) {
					command = DAISY_AUTO_DISCOVER;
				} else if (strncmp("ping",str,bytes) == 0) {
					command = DAISY_PING;
				} else if (strncmp("setall",str,bytes) == 0) {
					command = DAISY_SET_ALL;
				} else if (strncmp("resetall",str,bytes) == 0) {
					command = DAISY_RESET_ALL;
				} else if (strncmp("set",str,bytes) == 0) {
					uint8_t address;

					str = strtok_r(NULL," ",&saveptr);
					address = (uint8_t)atoi(str);
					leds.led1 = (uint16_t)atoi(strtok_r(NULL," ",&saveptr));
					leds.led2 = (uint16_t)atoi(strtok_r(NULL," ",&saveptr));
					leds.led3 = (uint16_t)atoi(strtok_r(NULL," ",&saveptr));
					bytes = 0;
					daisySendData(address,sizeof(leds),(uint8_t*) &leds);


				}

				send_commands(command);
				bytes = 0;
				return;
			}
		}
		bytes = 0;
	}
}

