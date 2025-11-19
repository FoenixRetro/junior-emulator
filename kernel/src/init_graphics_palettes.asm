; *******************************************************************************************
; *******************************************************************************************
;
;       Name :      init_graphics_palettes.asm
;       Purpose :   Initialise the Graphics Palettes
;       Date :      19th November 2025
;       Author :    Gadget
;
; *******************************************************************************************
; *******************************************************************************************
;
;     Lifted straight from OpenKernal
;
init_graphics_palettes

            phx
            phy

          ; Save I/O page
            ldy     $1

          ; Switch to I/O Page 1 (font and color LUTs)
            lda     #1
            sta     $1

          ; Init zTemp0
            stz     zTemp0+0
            lda     #$d0
            sta     zTemp0+1

            ldx     #0          ; Starting color byte.
_loop
          ; Write the next color entry
            jsr     write_bgra
            inx

          ; Advance the pointer; X will wrap around on its own

            lda     zTemp0
            adc     #4
            sta     zTemp0
            bne     _loop

            lda     zTemp0+1
            inc     a
            sta     zTemp0+1
            cmp     #$e0
            bne     _loop
        
          ; Restore I/O page
            sty     $1
            
            ply
            plx
            rts

write_bgra
    ; X = rrrgggbb
    ; A palette entry consists of four consecutive bytes: B, G, R, A.

            phy
            ldy     #3  ; Working backwards: A,R,G,B

          ; Write the Alpha value        
            lda     #255
            jsr     _write
            
          ; Write the RGB values
            txa
_loop       dey
            bmi     _done
            jsr     _write            
            bra     _loop
            
_done       ply
            clc
            rts
            
_write      
          ; Write the upper bits to (zTemp0),y
            pha
            and     #%111_00000
            sta     (zTemp0),y
            pla

          ; Shift in the next set of bits (blue truncated, alpha zero).
            asl     a
            asl     a
            asl     a

            rts
