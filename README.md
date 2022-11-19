# junior-emulator

C256Junior emulator (Windows, Mac and Linux)

BUILDING
========

For simplicity, it is worth loading the following 6 repositories

The emulator
https://github.com/paulscottrobson/junior-emulator

Assorted additional bits and pieces
https://github.com/paulscottrobson/junior-utilities

The tiny kernal currently used to boot the emulator
https://github.com/paulscottrobson/junior-utilities

Basic Repository
https://github.com/paulscottrobson/superbasic

The USB uploader over serial
https://github.com/pweingar/FoenixMgr

The original kernal source
https://github.com/ghackwrench/OpenKERNAL

The emulator has SDL2 as a dependency. In Windows this needs to be installed into C:\SDL2 - Windows also requires mingw-gcc, python and make
(Chocolatey is the best way of installing these)

RUNNING
=======

To run use jr256 <file> <file> <file> where <file> is a filename followed by an @ followed by a load address in hex

e.g. ./jr256 basic.rom@b	dummy.code@3000 - the make run option/

The kernal simply initialises and jumps to $8000. So this line loads the BASIC rom to $8000 and a dummy to $3000 (because the kernal has
no save and load at the moment, the source code is loaded into memory at boot - this is just an end of file marker)

STATE
=====

Things that are implemented (mostly partially)

- Text display
- Keyboard and Interrupts
- Bitmap
- Sprites
- Memory LUT

Apologies for the terrible keyboard - I will fix this in the next few days, as there is now a Javascript version so it needs to respond properly,
the duff key handling is because the keyboard code in the TinyKernel - which is a stopgap - was quick and dirty.

DEBUGGER
========

F6 enters the debugger
F1 reset
F5 run
F7 step into
F8 step over
F9 set breakpoints
TAB display screen
Type 0-9A-F to change code display, shift 0-9A-F to change data display.
