; *******************************************************************************************
; *******************************************************************************************
;
;		Name : 		newmonitor.asm
;		Purpose :	Replacement monitor for tweaked UK101 Hardware (64x32 display)
;		Date :		6th July 2022
;		Author : 	Paul Robson (paul@robsons.org.uk)
;
; *******************************************************************************************
; *******************************************************************************************

zTemp0 = $FC 								; 2 byte memory units.
eventBuffer = $F0 							; location of event buffer.

ClockMhz = 6 								; clock speed in MHz (affects repeat timing)
KeyboardInvert = 1 							; 0 if keyboard active high, 1 if active low.
KQSize = 8 									; size of keyboard queue

StartWorkSpace = $200
XPosition = $203 							; X Character position
YPosition = $204 							; Y Character position
TextColour = $205 							; Text colour
CurrentPage = $206 							; current I/O page
KeysInQueue = $207 							; last key press
ReleaseFlag = $208 							; set to non-zero if release code (F0) received
ScanShiftFlag = $209 						; set to zero for shift, $80 for shifted scans ($E0)
KeyStatus = $210 							; status bits for keys, 32 x 8 bits = 256 bits.
KeyboardQueue = $210+32
EndWorkSpace = KeyboardQueue+KQSize

CWidth = 80 								; display size
CHeight = 60

IOPageRegister = 1 							; select I/O Page

	.include "include/vicky.inc"
	.include "include/interrupt.inc"

 	*= $E000

	.include "src/hardware.asm"
	.include "include/ps2convert.inc"
	.include "src/init_graphics_palettes.asm"

; ********************************************************************************
;                                        
;                                	Page Switch
;                                        
; ********************************************************************************

SelectPage0:
	pha
	lda 	IOPageRegister
	and 	#$FC
SelectPageWrite:
	sta 	IOPageRegister
	sta 	CurrentPage
	pla
	rts

SelectPage1:
	pha
	lda 	IOPageRegister
	and 	#$FC
	ora 	#1
	bra 	SelectPageWrite

SelectPage2:
	pha
	lda 	IOPageRegister
	and 	#$FC
	ora 	#2
	bra 	SelectPageWrite

SelectPage3:
	pha
	lda 	IOPageRegister
	ora 	#3
	bra 	SelectPageWrite

; ********************************************************************************
;                                        
;                                	Clear Screen
;                                        
; ********************************************************************************

ClearScreen:
	phx
	jsr 	SelectPage3
	lda 	TextColour
	jsr 	_ScreenFill
	jsr 	SelectPage2
	lda 	#$20
	jsr 	_ScreenFill
	plx
	rts

_ScreenFill:	
	pha
	lda 	#$C0 								; fill D000-D7FF with $60
	sta 	zTemp0+1
	lda 	#$00
	sta 	zTemp0
	ldy 	#0
	pla
_CLSLoop:
	sta 	(zTemp0),y
	iny
	bne 	_CLSLoop
	inc 	zTemp0+1
	ldx 	zTemp0+1
	cpx 	#$D3
	bne 	_CLSLoop
	jsr 	SelectPage0

; ********************************************************************************
;                                        
;                                	Home Cursor
;                                        
; ********************************************************************************

HomeCursor:
	lda 	#0
	sta 	xPosition
	sta 	yPosition
	jsr 	UpdateCursor
	rts

; ********************************************************************************
;
;                     	  Update Cursor position in Vicky
;
; ********************************************************************************

UpdateCursor:
	pha
	lda 	xPosition
	sta 	$D014
	lda 	yPosition
	sta 	$D016
	pla
	rts

; ********************************************************************************
;
;                     Point zTemp0 at current cursor position
;
; ********************************************************************************

SetZTemp0CharPos:
	pha
	txa
	pha
	lda 	yPosition 						; zTemp0 = yPos
	sta 	zTemp0
	lda 	#0
	sta 	zTemp0+1
	ldx 	#6 								; x 80
_SZ0Shift:
	asl 	zTemp0
	rol 	zTemp0+1
	cpx 	#5
	bne 	_SZ0NoAdd
	clc
	lda 	zTemp0
	adc 	yPosition
	sta 	zTemp0
	bcc 	_SZ0NoAdd
	inc 	zTemp0+1
_SZ0NoAdd:		
	dex
	bne 	_SZ0Shift
	clc
	lda 	zTemp0 							; add in xPos
	adc 	xPosition
	sta 	zTemp0
	lda 	zTemp0+1 						; point to page D
	adc 	#$C0
	sta 	zTemp0+1
	pla
	tax
	pla
	rts	

; ********************************************************************************
;                                        
;                                	Print A in Hex
;                                        
; ********************************************************************************

PrintHex:
	pha
	lda 	#32
	jsr 	PrintCharacter
	pla
	pha
	pha
	lsr 	a
	lsr 	a
	lsr 	a
	lsr 	a
	jsr 	PrintNibble
	pla
	jsr 	PrintNibble
	pla
	rts
PrintNibble:
	and 	#15
	cmp 	#10
	bcc 	_PN0
	adc 	#6
_PN0:	
	adc 	#48
	jmp 	PrintCharacter

; ********************************************************************************
;                                        
;                                	Print Character
;                                        
; ********************************************************************************

PrintCharacter:
	pha
	phx
	phy

	ldx 	1
	phx
	jsr 	SelectPage2

	pha
	cmp 	#8
	beq 	_PCBackspace
	cmp 	#9
	beq 	_PCTab
	cmp 	#13 						
	beq 	_PCCRLF
	jsr 	SetZTemp0CharPos 				; all other characters
	sta 	(zTemp0)
	jsr 	SelectPage3
	lda 	TextColour
	sta 	(zTemp0)
	jsr 	SelectPage2
	inc 	xPosition
	lda 	xPosition
	cmp  	#CWidth
	bne 	_PCNotRight
	stz 	xPosition
	inc 	yPosition
	lda 	yPosition
	cmp 	#CHeight
	bne 	_PCNotRight
	dec 	yPosition
	jsr 	ScrollScreenUp
_PCNotRight:
	jsr 	SelectPage0
	jsr 	UpdateCursor
	pla

	plx 
	stx 	1

	ply
	plx
	pla
	rts

_PCTab:
	lda 	#' '
	jsr 	PrintCharacter
	lda 	xPosition
	and 	#7
	bne 	_PCTab
	bra 	_PCNotRight
	
_PCBackspace:
	lda 	xPosition
	beq 	_PCNotRight
	dec 	xPosition
	jsr 	SetZTemp0CharPos
	lda 	#' '
	sta 	(zTemp0)
	bra 	_PCNotRight

_PCCRLF:									; CR/LF
	lda 	#$20 							; fill with EOL $20
	jsr 	PrintCharacter
	lda 	xPosition 						; until back at left
	bne 	_PCCRLF
	bra 	_PCNotRight

; ********************************************************************************
;                                        
;                                Ignore Interrupts
;                                        
; ********************************************************************************

NMIHandler:
		rti

; ********************************************************************************
;
;								  Scroll Screen Up
;
; ********************************************************************************

ScrollScreenUp:
	tya
	pha
	jsr 	SelectPage3
	jsr 	_ScrollBank
	lda 	TextColour
	jsr 	_WriteBottomLine
	jsr 	SelectPage2
	jsr 	_ScrollBank
	lda 	#32
	jsr 	_WriteBottomLine
	pla
	tay
	rts

_WriteBottomLine
	pha
	lda 	#$70
	sta 	zTemp0
	lda 	#$D2
	sta 	zTemp0+1
	ldy 	#CWidth-1
	pla
_ScrollBottomLine:
	sta 	(zTemp0),y
	dey
	bpl 	_ScrollBottomLine
	rts

_ScrollBank
	lda 	#$C0
	sta 	zTemp0+1
	lda 	#$00
	sta 	zTemp0
	ldy 	#CWidth
_ScrollLoop:	
	lda 	(zTemp0),y
	sta 	(zTemp0)
	inc 	zTemp0
	bne 	_ScrollLoop
	inc 	zTemp0+1
	lda 	zTemp0+1
	cmp 	#$D3
	bne 	_ScrollLoop
	rts

; ********************************************************************************
;                                        
;                    ctrl-c check, returns Z flag set on error
;                                        
; ********************************************************************************

ControlCCheck:
	lda 	KeyStatus+KP_LEFTCTRL_ROW	; check LCtrl pressed
	and 	#KP_LEFTCTRL_COL
	beq 	Exit2
	lda 	KeyStatus+KP_C_ROW 			; check C pressed
	and 	#KP_C_COL 					; non-zero if so
	eor 	#KP_C_COL 			 		; Z set if so.
	rts
Exit2:
	lda 	#$FF 						; NZ set
	rts

; ********************************************************************************
;
;					Handle streams of keyboard data from IRQ
;
; ********************************************************************************

HandleKeyboard:
		pha
		phx
		phy

		cmp 	#$80 							; E0 (Scan shift)
		bcc 	_KeyBit 						; if 00-7F then it is a keystroke.
		cmp 	#$E0
		beq 	_HKIsShift 						; if = $E0 then it is a shifted key on the keyboard.
		cmp 	#$F0 							; if = $F0 then it is a release.
		bne 	_HKExit
		sta 	ReleaseFlag 					; set release flag to non-zero.
		bra 	_HKExit
_HKIsShift:
		lda 	#$80 							; set scan shift flag to $80		
		sta 	ScanShiftFlag 
		bra 	_HKExit

_KeyBit:
		ora 	ScanShiftFlag 					; A now contains the scan shift flag.
		sta 	ScanShiftFlag 					; store result there
		;
		;		Set/clear bit in the KeyStatus area
		;
		and 	#$7F
		lsr 	a 								; divide by 8 -> X, offset in table
		lsr 	a
		lsr 	a
		tax

		lda 	ScanShiftFlag 					; get the key press back.
		and 	#7 								; count in Y
		tay
		lda 	#0
		sec
_HKGetBits:		
		rol 	a
		dey
		bpl 	_HKGetBits
		ldy 	ReleaseFlag 					; is the release flag set
		bne 	_HKRelease
		ora 	KeyStatus,x  					; set bit
		bra 	_HKWrite
_HKRelease:
		eor 	#$FF 							; clear bit
		and 	KeyStatus,x
_HKWrite:
		sta 	KeyStatus,x
		;
		;		Process key if appropriate
		;
		lda 	ScanShiftFlag 					; restore new code
		ldx 	ReleaseFlag
		bne 	_HKNotInsert 					; process key down
		jsr 	ConvertInsertKey
_HKNotInsert:
		stz 	ReleaseFlag 					; zero both flags.
		stz 	ScanShiftFlag		
_HKExit:				
		ply
		plx
		pla
		rts

; ********************************************************************************
;                                        
;			Key code A has been pressed, convert to ASCII, put into buffer
;                                        
; ********************************************************************************

ConvertInsertKey:
		tax 								; scan code in X
		lda 	ASCIIFromScanCode,x 		; get ASCII unshifted 
		beq 	_CIKExit 					; key not known

		tay 								; save in Y
		bmi 	_CIKEndShiftCheck 			; if bit 7 was set shift doesn't affect this.
		lda 	KeyStatus+KP_LEFTSHIFT_ROW	; check left shift
		and 	#KP_LEFTSHIFT_COL
		bne 	_CIKShift
		lda 	KeyStatus+KP_RIGHTSHIFT_ROW	; check right shift
		and 	#KP_RIGHTSHIFT_COL
		beq 	_CIKEndShiftCheck
_CIKShift:
		ldx 	#254 						; check shift table.
_CIKShiftNext:		
		inx
		inx
		bit  	ShiftFixTable,x 			; end of table ?
		bmi 	_CIDefaultShift
		tya 								; found a match ?
		cmp 	ShiftFixTable,x
		bne 	_CIKShiftNext
		ldy 	ShiftFixTable+1,x 			; get replacement
		bra 	_CIKEndShiftCheck

_CIDefaultShift:							; don't shift control
		cmp 	#32
		bcc 	_CIKEndShiftCheck
		tya 								; default shift.
		eor 	#32
		tay		
_CIKEndShiftCheck: 							
		lda 	KeyStatus+KP_LEFTCTRL_ROW	; check LCtrl pressed
		and 	#KP_LEFTCTRL_COL
		beq 	_CIKNotControl
		tya 								; lower 5 bits only on control.
		and 	#31
		tay 								
_CIKNotControl:		
		tya 	
_CIKExit:		
		ldy 	KeysInQueue 				; space in queue ?
		cpy 	#KQSize
		beq 	_CIKQueueFull
		sta 	KeyboardQueue,y 			; write to queue.
		inc 	KeysInQueue
_CIKQueueFull:		
		rts

; ********************************************************************************
;                                        
;							New Read Keyboard routine
;                                        
; ********************************************************************************

NewReadKeyboard:
		jsr 	GetKeyIfPressed
		beq 	NewReadKeyboard
		rts

; ********************************************************************************
;                                        
;							Fake Screen Editing
;                                        
; ********************************************************************************

FakeKeyboardRead:
		jsr 	NewReadKeyboard 			; echo everything except CR, makes 
		cmp 	#13 						; it behave like the C64 with it's
		beq 	_FKRExit 					; line editing
		jsr 	PrintCharacter
_FKRExit:
		rts		

; ********************************************************************************
;                                        
;								Get key if available
;                                        
; ********************************************************************************

GetKeyIfPressed:
		lda 	KeysInQueue 				; anything in queue
		beq 	_GIKExit 					; if not, exit with A = 0, Z set
		lda 	KeyboardQueue 				; get and push front of queue
		pha
		phx
		ldx 	#0 							; remove from queue
_GIKPop:
		lda 	KeyboardQueue+1,x
		sta 	KeyboardQueue,x
		inx
		cpx 	#KQSize
		bne 	_GIKPop
		dec 	KeysInQueue 				; one fewer in queue
		plx
		pla 								; restore front of queue setting Z
_GIKExit:
		rts
		
; ********************************************************************************
;                                        
;									System startup
;                                        
; ********************************************************************************

SystemReset:
	ldx		#$FF
	txs
	sei

	lda 	#$80 								; access current LUT
	sta 	$00
	ldy 	15 									; get monitor page

	lda 	#$80+$30+$00 						; LUT 3 , Edit 3, Active 0
 	sta 	$00

	ldx 	#5 									; map all to memory.
_InitMMU3:
	txa
	sta 	8,x
	dex
	bpl 	_InitMMU3

	lda 	#BASIC_ADDRESS >> 13 				; map BASIC ROM into slots 4 & 5, consecutive pages
	sta 	12
	inc 	a
	sta 	13
	lda 	#6 									; theoretically ; owned by Kernal.
	sta 	14
	sty 	15 									; copy monitor page.
	lda 	#$80+$30+$03 						; LUT 3 , Edit 3, Active 3
	sta 	$00

	ldx 	#EndWorkSpace-StartWorkSpace
_SRClear:
	stz 	StartWorkSpace-1,x
	dex
	cpx 	#$FF
	bne 	_SRClear	
	jsr 	SelectPage0

    LDA #$FF
    ; Setup the EDGE Trigger 
    STA INT_EDGE_REG0
    STA INT_EDGE_REG1
    ; Mask all Interrupt @ This Point
    STA INT_MASK_REG0
    STA INT_MASK_REG1
    ; Clear both pending interrupt
    lda INT_PENDING_REG0
    sta INT_PENDING_REG0
    lda INT_PENDING_REG1
    sta INT_PENDING_REG1     

    stz 	$01
    lda 	$D670
    pha

	jsr 	TinyVickyInitialise
	jsr 	Init_Text_LUT
	jsr 	LoadGraphicsLUT
	jsr 	ClearScreen
	inc 	yPosition
	inc 	yPosition

	pla
	jsr 	PrintHex
	lda 	#13
	jsr 	PrintCharacter
	
	ldx 	#8
_ShowMMU:
	lda 	0,x
	jsr 	PrintHex
	lda 	#32
	jsr 	PrintCharacter
	inx
	cpx		#16
	bne 	_ShowMMU
	lda 	#13
	jsr 	PrintCharacter

	ldx 	#0
	lda 	15
	cmp 	#$3F
	bne 	_NotRAM
	ldx 	#Prompt2-Prompt1
_NotRAM:
	jsr 	PrintMsg
	ldx 	#Prompt3-Prompt1
	jsr 	PrintMsg

    lda #200
    sta VKY_LINE_CMP_VALUE_LO
    lda #0
    sta VKY_LINE_CMP_VALUE_HI
    lda #$01 
    sta VKY_LINE_IRQ_CTRL_REG

    SEI
    lda INT_PENDING_REG0  ; Read the Pending Register &
    and #JR0_INT01_SOL
    sta INT_PENDING_REG0  ; Writing it back will clear the Active Bit
    lda INT_MASK_REG0
    and #~JR0_INT01_SOL
    sta INT_MASK_REG0

    lda INT_PENDING_REG0  ; Read the Pending Register &
    and #JR0_INT02_KBD
    sta INT_PENDING_REG0  ; Writing it back will clear the Active Bit
    ; remove the mask
    lda INT_MASK_REG0
    and #~JR0_INT02_KBD
    sta INT_MASK_REG0                

	;
	jsr 	SelectPage0
	lda 	#1
	sta 	$D100
	stz 	$D101
	stz 	$D102
	stz 	$D103
	;
	inc 	$700
	lda 	$700
	and 	#15
	ora 	#64
	jsr 	$FFD2
	jsr 	$FFD2
	jsr 	$FFD2
	;
	jsr 	init_text_palette
	jsr 	init_graphics_palettes


	lda 	#42
	jsr 	$FFD2

	jsr 	InitStefanyPS2
	cli
	;
	;		Uncommenting this puts the keyboard into an echo loop.
	;
	;jmp 	EchoLoop

	jsr 	RunProgram
Halt:
	bra 	Halt	

Prompt1:
	.text 	13,"Running in RAM",13,0
Prompt2:
	.text 	13,"Running from Flash",13,0
Prompt3:
	.text 	13,"Requires new PS/2 Interface : B or 13th Dec FPGA or later",13,13,0

RunProgram:	
	jmp 	($FFF8)

PrintMsg:
	lda 	Prompt1,x
	inx
	jsr 	PrintCharacter		
	cmp 	#0
	bne 	PrintMsg
	rts

; ********************************************************************************
;
;							Echo Scan codes Stefany version
;
; ********************************************************************************

InitStefanyPS2:
		stz 	$01
		lda 	#$30 						; should reset the hardware
		sta 	$D640
		stz 	$D640
		rts

EchoScanCodesStefany:
_ESCSLoop:
		lda 	$D644 						; wait for FIFO not to be empty
		and 	#1
		bne 	_ESCSLoop
		lda 	$D642 						; read it in.
		jsr 	PrintHex
		bra 	_ESCSLoop

; ********************************************************************************
;
;									Echo code
;
; ********************************************************************************

EchoLoop:	
	jsr 	GetKeyIfPressed
	cmp 	#0
	beq 	EchoLoop	
	jsr 	PrintHex
	jsr 	PrintCharacter
	jmp 	EchoLoop

; ********************************************************************************
;
;					Read row A of the keyboard status table
;	
; ********************************************************************************

ReadKeyboardStatusTable:
	phx
	tax
	lda 	KeyStatus,x
	plx
	rts

init_text_palette
			stz 	1
            ldx     #0
_loop       lda     _palette,x
            sta     TEXT_LUT_FG,x
            sta     TEXT_LUT_BG,x
            inx
            cpx     #64
            bne     _loop
            rts
_palette
            .dword  $000000
            .dword  $ffffff
            .dword  $880000
            .dword  $aaffee
            .dword  $cc44cc
            .dword  $00cc55
            .dword  $0000aa
            .dword  $dddd77
            .dword  $dd8855
            .dword  $664400
            .dword  $ff7777
            .dword  $333333
            .dword  $777777
            .dword  $aaff66
            .dword  $0088ff
            .dword  $bbbbbb

; ********************************************************************************
;
;						Get next event (keyboard, ASCII only)
;
; ********************************************************************************

GetNextEvent:
		jsr 	GetKeyIfPressed 				; is a key pressed
		cmp 	#0
		sec 	
		beq 	_GNEExit 						; if not, return with carry set.
		phy
		ldy 	#4 								; write to event.key.raw
		sta 	(eventBuffer),y
		iny 									; write to event.key.ascii
		sta 	(eventBuffer),y 	
		lda 	#0
		iny 
		sta 	(eventBuffer),y 				; write zero to flags.
		ply
		lda 	#8 								; event type - this is key pressed.
		sta 	(eventBuffer)
		clc 									; event occurred.
_GNEExit:
		rts		

; ********************************************************************************
;
;							 Commodore Compatible Vectors
;
; ********************************************************************************

	* = $FF00 									; get next event
	jmp 	GetNextEvent

	* = $FFCF 									; CHRIN
Disable1:
	jmp 	NewReadKeyboard
	* = $FFD2 									; CHROUT
	jmp 	PrintCharacter
	* = $FFE1
	jmp 	ControlCCheck
	* = $FFE4
	jmp 	GetKeyIfPressed
	* = $FFE7
	jmp 	ReadKeyboardStatusTable
	* = $FFEA 									; Clear Screen
	jmp	 	ClearScreen
	
; ********************************************************************************
;
;									6502 Vectors
;
; ********************************************************************************

	* =	$FFF8

	.word 	$8040
	.word 	NMIHandler                       	; nmi ($FFFA)
	.word 	SystemReset                         ; reset ($FFFC)
	.word 	IRQHandler                          ; irq ($FFFE)

	.end 

