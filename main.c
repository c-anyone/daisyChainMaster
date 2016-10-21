/*
 * main.c
 *
 *  Created on: 2016 Jun 29 18:28:54
 *  Author: Fabio Pungg
 */

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include "./xmc_daisyChain/uart_cobs.h"

//#include "./xmc_daisyChain/DaisyChain.h"
/**

 * @brief main() - Application entry point
 *
 * <b>Details of function</b><br>
 * This routine is the application entry point. It is invoked by the device startup code.It is
 * responsible for invoking the APP initialization dispatcher routine - DAVE_Init() and hosting
 * the place-holder for user application
 * code.
 */
#define		ONESEC 		1000000U
#define		ONEMSEC		1000U

#define BUF_SIZE	(64u)
int8_t RxBuffer[BUF_SIZE] = { 0 };
int8_t TxBuffer[BUF_SIZE] = { 0 };
uint8_t Bytes;

static void usbCallback(void);
void receiveCallback(uint8_t address, uint8_t length, uint8_t *buf);

void sendPing() {
	DIGITAL_IO_ToggleOutput(&LED1);
	DIGITAL_IO_ToggleOutput(&LED2);
}

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

	uartCobsInit(UART_DAISY.channel);

	while (1U) {

		usbCallback();

		daisyWorker();

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

/*
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
 */
/*
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
 */

PWM_SETTINGS_t leds = { .identifier = 0x4142, .led1 = 0x4300, .led2 = 0x4400,
		.led3 = 0x4545 };

void usbCallback(void) {
	static uint8_t bytes = 0;
	uint8_t bytesReceived = 0;

	char *str;
	char *saveptr = NULL;
	bytesReceived = USBD_VCOM_BytesReceived();

	if (bytesReceived) {
		while (bytes < BUF_SIZE && bytesReceived > 0) {
			USBD_VCOM_ReceiveByte(&RxBuffer[bytes]);
			--bytesReceived;
			++bytes;

			// if the string ends in LF or CR, evaluate and act accordingly
			if (RxBuffer[bytes - 1] == '\n' || RxBuffer[bytes - 1] == '\r') {
				RxBuffer[bytes - 1] = '\0';
				str = strtok_r((char*) RxBuffer, " ", &saveptr);
				if (str == NULL) {
					break;
				} else if (strncmp("test", str, bytes) == 0) {
					uartCobsTransmit((uint8_t*) &leds, sizeof(leds));
				}

			}

			return;
		}
	}
	bytes = 0;
}

