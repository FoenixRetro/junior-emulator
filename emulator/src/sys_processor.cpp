// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		sys_processor.c
//		Purpose:	Processor Emulation.
//		Created:	29th June 2022
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sys_processor.h"
#include "sys_debug_system.h"
#include "hardware.h"

// *******************************************************************************************************************************
//														   Timing
// *******************************************************************************************************************************

#define CYCLE_RATE 		(6290*1000)													// Cycles per second (0.96Mhz)
#define FRAME_RATE		(70)														// Frames per second (50 arbitrary)
#define CYCLES_PER_FRAME (CYCLE_RATE / FRAME_RATE)									// Cycles per frame (20,000)

// *******************************************************************************************************************************
//														CPU / Memory
// *******************************************************************************************************************************

static BYTE8 a,x,y,s;																// 6502 A,X,Y and Stack registers
static BYTE8 carryFlag,interruptDisableFlag,breakFlag,								// Values representing status reg
			 decimalFlag,overflowFlag,sValue,zValue;
static WORD16 pc;																	// Program Counter.
static BYTE8 ramMemory[MEMSIZE];													// Memory at $0000 upwards
static BYTE8 writeProtect;
static BYTE8 isPageCMemory; 														// Is Page $C000-$DFFF memory.
static int argumentCount;
static char **argumentList;
static LONG32 cycles;																// Cycle Count.
static BYTE8 inFastMode; 															// Fast mode
static BYTE8 *currentMap;  															// Current map (8 bytes)
static BYTE8 *currentEditMap; 														// Current edited map (may be NULL)
static BYTE8 mappingMemory[32]; 													// Current mapped memory.
static BYTE8 trackingCalls; 														// Tracking JSR/RTS ?
static BYTE8 MMURegister;	 														// The MMU register
static BYTE8 IORegister; 															// The I/O Register.

// *******************************************************************************************************************************
//											 Memory and I/O read and write macros.
// *******************************************************************************************************************************

#define Read(a) 	_Read(a)														// Basic Read
#define Write(a,d)	_Write(a,d)														// Basic Write

#define ReadWord(a) (Read(a) | ((Read((a)+1) << 8)))								// Read 16 bit, Basic

#define Cycles(n) 	cycles += (n)													// Bump Cycles

#define Fetch() 	_Read(pc++)														// Fetch byte
#define FetchWord()	{ temp16 = Fetch();temp16 |= (Fetch() << 8); }					// Fetch word

static inline BYTE8 _Read(WORD16 address);											// Need to be forward defined as 
static inline void _Write(WORD16 address,BYTE8 data);								// used in support functions.

#include "6502/__6502support.h"

// *******************************************************************************************************************************
//											   Read and Write Inline Functions
// *******************************************************************************************************************************

#define MAPPING(a)  (((currentMap[(a) >> 13] << 13) | ((a) & 0x1FFF)) & 0xFFFFF)				// Map address through mapping table.

BYTE8 *CPUAccessMemory(void) {
	return ramMemory;
}

static inline BYTE8 _Read(WORD16 address) {

	if (isPageCMemory == 0 && address >= 0xC000 && address < 0xE000) { 				// Hardware check
		return IOReadMemory(IORegister & 3,address);
	} 

	if (currentEditMap != NULL && address >= 8 && address < 16) { 					// Access current memory map if editing only.
		return currentEditMap[address-8];
	}

	if (address == 0) return MMURegister;
	if (address == 1) return IORegister;

	int a = MAPPING(address);
	return ramMemory[a];
}

static inline void _Write(WORD16 address,BYTE8 data) { 
	if (address == 0xFFFA) inFastMode = data; 										// Switch fast off/on

	if (address < 16) { 															// Writing in the control area perhaps.
		if (currentEditMap != NULL && address >= 8 && address < 16) { 				// Writing current memory map in editing mode.
			currentEditMap[address-8] = data;
			return;
		}
		if (address == 1) IORegister = data;

		if (address == 0) {															// Accessing MMU Control
			MMURegister = data;
			currentMap = mappingMemory + 8 * (data & 3); 	 						// Select current usage map.
			currentEditMap = NULL;
			if (data & 0x80) { 														// Edit mode ?
				currentEditMap = mappingMemory + 8 * ((data >> 4) & 3); 			// Set current edit map pointer.
			}
		}

		if (address == 1) { 														// Accessing I/O control
			isPageCMemory = ((IORegister & 4) != 0); 								// Set Page C usage flag
		}
		return;
	}


	if (isPageCMemory == 0 && address >= 0xC000 && address < 0xE000) {				// Hardware check.
		IOWriteMemory(IORegister&3,address,data);
	} else {
		int mapAddr = MAPPING(address); 											// Write if in first 512k
		if (mapAddr < 0x8000000) {
			ramMemory[mapAddr] = data;
		}
	}
}

// *******************************************************************************************************************************
//													Remember Arguments
// *******************************************************************************************************************************

void CPUSaveArguments(int argc,char *argv[]) {
	argumentCount = argc;
	argumentList = argv;
}

// *******************************************************************************************************************************
//													ROM Copying helper
// *******************************************************************************************************************************

void CPUCopyROM(int address,int size,const BYTE8 *data) {
	for (int i = 0;i < size;i++) ramMemory[address+i] = data[i];					
}

// *******************************************************************************************************************************
//														Reset the CPU
// *******************************************************************************************************************************

#include "roms/monitor_rom.h"
#include "roms/character_rom.h"

static void CPULoadChunk(FILE *f,BYTE8* memory,WORD16 address,int count);

void CPUReset(void) {
	writeProtect = 0;
	currentMap = mappingMemory; 													// Current access map
	currentEditMap = NULL; 															// Not editing.
	MMURegister = IORegister = 0; 													// Default MMU Control
	for (int i = 0;i < 32;i++) { 													// Map LUT 0 to 0-7, all others to 0.
		mappingMemory[i] = (i < 88) ? i : 0;
	}
	mappingMemory[7] = PAGE_MONITOR;												// Map the last to flash memory's location.

	for (int i = 0;i < 8*256;i++) {
		IOWriteMemory(1,i+0xC000,character_rom[i]);
	}

	isPageCMemory = ((IORegister & 4) != 0);										// Set PageC RAM flag.
	CPUCopyROM((PAGE_MONITOR << 13),sizeof(monitor_rom),monitor_rom); 				// Load the tiny kernal by default to page 7.
	CPUCopyROM((0x7F << 13),sizeof(monitor_rom),monitor_rom); 						// Load it also to $7F
	HWReset();																		// Reset Hardware

	#ifdef EMSCRIPTEN  																// Loading in stuff alternative for emScripten
	#include "loaders.h" 															// Partly because preload storage does not work :(
	#endif

	int bootAddress = 0x8000;
	trackingCalls = 0;

	for (int i = 1;i < argumentCount;i++) {
		char szBuffer[128];
		int loadAddress;
		strcpy(szBuffer,argumentList[i]);											// Get buffer
		if (strcmp(szBuffer,"flash") == 0) {
			mappingMemory[7] = 0x7F; 												// Flash command, boot from $7F
		} else if (strcmp(szBuffer,"track") == 0) {
			trackingCalls = -1;
		} else {
			char *p = strchr(szBuffer,'@');
			if (p == NULL) exit(fprintf(stderr,"Bad argument %s\n",argumentList[i]));
			*p++ = '\0';
			loadAddress = -1;
			if (p[1] == '\0') {
				if (toupper(p[0]) == 'B') loadAddress = PAGE_BASIC << 13;
				if (toupper(p[0]) == 'M') loadAddress = PAGE_MONITOR << 13;
				if (toupper(p[0]) == 'X') loadAddress = PAGE_SOURCE << 13;
				if (toupper(p[0]) == 'S') loadAddress = PAGE_SPRITES << 13;
			}
			if (loadAddress < 0) {
				if (sscanf(p,"%x",&loadAddress) != 1) exit(fprintf(stderr,"Bad argument %s\n",argumentList[i]));
			}
			if (strcmp(szBuffer,"boot") != 0) {
				printf("Loading '%s' to $%06x ..",szBuffer,loadAddress);
				FILE *f = fopen(szBuffer,"rb");
				if (f == NULL) exit(fprintf(stderr,"No file %s\n",argumentList[i]));
				while (!feof(f)) {
					if (loadAddress < MEMSIZE) {
						ramMemory[loadAddress++] = fgetc(f);
					}
				}
				fclose(f);
				printf("Okay\n");		
			} else {
				printf("Now booting to $%04x\n",bootAddress);
				bootAddress = loadAddress;
			}
		}
	}
	inFastMode = 0;																	// Fast mode flag reset
	writeProtect = -1;
	resetProcessor();																// Reset CPU
	printf("Booting to %04x\n",bootAddress);
	int patch = (PAGE_MONITOR << 13)+0x1FF8; 										// Where to patch.
	ramMemory[patch] = bootAddress & 0xFF;
	ramMemory[patch+1] = bootAddress >> 8;
}

// *******************************************************************************************************************************
//													  Invoke IRQ
// *******************************************************************************************************************************

void CPUInterruptMaskable(void) {
	irqCode();
}

// *******************************************************************************************************************************
//													 Track JSR/RTS
// *******************************************************************************************************************************

static void CPUTrackCallReturn(BYTE8 opcode) {
	if (opcode == 0x20) {
		WORD16 addr = CPUReadMemory(pc)+CPUReadMemory(pc+1) * 256;
		fprintf(stdout,"TRACK:%04x jsr %04x\n",pc-1,addr);
	}
	if (opcode == 0x60) {
		fprintf(stdout,"TRACK:%04x rts\n",pc-1);
	}
}

// *******************************************************************************************************************************
//												Execute a single instruction
// *******************************************************************************************************************************

BYTE8 CPUExecuteInstruction(void) {
	if (pc == 0xFFFF) {
		printf("CPU $FFFF\n");
		CPUExit();
		return FRAME_RATE;
	}
	BYTE8 opcode = Fetch();															// Fetch opcode.

	//printf("%04x %02x *%02x %02x %02x %02x\n",pc-1,opcode,CPUReadMemory(0x62DC),CPUReadMemory(0x4B),CPUReadMemory(0x4A),y);

	if (trackingCalls != 0) { 														// Tracking for 'C'
		if (opcode == 0x20 || opcode == 0x60) {
			CPUTrackCallReturn(opcode);
		}
	}
	switch(opcode) {																// Execute it.
		#include "6502/__6502opcodes.h"
	}
	int cycleMax = inFastMode ? CYCLES_PER_FRAME*10:CYCLES_PER_FRAME; 		
	if (cycles < cycleMax) return 0;												// Not completed a frame.
	cycles = 0;																		// Reset cycle counter.
	HWSync();																		// Update any hardware
	return FRAME_RATE;																// Return frame rate.
}

// *******************************************************************************************************************************
//												Read/Write Memory
// *******************************************************************************************************************************

BYTE8 CPUReadMemory(WORD16 address) {
	return Read(address);
}

void CPUWriteMemory(WORD16 address,BYTE8 data) {
	Write(address,data);
}


#include "gfx.h"

// *******************************************************************************************************************************
//		Execute chunk of code, to either of two break points or frame-out, return non-zero frame rate on frame, breakpoint 0
// *******************************************************************************************************************************

BYTE8 CPUExecute(WORD16 breakPoint1,WORD16 breakPoint2) { 
	BYTE8 next;
	do {
		BYTE8 r = CPUExecuteInstruction();											// Execute an instruction
		if (r != 0) return r; 														// Frame out.
		next = CPUReadMemory(pc);
	} while (pc != breakPoint1 && pc != breakPoint2 && next != 0xDB);				// Stop on breakpoint or $03 break
	return 0; 
}

// *******************************************************************************************************************************
//									Return address of breakpoint for step-over, or 0 if N/A
// *******************************************************************************************************************************

WORD16 CPUGetStepOverBreakpoint(void) {
	BYTE8 opcode = CPUReadMemory(pc);												// Current opcode.
	if (opcode == 0x20) return (pc+3) & 0xFFFF;										// Step over JSR.
	return 0;																		// Do a normal single step
}

void CPUEndRun(void) {
	FILE *f = fopen("memory.dump","wb");
	fwrite(ramMemory,1,MEMSIZE,f);
	fclose(f);	
}

void CPUExit(void) {	
	printf("Exiting.\n");

	GFXExit();
}

static char chunkBuffer[4096];

static void CPULoadChunk(FILE *f,BYTE8* memory,WORD16 address,int count) {
	while (count != 0) {
		int qty = (count > 4096) ? 4096 : count;
		int n = fread(chunkBuffer,1,qty,f);
		for (int i = 0;i < n;i++) {
			int addr = MAPPING(address);
			ramMemory[addr+i] = chunkBuffer[i];
		}
		count = count - qty;
		address = address + qty;
	}
}

// *******************************************************************************************************************************
//											Retrieve a snapshot of the processor
// *******************************************************************************************************************************

static CPUSTATUS st;																	// Status area

CPUSTATUS *CPUGetStatus(void) {
	st.a = a;st.x = x;st.y = y;st.sp = s;st.pc = pc;
	st.carry = carryFlag;st.interruptDisable = interruptDisableFlag;st.zero = (zValue == 0);
	st.decimal = decimalFlag;st.brk = breakFlag;st.overflow = overflowFlag;
	st.sign = (sValue & 0x80) != 0;st.status = constructFlagRegister();
	st.cycles = cycles;
	for (int i = 0;i < 8;i++) st.mapping[i] = currentMap[i];
	return &st;
}

