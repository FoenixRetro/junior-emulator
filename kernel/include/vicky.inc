;Internal Tiny VICKY Registers and Internal Memory Locations (LUTs)
; IO Page 0
MASTER_CTRL_REG_L	    = $D000
;Control Bits Fields
Mstr_Ctrl_Text_Mode_En  = $01       ; Enable the Text Mode
Mstr_Ctrl_Text_Overlay  = $02       ; Enable the Overlay of the text mode on top of Graphic Mode (the Background Color is ignored)
Mstr_Ctrl_Graph_Mode_En = $04       ; Enable the Graphic Mode
Mstr_Ctrl_Bitmap_En     = $08       ; Enable the Bitmap Module In Vicky
Mstr_Ctrl_TileMap_En    = $10       ; Enable the Tile Module in Vicky
Mstr_Ctrl_Sprite_En     = $20       ; Enable the Sprite Module in Vicky
Mstr_Ctrl_GAMMA_En      = $40       ; this Enable the GAMMA correction - The Analog and DVI have different color value, the GAMMA is great to correct the difference
Mstr_Ctrl_Disable_Vid   = $80       ; This will disable the Scanning of the Video hence giving 100% bandwith to the CPU
MASTER_CTRL_REG_H	    = $D001
; Reserved - TBD
VKY_RESERVED_00         = $D002
VKY_RESERVED_01         = $D003
; 
BORDER_CTRL_REG         = $D004 ; Bit[0] - Enable (1 by default)  Bit[4..6]: X Scroll Offset ( Will scroll Left) (Acceptable Value: 0..7)
Border_Ctrl_Enable      = $01
BORDER_COLOR_B          = $D005
BORDER_COLOR_G          = $D006
BORDER_COLOR_R          = $D007
BORDER_X_SIZE           = $D008; X-  Values: 0 - 32 (Default: 32)
BORDER_Y_SIZE           = $D009; Y- Values 0 -32 (Default: 32)
; Reserved - TBD
VKY_RESERVED_02         = $D00A
VKY_RESERVED_03         = $D00B
VKY_RESERVED_04         = $D00C
; Valid in Graphics Mode Only
BACKGROUND_COLOR_B      = $D00D ; When in Graphic Mode, if a pixel is "0" then the Background pixel is chosen
BACKGROUND_COLOR_G      = $D00E
BACKGROUND_COLOR_R      = $D00F ;
; Cursor Registers
VKY_TXT_CURSOR_CTRL_REG = $D010   ;[0]  Enable Text Mode
Vky_Cursor_Enable       = $01
Vky_Cursor_Flash_Rate0  = $02
Vky_Cursor_Flash_Rate1  = $04
VKY_TXT_START_ADD_PTR   = $D011   ; This is an offset to change the Starting address of the Text Mode Buffer (in x)
VKY_TXT_CURSOR_CHAR_REG = $D012
VKY_TXT_CURSOR_COLR_REG = $D013
VKY_TXT_CURSOR_X_REG_L  = $D014
VKY_TXT_CURSOR_X_REG_H  = $D015
VKY_TXT_CURSOR_Y_REG_L  = $D016
VKY_TXT_CURSOR_Y_REG_H  = $D017
; Line Interrupt 
VKY_LINE_IRQ_CTRL_REG   = $D018 ;[0] - Enable Line 0 - WRITE ONLY
VKY_LINE_CMP_VALUE_LO  = $D019 ;Write Only [7:0]
VKY_LINE_CMP_VALUE_HI  = $D01A ;Write Only [3:0]

VKY_PIXEL_X_POS_LO     = $D018 ; This is Where on the video line is the Pixel
VKY_PIXEL_X_POS_HI     = $D019 ; Or what pixel is being displayed when the register is read
VKY_LINE_Y_POS_LO      = $D01A ; This is the Line Value of the Raster
VKY_LINE_Y_POS_HI      = $D01B ; 

; Bitmap
TyVKY_BM0_CTRL_REG       = $D100 
BM0_Ctrl                = $01       ; Enable the BM0
BM0_LUT0                = $02       ; LUT0
BM0_LUT1                = $04       ; LUT1
BM0_LUT2                = $08       ; LUT2
TyVKY_BM0_START_ADDY_L   = $D101
TyVKY_BM0_START_ADDY_M   = $D102
TyVKY_BM0_START_ADDY_H   = $D103
TyVKY_BM1_CTRL_REG       = $D108 
BM1_Ctrl                = $01       ; Enable the BM0
BM1_LUT0                = $02       ; LUT0
BM1_LUT1                = $04       ; LUT1
BM1_LUT2                = $08       ; LUT2
TyVKY_BM1_START_ADDY_L   = $D109
TyVKY_BM1_START_ADDY_M   = $D10A
TyVKY_BM1_START_ADDY_H   = $D10B
; Tile Map 
TyVKY_TL_CTRL0          = $D200 


TyVKY_TL_MISC           = $D300 

TEXT_LUT_FG      = $D800
TEXT_LUT_BG		 = $D840


; PAGE 1
TyVKY_LUT0              = $D000 ; -$D000 - $D3FF
TyVKY_LUT1              = $D400 ; -$D400 - $D7FF
TyVKY_LUT2              = $D800 ; -$D800 - $DBFF
TyVKY_LUT3              = $DC00 ; -$DC00 - $DFFF

