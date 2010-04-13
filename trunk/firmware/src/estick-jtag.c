/*
    estick-jtag, by Cahya Wirawan <cahya@gmx.at> 
    Based on opendous-jtag by Vladimir Fonov and LUFA demo applications by Dean Camera and Denver Gingerich.
    Released under the MIT Licence.
*/

#include "estick-jtag.h"
#include "jtag_defs.h"
#include "jtag_functions.h"

#ifdef DEBUG
#include <LUFA/Drivers/AT90USBXXX/Serial_Stream.h>
#include <stdio.h>
#endif //DEBUG

/* Scheduler Task List */
TASK_LIST
{
	{ .Task= USB_USBTask         , .TaskStatus= TASK_STOP },
	{ .Task= USB_MainTask        , .TaskStatus= TASK_STOP },
};

/* Global Variables */

uint8_t  usbBuffer[ESTICK_USB_BUFFER_SIZE+ESTICK_USB_BUFFER_OFFSET];
uint8_t  *dataFromHost = usbBuffer+ESTICK_USB_BUFFER_OFFSET;
uint8_t  *dataToHost = usbBuffer;
uint16_t dataFromHostSize=0;
uint16_t dataToHostSize=0;

volatile uint8_t resetJtagTransfers=0;

/** Main program entry point. This routine configures the hardware required by the application, then
 *  starts the scheduler to run the USB management task.
 */
int main(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable Clock Division */
	clock_prescale_set(clock_div_1);
  jtag_init();

  //DEBUG
#ifdef DEBUG
  SerialStream_Init(9600,0);
#endif //DEBUG
  
	// initialize the send and receive buffers
	uint16_t i = 0;
	for (i = 0; i < ESTICK_OUT_BUFFER_SIZE; i++) {
		dataFromHost[i] = 0;
	}

	for (i = 0; i < ESTICK_IN_BUFFER_SIZE; i++) {
		dataToHost[i] = 0;
	}
  dataFromHostSize=0;
  dataToHostSize=0;
  resetJtagTransfers=0;

 	/* Initialize Scheduler so that it can be used */
	Scheduler_Init();

	/* Initialize USB Subsystem */
	USB_Init();
#ifdef DEBUG
	printf("Starting estick-jtag\r\n");
#endif //DEBUG
	/* Scheduling - routine never returns, so put this last in the main function */
	Scheduler_Start();
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Connect(void)
{
 	/* Start USB management task */
	Scheduler_SetTaskMode(USB_USBTask, TASK_RUN);
	Scheduler_SetTaskMode(USB_MainTask, TASK_RUN);
}


/** Event handler for the USB_Reset event. This fires when the USB interface is reset by the USB host, before the
 *  enumeration process begins, and enables the control endpoint interrupt so that control requests can be handled
 *  asynchronously when they arrive rather than when the control endpoint is polled manually.
 */
//void EVENT_USB_Reset(void)
//{
//	/* Select the control endpoint */
//  Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);

	/* Enable the endpoint SETUP interrupt ISR for the control endpoint */
//	USB_INT_Enable(ENDPOINT_INT_SETUP);
//}


/** Event handler for the USB_Disconnect event.
 */
void EVENT_USB_Disconnect(void)
{
	/* Stop running keyboard reporting and USB management tasks */
	Scheduler_SetTaskMode(USB_MainTask, TASK_STOP);
	Scheduler_SetTaskMode(USB_USBTask, TASK_STOP);
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the keyboard device endpoints.
 */
void EVENT_USB_ConfigurationChanged(void)
{
	/* Setup Keyboard Keycode Report Endpoint */
	Endpoint_ConfigureEndpoint(IN_EP, EP_TYPE_BULK,
								ENDPOINT_DIR_IN, IN_EP_SIZE,
								ENDPOINT_BANK_SINGLE);

	/* Enable the endpoint IN interrupt ISR for data being sent TO the host */
	//USB_INT_Enable(ENDPOINT_INT_IN);

	/* Setup Keyboard LED Report Endpoint */
	Endpoint_ConfigureEndpoint(OUT_EP, EP_TYPE_BULK,
								ENDPOINT_DIR_OUT, OUT_EP_SIZE,
								ENDPOINT_BANK_SINGLE);

	/* Enable the endpoint OUT interrupt ISR for data recevied FROM the host */
	//USB_INT_Enable(ENDPOINT_INT_OUT);

	/* start USB_USBTask */
	Scheduler_SetTaskMode(USB_MainTask, TASK_RUN);
  
  // pull lines TRST and SRST high
  JTAG_OUT=(1<<JTAG_PIN_TRST)|(1<<JTAG_PIN_SRST);
  
}


//TODO: add possibility to abor current JTAG sequence and reset the pins

/** Event handler for the USB_UnhandledControlPacket event. This is used to catch standard and class specific
 *  control requests that are not handled internally by the USB library (including the HID commands, which are
 *  all issued via the control endpoint), so that they can be handled appropriately for the application.
 */
void EVENT_USB_UnhandledControlPacket(void)
{
	//NOTE - this is here as a template only, LoopBack does not make use of it

	/* Handle HID Class specific requests here (these are Control EP requests) */
	switch (USB_ControlRequest.bRequest)
	{
		case 0x01:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();

        //TODO add reset code here
				/* Write the report data to the control endpoint */
				//Endpoint_Write_Control_Stream_LE(&dataToSend, sizeof(dataToSend));
        
				/* Finalize the transfer, acknowedge the host error or success OUT transfer */
				Endpoint_ClearOUT();
			}
  		break;
	}
}

TASK(USB_MainTask)
{	
	/* Check if the USB System is connected to a Host */
	if (USB_IsConnected)
	{
		/* process data or do something generally useful */
		/* note that TASK(USB_MainTask) will be periodically executed when no other tasks or functions are running */
     
    Endpoint_SelectEndpoint(IN_EP);

    if (dataToHostSize && Endpoint_IsReadWriteAllowed())
    {
      if(dataToHostSize)
        Endpoint_Write_Stream_LE(dataToHost,dataToHostSize);
      
      /* Handshake the IN Endpoint - send the data to the host */
      //Endpoint_ClearCurrentBank();
      Endpoint_ClearIN();
      
      dataToHostSize=0;
    }

    Endpoint_SelectEndpoint(OUT_EP);

    if (Endpoint_IsReadWriteAllowed())
    {
	  dataFromHostSize = Endpoint_Read_Word_LE();
	  Endpoint_Read_Stream_LE(dataFromHost, dataFromHostSize);
	  Endpoint_ClearOUT();
	  if(dataFromHostSize>0)
      {        
        //first byte is always the command
        dataFromHostSize--;
        
        dataToHostSize=0;
        
        switch( dataFromHost[0] &JTAG_CMD_MASK ) 
        {
          
        case JTAG_CMD_TAP_OUTPUT:
          
          dataFromHostSize*=4;

          if( dataFromHost[0] & JTAG_DATA_MASK )
            dataFromHostSize-= (4- ((dataFromHost[0] & JTAG_DATA_MASK)>>4));
          if(jtag_delay)
            dataToHostSize= jtag_tap_output_with_delay( &dataFromHost[1] , dataFromHostSize, dataToHost);
          else
            dataToHostSize= jtag_tap_output_max_speed( &dataFromHost[1] , dataFromHostSize, dataToHost);
          break;
          
        case JTAG_CMD_TAP_OUTPUT_EMU:
          dataFromHostSize*=4;
          if(dataFromHost[0]&JTAG_DATA_MASK)
            dataFromHostSize-=(4- ((dataFromHost[0]&JTAG_DATA_MASK)>>4));
          
          dataToHostSize=jtag_tap_output_emu(&dataFromHost[1], dataFromHostSize, dataToHost);
          
          break;
          
        case JTAG_CMD_READ_INPUT:
          dataToHost[0]=jtag_read_input();
          dataToHostSize=1;
          break;
        
        case JTAG_CMD_SET_SRST:
          jtag_set_srst(dataFromHost[1]&1);
          dataToHost[0]=0;//TODO: what to output here?
          dataToHostSize=1;
          break;
        
        case JTAG_CMD_SET_TRST:
          jtag_set_trst(dataFromHost[1]&1);
          dataToHost[0]=0;//TODO: what to output here?
          dataToHostSize=1;
          break;
        
        case JTAG_CMD_SET_DELAY:
          jtag_delay=dataFromHost[1]*256;
          dataToHost[0]=0;//TODO: what to output here?
          dataToHostSize=1;

        case JTAG_CMD_SET_SRST_TRST:
          jtag_set_trst_srst(dataFromHost[1]&2?1:0,dataFromHost[1]&1);
          dataToHost[0]=0;//TODO: what to output here?
          dataToHostSize=1;
        
        default: //REPORT ERROR?
          break;
        }
      }
    }
	}
}
