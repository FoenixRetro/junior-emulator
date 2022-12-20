// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		hardware.c
//		Purpose:	Hardware Emulation
//		Created:	29th June 2022
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include "sys_processor.h"
#include "hardware.h"

#include "gfx.h"
#include <stdio.h>
#include <stdlib.h>

#define QSIZE 	(10)

struct _Queue {
	int   	count;
	int   	queue[QSIZE];
};

struct _Queue keyboardQueue;

static int SN76489_reg[8];										// 8 registers of 76489 (Tone/Attenuation 0..3)
static int SN76489_current = 0;									// Currently selected register.

static BYTE8 ioMemory[4*0x4000];

static void HWWriteSoundChip(int data);

// *******************************************************************************************************************************
//												Read from I/O Space
// *******************************************************************************************************************************

BYTE8 IOReadMemory(BYTE8 page,WORD16 address) {
	if (page == 0) {
		if (address >= 0xD640 && address < 0xD650) {
			return HWReadKeyboardHardware(address);
		}
		if (address == 0xDC00) {
			return GFXReadJoystick0();
		}
	}
	return ioMemory[(page << 14)|(address & 0x3FFF)];
}

// *******************************************************************************************************************************
//												Write to I/O Space
// *******************************************************************************************************************************

void IOWriteMemory(BYTE8 page,WORD16 address,BYTE8 data) {
	if (page == 0) {
		if (address == 0xD600 || address == 0xD610) {
			HWWriteSoundChip(data);
		}
		if (address >= 0xD640 && address < 0xD650) {
			HWWriteKeyboardHardware(address,data);
		}
	}
	ioMemory[(page << 14)|(address & 0x3FFF)] = data;
}

// *******************************************************************************************************************************
//												Insert/Delete Queue
// *******************************************************************************************************************************

static void HWQueueInsert(struct _Queue *q,int value) {
	if (q->count < QSIZE) q->queue[q->count++] = value;
}

static int HWQueueRemove(struct _Queue *q) {
	if (q->count == 0) return -1;
	int r = q->queue[0];
	for (int i = 0;i < q->count;i++) q->queue[i] = q->queue[i+1];
	q->count--;
	return r;
}

// *******************************************************************************************************************************
//												Reset Hardware
// *******************************************************************************************************************************

#include "roms/foenix_charset.h"

void HWReset(void) {
	keyboardQueue.count = 0;
	HWResetKeyboardHardware();
	for (int i = 0;i < 4;i++) {				
		SN76489_reg[i*2+1] = 0xF;							// Set all attenuation to $F e.g. off
		SN76489_reg[i*2+0] = 0;								// Pitch zero.
		GFXSetFrequency(i,0);								// All beepers off
	}
	for (int i = 0;i < 0x800;i++) {
		IOWriteMemory(1,0xC000+i,foenix_charset[i]);
	}
}

// *******************************************************************************************************************************
//												  Reset CPU
// *******************************************************************************************************************************

void HWSync(void) {
	if (keyboardQueue.count != 0) {
		int key = HWQueueRemove(&keyboardQueue);
		HWKeyboardHardwareDequeue(key);
		//printf("Dequeue %x\n",key);
		if (HWCheckKeyboardInterruptEnabled()) {
			//printf("Interrupt\n");
			CPUInterruptMaskable();							// fire IRQ
		}
	}
	if (IOReadMemory(0,0xD658) & 1) { 						// Timer on.
		int a = 0xD659;
		int c = 1;
		while (c != 0) {
			int b = IOReadMemory(0,a)+1;
			IOWriteMemory(0,a,b & 0xFF);
			c = (b == 0x100) ? 1 : 0;
			a++;
		}
	}
}

// *******************************************************************************************************************************
//									  Write to display/colour RAM
// *******************************************************************************************************************************

void HWWriteDisplay(WORD16 address,BYTE8 data) {	
}

// *******************************************************************************************************************************
//									  Receive faux PS/2 Keyboard event
// *******************************************************************************************************************************

void HWQueueKeyboardEvent(int ps2code) {
	HWQueueInsert(&keyboardQueue,ps2code);
}

// *******************************************************************************************************************************
//								Write byte to 76489, handles left & right
// *******************************************************************************************************************************


static int _HWGetFrequency(void) {
	int rPair = SN76489_current & 0xFE;
	if (SN76489_reg[rPair+1] != 0 || SN76489_reg[rPair] == 0) return 0;
	return 111563 / SN76489_reg[rPair];
}

static void HWWriteSoundChip(int data) {
	int startFreq,endFreq;
	if (data & 0x80) {
		SN76489_current = (data >> 4) & 7;
	}
	startFreq = _HWGetFrequency();
	if (data & 0x80) {
		SN76489_reg[SN76489_current] &= 0xFFF0;
		SN76489_reg[SN76489_current] |= data & 0x0F;
	} else {
		SN76489_reg[SN76489_current] &= 0x000F;
		SN76489_reg[SN76489_current] |= ((data & 0x3F) << 4);
	}
	//printf("Register %d is %x %d\n",SN76489_current,SN76489_reg[SN76489_current],SN76489_reg[SN76489_current]);
	endFreq = _HWGetFrequency();
	if (startFreq != endFreq) {
		int gChannel = (SN76489_current >> 1) ^ 3;
		//printf("Changing pitch of %d to %d\n",gChannel,endFreq);
		GFXSetFrequency(endFreq,gChannel);
	}
	//printf("Change: %d %d\n",endFreq,startFreq);
}
