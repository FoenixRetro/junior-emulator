# junior-emulator
C256Junior emulator (Windows, Mac and Linux)

Notes:

To run use jr256 <file> <file> <file> where <file> is a filename followed by an @ followed by a load address in hex

e.g. ./jr256 basic.rom@8000	

The run will be from the address of the last loaded file.

Things that are implemented (mostly partially)

- Text display
- Keyboard and Interrupts
- Bitmap
- Memory LUT (under consideration as not sure how it works)

Additionally if you have a file storage/load.dat in the directory, you can read this file in via the I/O in $C000-$DFFF via $D644 

Basic but useable debugger
--------------------------
F6 enters the debugger
F1 reset
F5 run
F7 step into
F8 step over
F9 set breakpoints
TAB display screen
Type 0-9A-F to change code display, shift 0-9A-F to change data display.