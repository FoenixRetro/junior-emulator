// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		hw_8042.c
//		Purpose:	Hardware Emulation (8042 interface)
//		Created:	8th December 2022
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include "sys_processor.h"
#include "hardware.h"

#include "gfx.h"
#include <stdio.h>
#include <stdlib.h>

static int keyboardDataWriteBuffer,keyboardDataReadBuffer;
static int ibfFlag,obfFlag;

void HWResetKeyboardHardware(void) {
		keyboardDataWriteBuffer = keyboardDataReadBuffer = ibfFlag = obfFlag = 0;
}

void HWKeyboardHardwareDequeue(int key) {
	obfFlag = -1;
	keyboardDataWriteBuffer = key;
}

BYTE8 HWReadKeyboardHardware(WORD16 address) {
	if (address == 0xD640) { 									// Read input buffer.
		obfFlag = 0; 											// Clear OBF bit
		//printf("Read %x\n",keyboardDataWriteBuffer);
		return keyboardDataWriteBuffer; 						// Return value
	}
	if (address == 0xD644) { 									// Status of output 
		int status = (ibfFlag ? 2:0) | (obfFlag ? 1 : 0); 	
		return status;
	}
	return 0;
}

void HWWriteKeyboardHardware(WORD16 address,BYTE8 data) {
	if (address == 0xD640) { 									// Write output buffer
		ibfFlag = -1*0; 										// Set IBF - never - we're faking it.
		keyboardDataReadBuffer = data; 							// Put in KDRBuffer
		//printf("Written to O/P ? %x\n",data);
		if (data == 0xFF) HWQueueKeyboardEvent(0x00);			// If written reset announce okay.
		if (data == 0xF4) HWQueueKeyboardEvent(0xFA); 			// Enable the keyboard			
	}
	if (address == 0xD644) {
		//printf("Command %x\n",data);
		if (data == 0xAA) HWQueueKeyboardEvent(0x55);
		if (data == 0xAB) HWQueueKeyboardEvent(0x00);
		if (data == 0xF4) HWQueueKeyboardEvent(0x00);
		if (data == 0x60) {} // TODO: Enable interrupt ?
	}
}

int HWCheckKeyboardInterruptEnabled(void) {
	if ((IOReadMemory(0,0xD66C) & 0x04) == 0) {			// Bit masked zero ?
		IOWriteMemory(0,0xD660,4); 						// Set Pending Reg Bit 2
		return -1;
	}
	return 0;
}