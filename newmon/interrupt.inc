; Pending Interrupt (Read and Write Back to Clear)
INT_PENDING_REG0 = $D660 ;
INT_PENDING_REG1 = $D661 ;
INT_PENDING_REG2 = $D662 ; NOT USED
INT_PENDING_REG3 = $D663 ; NOT USED
; Polarity Set
INT_POL_REG0     = $D664 ;
INT_POL_REG1     = $D665 ;
INT_POL_REG2     = $D666 ;  NOT USED
INT_POL_REG3     = $D667 ; NOT USED
; Edge Detection Enable
INT_EDGE_REG0    = $D668 ;
INT_EDGE_REG1    = $D669 ;
INT_EDGE_REG2    = $D66A ; NOT USED
INT_EDGE_REG3    = $D66B ; NOT USED
; Mask
INT_MASK_REG0    = $D66C ;
INT_MASK_REG1    = $D66D ;
INT_MASK_REG2    = $D66E ; NOT USED
INT_MASK_REG3    = $D66F ; NOT USED
; Interrupt Bit Definition
; Register Block 0
JR0_INT00_SOF        = $01  ;Start of Frame @ 60FPS
JR0_INT01_SOL        = $02  ;Start of Line (Programmable)
JR0_INT02_KBD        = $04  ;
JR0_INT03_MOUSE      = $08  ;
JR0_INT04_TMR0       = $10  ;
JR0_INT05_TMR1       = $20  ;Real-Time Clock Interrupt
JR0_INT06_DMA        = $40  ;Floppy Disk Controller
JR0_INT07_TBD        = $80  ; Mouse Interrupt (INT12 in SuperIO IOspace)
; Register Block 1
JR1_INT00_UART       = $01  ;Keyboard Interrupt
JR1_INT01_COL0       = $02  ;TYVKY Collision TBD
JR1_INT02_COL1       = $04  ;TYVKY Collision TBD
JR1_INT03_COL2       = $08  ;TYVKY Collision TBD
JR1_INT04_RTC        = $10  ;Serial Port 1
JR1_INT05_VIA        = $20  ;Midi Controller Interrupt
JR1_INT06_IEC        = $40  ;Parallel Port
JR1_INT07_SDCARD     = $80  ;SDCard Insert

