// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		sys_debug_uk101.c
//		Purpose:	Debugger Code (System Dependent)
//		Created:	29th June 2022
//		Author:		Paul Robson (paul@robsons->org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gfx.h"
#include "sys_processor.h"
#include "debugger.h"
#include "hardware.h"

#include "6502/__6502mnemonics.h"

#define DBGC_ADDRESS 	(0x0F0)														// Colour scheme.
#define DBGC_DATA 		(0x0FF)														// (Background is in main.c)
#define DBGC_HIGHLIGHT 	(0xFF0)

static int renderCount = 0;

static void DGBXRenderBitmap(int base,int x1,int y1,int xs,int ys);
static void DGBXRenderSprite(int addr,int x1,int y1,int xs,int ys);

// *******************************************************************************************************************************
//											This renders the debug screen
// *******************************************************************************************************************************

static const char *labels[] = { "A","X","Y","PC","SP","SR","CY","N","V","B","D","I","Z","C", NULL };

int DBGXGetColour(int page,int base,int colour) {
	base = base + colour * 4;
	int blue = IOReadMemory(page,base+0) >> 4;
	int green = IOReadMemory(page,base+1) >> 4;
	int red = IOReadMemory(page,base+2) >> 4;
	return (red << 8) | (green << 4) | blue;
}

void DBGXRender(int *address,int showDisplay) {

	int n = 0;
	char buffer[32];
	CPUSTATUS *s = CPUGetStatus();

	#ifndef EMSCRIPTEN

	GFXSetCharacterSize(36,24);
	DBGVerticalLabel(21,0,labels,DBGC_ADDRESS,-1);									// Draw the labels for the register

	GFXString(GRID(30,0),"MMU",GRIDSIZE,DBGC_ADDRESS,-1);
	GFXString(GRID(30,1),"IO",GRIDSIZE,DBGC_ADDRESS,-1);
	GFXNumber(GRID(34,0),CPUReadMemory(0),16,2,GRIDSIZE,DBGC_DATA,-1);
	int io = CPUReadMemory(1);
	if (io & 4) {
		GFXString(GRID(30,1),"Mem",GRIDSIZE,DBGC_DATA,-1);
	} else {
		GFXNumber(GRID(34,1),io & 3,16,1,GRIDSIZE,DBGC_DATA,-1);
	}
	for (int i = 0;i < 8;i++) {
		GFXNumber(GRID(30,i+2),i,16,1,GRIDSIZE,DBGC_ADDRESS,-1);
		GFXNumber(GRID(34,i+2),s->mapping[i] << 13,16,5,GRIDSIZE,DBGC_DATA,-1);
	}

	#define DN(v,w) GFXNumber(GRID(24,n++),v,16,w,GRIDSIZE,DBGC_DATA,-1)			// Helper macro

	DN(s->a,2);DN(s->x,2);DN(s->y,2);DN(s->pc,4);DN(s->sp+0x100,4);DN(s->status,2);DN(s->cycles,4);
	DN(s->sign,1);DN(s->overflow,1);DN(s->brk,1);DN(s->decimal,1);DN(s->interruptDisable,1);DN(s->zero,1);DN(s->carry,1);

	n = 0;
	int a = address[1];																// Dump Memory.
	for (int row = 15;row < 23;row++) {
		GFXNumber(GRID(0,row),a,16,4,GRIDSIZE,DBGC_ADDRESS,-1);
		for (int col = 0;col < 8;col++) {
			int c = CPUReadMemory(a);
			GFXNumber(GRID(5+col*3,row),c,16,2,GRIDSIZE,DBGC_DATA,-1);
			c = (c & 0x7F);if (c < ' ') c = '.';
			GFXCharacter(GRID(30+col,row),c,GRIDSIZE,DBGC_DATA,-1);
			a = (a + 1) & 0xFFFF;
		}		
	}

	int p = address[0];																// Dump program code. 
	int opc;

	for (int row = 0;row < 14;row++) {
		int isPC = (p == ((s->pc) & 0xFFFF));										// Tests.
		int isBrk = (p == address[3]);
		GFXNumber(GRID(0,row),p,16,4,GRIDSIZE,isPC ? DBGC_HIGHLIGHT:DBGC_ADDRESS,	// Display address / highlight / breakpoint
																	isBrk ? 0xF00 : -1);
		opc = CPUReadMemory(p);p = (p + 1) & 0xFFFF;								// Read opcode.
		strcpy(buffer,_mnemonics[opc]);												// Work out the opcode.
		char *at = strchr(buffer,'@');												// Look for '@'
		if (at != NULL) {															// Operand ?
			char hex[6],temp[32];	
			if (at[1] == '1') {
				sprintf(hex,"%02x",CPUReadMemory(p));
				p = (p+1) & 0xFFFF;
			}
			if (at[1] == '2') {
				sprintf(hex,"%02x%02x",CPUReadMemory(p+1),CPUReadMemory(p));
				p = (p+2) & 0xFFFF;
			}
			if (at[1] == 'r') {
				int addr = CPUReadMemory(p);
				p = (p+1) & 0xFFFF;
				if ((addr & 0x80) != 0) addr = addr-256;
				sprintf(hex,"%04x",addr+p);
			}
			strcpy(temp,buffer);
			strcpy(temp+(at-buffer),hex);
			strcat(temp,at+2);
			strcpy(buffer,temp);
		}
		GFXString(GRID(5,row),buffer,GRIDSIZE,isPC ? DBGC_HIGHLIGHT:DBGC_DATA,-1);	// Print the mnemonic
	}

	#endif 

	int xs = 80;
	int ys = 60;
	int xCursor = -1,yCursor = -1;
	renderCount++;
	if (showDisplay != 0) {
		int xSize = 2;
		int ySize = 2;
		int x1 = WIN_WIDTH/2-xs*xSize*8/2;
		int y1 = WIN_HEIGHT/2-ys*ySize*8/2;
		int cursorPos = 0;
		int ctrl = IOReadMemory(0,0xD000);
		SDL_Rect r;
		//
		//		Do border
		//
		r.x = r.y = 0;r.w = WIN_WIDTH;r.h = WIN_HEIGHT;
		GFXRectangle(&r,DBGXGetColour(0,0xD005,0));
		//
		//		Do background as graphics background colour / black
		//
		r.x = x1;r.y = y1;r.w = xs*xSize*8;r.h=ys*ySize*8;
		GFXRectangle(&r,(ctrl & 4) ? DBGXGetColour(0,0xD00D,0) : 0);
		//
		//		Bitmaps if Bitmap & Graphic on.
		//
		if ((ctrl & 0x0C) == 0x0C) {
			if (IOReadMemory(0,0xD100) & 1) DGBXRenderBitmap(0xD100,x1,y1,xSize*2,ySize*2);
			if (IOReadMemory(0,0xD108) & 1) DGBXRenderBitmap(0xD108,x1,y1,xSize*2,ySize*2);
		}
		//
		//		Draw sprites
		//
		if ((ctrl & 0x20) == 0x20) {
			for (int s = 0;s < 64;s++) {
				if (IOReadMemory(0,0xD900+s*8) & 0x01) {
					DGBXRenderSprite(0xD900+s*8,x1,y1,xSize*2,ySize*2);
				}
			}		
		}
		//
		//		Draw text mode, possibly overlaid.
		//
		int height = (IOReadMemory(0,0xD001) & 1) ? 50 : 60;
		if (ctrl & 1) {
			if ((IOReadMemory(0,0xD010) & 1) != 0) {
				xCursor = IOReadMemory(0,0xD014);
				yCursor = IOReadMemory(0,0xD016);
			}
			for (int x = 0;x < xs;x++) 
			{
				for (int y = 0;y < height;y++)
			 	{
			 		int ch = IOReadMemory(2,0xC000+x+y*xs);
			 		int tc = IOReadMemory(3,0xC000+x+y*xs);
			 		int xc = x1 + x * 8 * xSize;
			 		int yc = y1 + y * 8 * ySize;
			 		SDL_Rect rc;
			 		int isCursor = 0;
			 		if (xCursor == x && yCursor == y && (renderCount & 0x20) == 0) { 				// At cursor.
			 			ch = IOReadMemory(0,0xD012);
			 			tc = IOReadMemory(0,0xD013);
			 			isCursor = -1;
			 		}
			 		int col1 = DBGXGetColour(0,0xD800,tc >> 4); 									// Get Text Colour
			 		int col2 = DBGXGetColour(0,0xD840,tc & 0x0F); 									// Get Background Colour
			 		r.x = xc;r.y = yc;r.w = xSize * 8;r.h = ySize * 8;
			 		if ((ctrl & 2) == 0 || isCursor != 0) GFXRectangle(&r,col2); 					// If not overlay or cursor, paint background.
			 		int cp = ch * 8;
			 		rc.w = xSize;rc.h = ySize;														// Width and Height of pixel.
			 		for (int x = 0;x < 8;x++) {														// 8 Across
			 			rc.x = xc + x * xSize;
			 			for (int y = 0;y < 8;y++) {													// 8 Down
			 				int f = IOReadMemory(1,0xC000+cp+y);
			 				rc.y = yc + y * ySize;
			 				if (f & (0x80 >> x)) GFXRectangle(&rc,col1);			
			 			}
			 		}
			 	}
			}
		}

	}
}	

// *******************************************************************************************************************************
//
//													Render one bitmap
//
// *******************************************************************************************************************************

static void DGBXRenderBitmap(int base,int x1,int y1,int xs,int ys) {
	int height = (IOReadMemory(0,0xD001) & 1) ? 200 : 240;
	int address = IOReadMemory(0,base+1)+(IOReadMemory(0,base+2) << 8)+(IOReadMemory(0,base+3) << 16);
	address &= 0x3FFFF;																			// 22 bit bitmap addres
	int lut = ((IOReadMemory(0,base) >> 1) & 3) * 0x400+0xD000; 								// Graphics LUT in page 1.
	BYTE8 *bitmap = CPUAccessMemory()+address;
	int colourCache[256]; 																		// Cache of converted colours.
	for (int i = 0;i < 256;i++) colourCache[i] = -1;

	for (int y = 0;y < height;y++) {
		SDL_Rect rc;rc.x = x1;rc.y = y1 + y * ys;rc.w = xs;rc.h = ys;
		for (int x = 0;x < 320;x++) {
			int colour = *bitmap++;
			//if (x == 0 && y == 0) printf("B:%d %d\n",colour,lut);
			if (colourCache[colour] < 0) colourCache[colour] = DBGXGetColour(1,lut,colour);
			if (colour != 0) {
				GFXRectangle(&rc,colourCache[colour]);
			}
			rc.x += rc.w;
		}
	}
}

static void DGBXRenderSprite(int addr,int x1,int y1,int xs,int ys) {
	int sprGraphic = IOReadMemory(0,addr+1)+													// Sprite address
							(IOReadMemory(0,addr+2) << 8)+(IOReadMemory(0,addr+3) << 16);
	int size = 8*(4-((IOReadMemory(0,addr) >> 5) & 3));											// Size
	int lut = ((IOReadMemory(0,addr) >> 2) & 3) * 0x400+0xD000; 								// Graphics LUT in page 1.
	int xPos = IOReadMemory(0,addr+4)+(IOReadMemory(0,addr+5) << 8);							// Position
	int yPos = IOReadMemory(0,addr+6)+(IOReadMemory(0,addr+7) << 8);							
	//printf("SP:%x %x %d %d %x %x\n",addr,sprGraphic,size,lut,xPos,yPos);
	BYTE8 *bitmap = CPUAccessMemory()+sprGraphic;
	int colourCache[256]; 																		// Cache of converted colours.
	for (int i = 0;i < 256;i++) colourCache[i] = -1;

	for (int y = 0;y < size;y++) {
		SDL_Rect rc;rc.x = x1+(xPos-32)*xs;rc.y = y1+(yPos+y-32) * ys;rc.w = xs;rc.h = ys;
		for (int x = 0;x < size;x++) {
			int colour = *bitmap++;
			//if (x == 0 && y == 0) printf("S:%d %d\n",colour,lut);
			if (colourCache[colour] < 0) colourCache[colour] = DBGXGetColour(1,lut,colour);
			if (colour != 0) {
				GFXRectangle(&rc,colourCache[colour]);
			}
			rc.x += rc.w;			
		}
	}
}