/*
    estick-jtag, by Cahya Wirawan <cahya@gmx.at> 
    Based on opendous-jtag by Vladimir Fonov and LUFA demo applications by Dean Camera and Denver Gi
ngerich.
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
