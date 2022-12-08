// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		hardware.h
//		Purpose:	Hardware Emulation Header
//		Created:	29th June 2022
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#ifndef _HARDWARE_H
#define _HARDWARE_H

void HWReset(void);
void HWSync(void);
BYTE8 HWWriteKeyboard(BYTE8 pattern);
void HWWriteDisplay(WORD16 address,BYTE8 data);
int HWGetScanCode(void);
void HWWriteCharacter(WORD16 x,WORD16 y,BYTE8 ch);
void HWQueueKeyboardEvent(int ps2code);

BYTE8 IOReadMemory(BYTE8 page,WORD16 address);
void IOWriteMemory(BYTE8 page,WORD16 address,BYTE8 data);
BYTE8 IOReadSource(void);

BYTE8 HWReadKeyboardHardware(WORD16 address);
void HWWriteKeyboardHardware(WORD16 address,BYTE8 data);
void HWResetKeyboardHardware(void);
void HWKeyboardHardwareDequeue(int key);
int HWCheckKeyboardInterruptEnabled(void);

#endif