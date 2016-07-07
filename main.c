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
 #define ONESEC 1000000U

#define BUF_SIZE	(64u)
int8_t RxBuffer[BUF_SIZE] = { 0 };
int8_t TxBuffer[BUF_SIZE] = { 0 };
uint8_t Bytes;

static void usbCallback(void);

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
		}
	}


	if(USBD_VCOM_Connect() != USBD_VCOM_STATUS_SUCCESS)
	{
		return -1;
	}
	while(!USBD_VCOM_IsEnumDone());
    XMC_UART_CH_Start(UART_DAISY.channel);
//	XMC_USIC_CH_TXFIFO_Flush(UART_DAISY.channel);
//	XMC_USIC_CH_TXFIFO_PutData(UART_DAISY.channel,0x35);
//	XMC_UART_CH_Transmit(UART_DAISY.channel,0x35);

	  uint32_t TimerId = SYSTIMER_CreateTimer(ONESEC,SYSTIMER_MODE_PERIODIC,(void*)sendPing,NULL);
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
	while (1U)
	{
		Bytes = USBD_VCOM_BytesReceived();

		if (Bytes)
		{

			// USBD_VCOM_ReceiveByte(&RxBuffer[0]);

			//USBD_VCOM_SendByte(RxBuffer[0]);

			usbCallback();

		}
		CDC_Device_USBTask(&USBD_VCOM_cdc_interface);
	}

	return 1;
}

void usbCallback(void) {
	static uint8_t bytes = 0;
//	uint8_t tmp = 0;
	uint8_t bytesReceived = 0;
	bytesReceived = USBD_VCOM_BytesReceived();
	if(bytesReceived) {
//		for(;bytes < BUF_SIZE && bytesReceived > 0 && RxBuffer[bytes] != '\n'; ++bytes, --bytesReceived) {
//			USBD_VCOM_ReceiveByte(&RxBuffer[bytes]);
//		}
		while(bytes < BUF_SIZE && bytesReceived > 0) {
			USBD_VCOM_ReceiveByte(&RxBuffer[bytes]);
			--bytesReceived;
			if(RxBuffer[bytes]=='\n' || RxBuffer[bytes] == '\r') {
				RxBuffer[bytes] = '\0';
				break;
			}
			++bytes;
		}
		USBD_VCOM_SendData(RxBuffer,bytes);
		DaisyChainSendData(0x00,bytes,(uint8_t*)RxBuffer);
		bytes = 0;
	}
}

