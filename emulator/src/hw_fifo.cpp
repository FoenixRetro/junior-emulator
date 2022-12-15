// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		hw_fifo.c
//		Purpose:	Hardware Emulation (Stefany's interface)
//		Created:	12th December 2022
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include "sys_processor.h"
#include "hardware.h"

#include "gfx.h"
#include <stdio.h>
#include <stdlib.h>

#define FIFO_QUEUE_SIZE		(8)

static int queueSize;
static int fifoQueue[FIFO_QUEUE_SIZE];

// *******************************************************************************************************************************
//
//											Empty the new FIFO queue
//
// *******************************************************************************************************************************

void HWResetKeyboardHardware(void) {	
	queueSize = 0;
}

// *******************************************************************************************************************************
//
//								Add a received key event to the queue, if space available
//
// *******************************************************************************************************************************

void HWKeyboardHardwareDequeue(int key) {
	//printf("Received : %x\n",key);
	if (queueSize < FIFO_QUEUE_SIZE) {
		fifoQueue[queueSize++] = key;
	}
}

// *******************************************************************************************************************************
//
//											Simple PS/2 implementation
//
// *******************************************************************************************************************************

BYTE8 HWReadKeyboardHardware(WORD16 address) {
	if (address == 0xD644) {
		return (queueSize == 0) ? 1 : 0;
	}
	if (address == 0xD642 && queueSize > 0) {
		int head = fifoQueue[0];
		//printf("Popped : %x\n",head);
		for (int i = 0;i < queueSize;i++) {
			fifoQueue[i] = fifoQueue[i+1];
		}
		queueSize--;
		return head;
	}
	return 0;
}

// *******************************************************************************************************************************
//
//							Even simpler control implementation, sufficient to function
//
// *******************************************************************************************************************************

void HWWriteKeyboardHardware(WORD16 address,BYTE8 data) {
	// Should clear the queue here. Not really required ?
}

// *******************************************************************************************************************************
//
//										Interrupt for new key enabled ?
//
// *******************************************************************************************************************************

int HWCheckKeyboardInterruptEnabled(void) {
	if ((IOReadMemory(0,0xD66C) & 0x04) == 0) {			// Bit masked zero ?
		IOWriteMemory(0,0xD660,4); 						// Set Pending Reg Bit 2
		return -1;
	}
	return 0;
}