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
static BYTE8 *mapping;
static BYTE8 writeProtect;
static BYTE8 isPageCMemory; 														// Is Page $C000-$DFFF memory.
static int argumentCount;
static char **argumentList;
static LONG32 cycles;																// Cycle Count.
static BYTE8 inFastMode; 															// Fast mode

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

#define MAPPING(a)  ((mapping[(a) >> 13] << 13) | ((a) & 0x1FFF)) 					// Map address through mapping table.

BYTE8 *CPUAccessMemory(void) {
	return ramMemory;
}

static inline BYTE8 _Read(WORD16 address) {
	if (address == 0xFFFA) return IOReadSource(); 									// Cheat reading from 'disk'

	if (isPageCMemory == 0 && address >= 0xC000 && address < 0xE000) { 				// Hardware check
		return IOReadMemory(ramMemory[1] & 3,address);
	} 
	int a = MAPPING(address);
	return ramMemory[a];
}

static inline void _Write(WORD16 address,BYTE8 data) { 
	if (address == 0xFFFA) inFastMode = data; 										// Switch fast off/on
	
	if (address < 0x10) {						
		ramMemory[address] = data;													// Save it.
		if (address == 1) isPageCMemory = ((ramMemory[1] & 4) != 0);
		if (address >= 0x08 && address < 0x10) { 									// Set up mapping.
			mapping[address-0x08] = data;
		}
		return;
	}
	if (isPageCMemory == 0 && address >= 0xC000 && address < 0xE000) {				// Hardware check.
		IOWriteMemory(ramMemory[1] & 3,address,data);
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
	printf("%d\n",argumentCount);
}

// *******************************************************************************************************************************
//														Reset the CPU
// *******************************************************************************************************************************

#include "roms/monitor_rom.h"
#include "roms/character_rom.h"

static void CPULoadChunk(FILE *f,BYTE8* memory,WORD16 address,int count);

void CPUReset(void) {
	for (int i = 0x10000;i < MEMSIZE;i++) {
		ramMemory[i] = rand();
	}
	mapping = ramMemory+0x08; 														// Default mapping (through LUT0)
	writeProtect = 0;
	for (int i = 0;i < 8;i++) { 													// Map to first pages.
		mapping[i] = i;
		ramMemory[i+8] = mapping[i];
	}
	for (int i = 0;i < 8*256;i++) {
		IOWriteMemory(1,i+0xC000,character_rom[i]);
	}

	isPageCMemory = ((ramMemory[1] & 4) != 0);										// Set PageC RAM flag.

	for (int i = 0;i < 4096;i++) Write(0xF000+i,monitor_rom[i]);					// Copy ROM images in

	HWReset();																		// Reset Hardware

	#ifdef EMSCRIPTEN  																// Loading in stuff alternative for emScripten
	#include "loaders.h" 															// Partly because preload storage does not work :(
	#endif

	for (int i = 1;i < argumentCount;i++) {
		char szBuffer[128];
		int loadAddress,startAddress;
		strcpy(szBuffer,argumentList[i]);											// Get buffer
		char *p = strchr(szBuffer,'@');
		if (p == NULL) exit(fprintf(stderr,"Bad argument %s\n",argumentList[i]));
		*p++ = '\0';
		if (sscanf(p,"%x",&loadAddress) != 1) exit(fprintf(stderr,"Bad argument %s\n",argumentList[i]));
		startAddress = loadAddress;
		ramMemory[0xFE] = loadAddress & 0xFF;
		ramMemory[0xFF] = loadAddress >> 8;
		FILE *f = fopen(szBuffer,"rb");
		if (f == NULL) exit(fprintf(stderr,"No file %s\n",argumentList[i]));
		while (!feof(f)) {
			if (loadAddress < MEMSIZE) {
				ramMemory[loadAddress++] = fgetc(f);
			}
		}
		fclose(f);
		printf("Loaded '%s' to $%04x..$%04x\n",szBuffer,startAddress,loadAddress-1);
	}

	inFastMode = 0;																	// Fast mode flag reset

	writeProtect = -1;
	resetProcessor();																// Reset CPU
}

void CPUInterruptMaskable(void) {
	irqCode();
}
// *******************************************************************************************************************************
//												Execute a single instruction
// *******************************************************************************************************************************

BYTE8 CPUExecuteInstruction(void) {
	if (pc == 0xFFFF) {
		CPUExit();
		return FRAME_RATE;
	}
	BYTE8 opcode = Fetch();															// Fetch opcode.
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
	for (int i = 0;i < 8;i++) st.mapping[i] = mapping[i];
	return &st;
}

