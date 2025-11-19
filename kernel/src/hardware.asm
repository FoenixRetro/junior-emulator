; *******************************************************************************************
; *******************************************************************************************
;
;       Name :      hardware.asm
;       Purpose :   TinyVicky hardware interface
;       Date :      19th November 2025
;       Author :    Paul Robson (paul@robsons.org.uk)
;
; *******************************************************************************************
; *******************************************************************************************

; ****************************************************************************************
;
;                          Display some text on Page 2 Text RAM
;
; ****************************************************************************************

TinyVickyInitialise:
            lda # Mstr_Ctrl_Text_Mode_En        ; Text on, Overlay,Graphic,Bitmap,Tilemap,Sprite,Gamma,Disable off.
            sta MASTER_CTRL_REG_L               ; Register $D000 in page 0
            ;
            lda MASTER_CTRL_REG_L           
            lda #Border_Ctrl_Enable             ; Enable border
            sta BORDER_CTRL_REG                 ; Register $D004 in page 0
            ;
            lda #$80                            ; set border colour to $804000
            sta BORDER_COLOR_B                  ; (registers $D005..$D007)
            lda #$00
            sta BORDER_COLOR_G           
            lda #$00
            sta BORDER_COLOR_R

            lda #16                             ; border size (offset from top left ?)
            sta BORDER_X_SIZE                   ; (registers $D008 .. $D009)
            sta BORDER_Y_SIZE
                                                ; Cursor on, Rate 1, no idea what that is :)
            lda #Vky_Cursor_Enable | Vky_Cursor_Flash_Rate1  
            sta VKY_TXT_CURSOR_CTRL_REG         ; turn cursor on ($D010)
            lda #6
            sta VKY_TXT_CURSOR_CHAR_REG         ; 160 is 128+32 so inverse space. ($D012)
            lda #28
            sta VKY_TXT_CURSOR_COLR_REG         ; colour $2 $8 ($D013)

            lda #0                              ; cursor to (0,5)
            sta VKY_TXT_CURSOR_X_REG_L          ; ($D014 .. $D017)
            sta VKY_TXT_CURSOR_X_REG_H
            sta VKY_TXT_CURSOR_Y_REG_H
            lda #5
            sta VKY_TXT_CURSOR_Y_REG_L

            lda		#$16   						; set text colour.
            sta 	TextColour

            rts

; ****************************************************************************************
;
;                             Copy Text colour Data 
;
; ****************************************************************************************

Init_Text_LUT
                LDX #$00
                ;
                ;       64 bytes (16x4) foreground data
                ;
lutinitloop0    LDA fg_color_lut,x      ; get Local Data
                sta TEXT_LUT_FG,x   ; Write in LUT Memory ($D800)
                inx
                cpx #$40
                bne lutinitloop0
                ;
                ;       64 bytes (16x4) background data
                ;
                LDX #$00
lutinitloop1    LDA bg_color_lut,x      ; get Local Data
                STA TEXT_LUT_BG,x   ; Write in LUT Memory ($D840)
                INX
                CPX #$40
                bne lutinitloop1
                RTS

; ****************************************************************************************
;
;                             Colour BGRA for Text
;
; ****************************************************************************************

fg_color_lut    .text $00, $00, $00, $FF
                .text $00, $00, $80, $FF        ; blue
                .text $00, $80, $00, $FF        ; green
                .text $80, $00, $00, $FF        ; red
                .text $00, $80, $80, $FF        ; yellow (etc.)
                .text $80, $80, $00, $FF
                .text $80, $00, $80, $FF
                .text $80, $80, $80, $FF
                .text $00, $45, $FF, $FF
                .text $13, $45, $8B, $FF
                .text $00, $00, $20, $FF
                .text $00, $20, $00, $FF
                .text $20, $00, $00, $FF
                .text $20, $20, $20, $FF
                .text $FF, $80, $00, $FF
                .text $FF, $FF, $FF, $FF

bg_color_lut    .text $00, $00, $00, $FF  ;BGRA
                .text $AA, $00, $00, $FF
                .text $00, $80, $00, $FF
                .text $00, $00, $80, $FF
                .text $00, $20, $20, $FF
                .text $20, $20, $00, $FF
                .text $20, $00, $20, $FF
                .text $20, $20, $20, $FF
                .text $1E, $69, $D2, $FF
                .text $13, $45, $8B, $FF
                .text $00, $00, $20, $FF
                .text $00, $20, $00, $FF
                .text $40, $00, $00, $FF
                .text $10, $10, $10, $FF
                .text $40, $40, $40, $FF
                .text $FF, $FF, $FF, $FF

; ****************************************************************************************
;
;                               Interrupt Handler (Non Maskable)
;
; ****************************************************************************************

IRQHandler      
                pha

                lda     1
                pha
                and     #$F8
                sta     1

                LDA INT_PENDING_REG0                ; received Keyboard interrupt ?
                AND #JR0_INT02_KBD
                CMP #JR0_INT02_KBD
                BNE EXIT_IRQ_HANDLE
                ; clear pending

                lda     $D644                       ; shouldn't be empty
                and     #1
                bne     EXIT_IRQ_HANDLE

;                lda     #">"
;               jsr     PrintCharacter
                lda     $D642            
                jsr     HandleKeyboard
;                jsr     PrintHex


EXIT_IRQ_HANDLE:
                lda INT_PENDING_REG0
                sta INT_PENDING_REG0
                lda INT_PENDING_REG1
                sta INT_PENDING_REG1                      

                pla
                sta     1

                pla
                rti


; ****************************************************************************************
;
;                               Upload Graphics LUT
;
; ****************************************************************************************

LoadGraphicsLUT:
            jsr     SelectPage1
            ldx     #0
_LGLLoop:   lda     _GraphicsLUT,x
            sta     TyVKY_LUT0,x            
            lda     _GraphicsLUT+256,x
            sta     TyVKY_LUT0+256,x            
            lda     _GraphicsLUT+512,x
            sta     TyVKY_LUT0+512,x            
            lda     _GraphicsLUT+768,x
            sta     TyVKY_LUT0+768,x            
            dex
            bne     _LGLLoop
            rts
            
_GraphicsLUT:
            .binary    "gfxlut.palette"
            