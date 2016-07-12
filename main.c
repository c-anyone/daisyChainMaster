/*
 * main.c
 *
 *  Created on: 2016 Jun 29 18:28:54
 *  Author: faebsn
 */




#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include "DaisyChain.h"
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
void receiveCallback(uint8_t length,uint8_t *buf);

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

/*
 *  callback for daisy chain reception
 *  in this case (master) it should implement catologuing of devices and functions
 *  as well as transmitting received data to a controlling computer
 */
void receiveCallback(uint8_t length,uint8_t *buf) {
	USBD_VCOM_SendData((int8_t*)buf,length);
}

void usbCallback(void) {
	static uint8_t bytes = 0;
	uint8_t bytesReceived = 0;

	bytesReceived = USBD_VCOM_BytesReceived();

	if(bytesReceived) {
		while(bytes < BUF_SIZE && bytesReceived > 0) {
			USBD_VCOM_ReceiveByte(&RxBuffer[bytes]);
			--bytesReceived;
			if(RxBuffer[bytes]=='\n' || RxBuffer[bytes] == '\r') {
				RxBuffer[bytes] = '\0';
				break;
			}
			++bytes;
		}
	//	USBD_VCOM_SendData(RxBuffer,bytes);
		daisySendData(0xff,bytes,(uint8_t*)RxBuffer);
		bytes = 0;
	}
}

