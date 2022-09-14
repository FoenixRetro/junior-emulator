
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

            lda		#$EC 						; set text colour.
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

;Keyboard $D640 - $D64F
STATUS_PORT     = $D644;
KBD_CMD_BUF     = $D644;
KBD_OUT_BUF     = $D640;
KBD_INPT_BUF    = $D640;
KBD_DATA_BUF    = $D640;

OUT_BUF_FULL    = $01
INPT_BUF_FULL   = $02
SYS_FLAG        = $04
CMD_DATA        = $08
KEYBD_INH       = $10
TRANS_TMOUT     = $20
RCV_TMOUT       = $40
PARITY_EVEN     = $80
INH_KEYBOARD    = $10
KBD_ENA         = $AE
KBD_DIS         = $AD

; ****************************************************************************************
;
;                          Wait till input buffer empty
;
; ****************************************************************************************

Poll_Inbuf      lda STATUS_PORT     ; Load Status Byte
                and #INPT_BUF_FULL  ; Test bit $02 (if 0, Empty)
                cmp #INPT_BUF_FULL
                beq Poll_Inbuf
                rts

; ****************************************************************************************
;
;                          Wait till output buffer empty
;
; ****************************************************************************************

Poll_Outbuf     lda STATUS_PORT
                and #OUT_BUF_FULL ; Test bit $01 (if 1, Full)
                cmp #OUT_BUF_FULL
                bne Poll_Outbuf
                rts

; ****************************************************************************************
;
;                          Initialise the keyboard
;
; ****************************************************************************************

INITKEYBOARD    clc
                ; Test AA
                lda #$AA                    ; Send self test command
                sta KBD_CMD_BUF
                                
                jsr Poll_Outbuf             ; Sent Self-Test Code and Waiting for Return value, it ought to be 0x55.

                lda KBD_OUT_BUF             ; Check self test result
                cmp #$55
                beq passAAtest

                jmp initkb_loop_out
                ;
                ;       passed the initial test
                ;
passAAtest      jsr SelectPage2              ; put chr$(31) "1" on top left of screen
                lda #$31
                sta $C000 
                jsr SelectPage0
                ;; Test AB
                ;
                ;       respond to the AB command    
                ;
                jsr Poll_Inbuf
                lda #$AB                     ;Send test Interface command
                sta KBD_CMD_BUF
                jsr Poll_Outbuf ;
TryAgainAB:
                lda KBD_OUT_BUF               ;Display Interface test results
                cmp #$00                      ;Should be 00
                beq passABtest
                bne TryAgainAB
                ;
                jsr SelectPage2                 ; not reachable
                lda #$23
                sta $C005
                jsr SelectPage0
                jmp initkb_loop_out
                ;
                ;       passed the AB command
                ;
passABtest      jsr SelectPage2                  ; put "2" on top left, 2nd character
                lda #$32
                sta $C001
                jsr SelectPage0
                ;
                ;       Program the Keyboard & Enable Interrupt with Cmd 0x60
                ;
                lda #$60                        ; Send Command 0x60 so to Enable Interrupt
                sta KBD_CMD_BUF
                jsr Poll_Inbuf 
                lda #%01000001                  ; Enable Interrupt (keyboard) and parallel port (?)
                ;LDA #%01001001      ; Enable Interrupt for Mouse and Keyboard
                sta KBD_DATA_BUF
                jsr SelectPage2                  ; put "3" on 3rd character
                lda #$33
                sta $C002
                jsr SelectPage0
                ;
                ;       Reset Keyboard
                ;
                jsr Poll_Inbuf;
                lda #$FF      ; Send Keyboard Reset command
                sta KBD_DATA_BUF
                ;
                ;       Delay while the keyboard resets.
                ;
                 ldy #$FF 
DLY_LOOP2       ldx #$FF
DLY_LOOP1       dex
                nop
                nop
                nop
                nop
                cpx #$00
                bne DLY_LOOP1
                dey 
                cpy #$00 
                bne DLY_LOOP2
                nop
;endless                jmp endless
                jsr Poll_Outbuf ;

                lda KBD_OUT_BUF                 ; Read Output Buffer

                jsr SelectPage2                     ;
                lda #$34                        ; put "4" in fourth slot.
                sta $C003
                jsr SelectPage0
DO_CMD_F4_AGAIN
                jsr Poll_Inbuf ;
                lda #$F4                        ; Enable the Keyboard
                sta KBD_DATA_BUF
                jsr Poll_Outbuf ;

                lda KBD_OUT_BUF                  ; Clear the Output buffer
;                cmp #$FA
;                bne DO_CMD_F4_AGAIN

                rts
                ;
                ;       Come here if keyboard failed
                ;
initkb_loop_out 
                rts


; ****************************************************************************************
;
;                               Interrupt Handler (Non Maskable)
;
; ****************************************************************************************

IRQHandler      
                pha                
                lda     CurrentPage                 ; switch to page 0
                and     #$FC
                sta     IOPageRegister

                LDA INT_PENDING_REG0                ; received reg0 SOL interrupt
                CMP #$00
                BEQ CHECK_PENDING_REG1

                ; Process SOL
                LDA INT_PENDING_REG0                ; was it SOL interrupt
                AND #JR0_INT01_SOL
                CMP #JR0_INT01_SOL
                BNE CHECK_KEYBOARD                  ; bug, was pending S1
                ; clear pending
                STA INT_PENDING_REG0

                
                lda BORDER_COLOR_R                  ; change the background colour.
                adc #$01
                sta BORDER_COLOR_R

CHECK_KEYBOARD
                LDA INT_PENDING_REG0                ; received Keyboard interrupt ?
                AND #JR0_INT02_KBD
                CMP #JR0_INT02_KBD
                BNE CHECK_PENDING_REG1
                ; clear pending
                STA INT_PENDING_REG0

                LDA KBD_INPT_BUF                    ; Get Scan Code from KeyBoard
                jsr     HandleKeyboard

                bra CHECK_PENDING_REG1


CHECK_PENDING_REG1                                  ; R1 interrupt checks (none)
                LDA INT_PENDING_REG1
                CMP #$00
                BEQ EXIT_IRQ_HANDLE          

EXIT_IRQ_HANDLE
                lda     CurrentPage                 ; restore current I/O Page
                sta     IOPageRegister
                pla
                RTI 

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