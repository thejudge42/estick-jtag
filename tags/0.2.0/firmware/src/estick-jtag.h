/*
    estick-jtag, by Cahya Wirawan <cahya@gmx.at> 
    Based on opendous-jtag by Vladimir Fonov and LUFA demo applications by Dean Camera and Denver Gingerich.
    Released under the MIT Licence.
*/

#ifndef _ESTICK_JTAG_H_
#define _ESTICK_JTAG_H_

	/* Includes: */
	#include <avr/io.h>
	#include <avr/wdt.h>
	#include <avr/power.h>
	#include <util/delay_basic.h>
	#include "Descriptors.h"

	#include <LUFA/Version.h>				// Library Version Information
	#include <LUFA/Drivers/USB/USB.h>            // USB Functionality
	#include <LUFA/Scheduler/Scheduler.h>		// Simple scheduler for task management

	/* Macros: */

	/* Type Defines: */
	
	#define ESTICK_USB_BUFFER_SIZE 360
	#define ESTICK_USB_BUFFER_OFFSET 2
	#define ESTICK_IN_BUFFER_SIZE	(ESTICK_USB_BUFFER_SIZE)
	#define ESTICK_OUT_BUFFER_SIZE  (ESTICK_USB_BUFFER_SIZE)

	/* Global Variables: */

	/* Task Definitions: */
	TASK(USB_MainTask);

	/* Event Handlers: */
	void EVENT_USB_Connect(void);
	void EVENT_USB_Reset(void);
	void EVENT_USB_Disconnect(void);
	void EVENT_USB_ConfigurationChanged(void);
	void EVENT_USB_UnhandledControlPacket(void);

	/* Function Prototypes: */

#endif //ESTICK_JTAG
