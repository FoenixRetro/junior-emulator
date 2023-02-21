# ***********************************************************************************
#
#										Common Build 
#
# ***********************************************************************************
#
#	NB: Windows SDL2 is hard coded.
#
ifeq ($(OS),Windows_NT)
CCOPY = copy
CMAKE = make
CDEL = del /Q
CDELQ = >NUL
APPSTEM = .exe
S = \\
SDLDIR = C:\\sdl2
CXXFLAGS = -I$(SDLDIR)$(S)include$(S)SDL2 -I . -fno-stack-protector -w -Wl,-subsystem,windows -DSDL_MAIN_HANDLED
LDFLAGS = -lmingw32 
SDL_LDFLAGS = -L$(SDLDIR)$(S)lib -lSDL2 -lSDL2main -static-libstdc++ -static-libgcc
OSNAME = windows
EXTRAFILES = libwinpthread-1.dll  SDL2.dll
PYTHON = python
else
CCOPY = cp
CDEL = rm -f
CDELQ = 
CMAKE = make
APPSTEM =
S = /
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs)
CXXFLAGS = $(SDL_CFLAGS) -O2 -DLINUX  -fmax-errors=5 -I.  
LDFLAGS = 
OSNAME = linux
EXTRAFILES = 
PYTHON = python3
endif
#
#		Root directory
#
ROOTDIR = ..$(S)
#
#		Current assembler
# 
ASM = 64tass
#
#		Load Addresses
#
LMONITOR = E000
LLOCKOUT = F000
LTILEMAP = 24000
LTILEIMAGES = 26000
LSOURCE = 28000
LSPRITES = 30000
LBASIC = 34000
#
#		Expanded for command lines / makefiles.
#
DOLLAR = $$
CADDRESSES = -D MONITOR_ADDRESS=0x$(LMONITOR) -D LOCKOUT_ADDRESS=0x$(LLOCKOUT) -D BASIC_ADDRESS=0x$(LBASIC) -D SOURCE_ADDRESS=0x$(LSOURCE) -D SPRITE_ADDRESS=0x$(LSPRITES) \
			 -D TILEMAP_ADDRESS=0x$(LTILEMAP) -D TILEIMAGES_ADDRESS=0x$(LTILEIMAGES)
AADDRESSES = '-D MONITOR_ADDRESS=$(DOLLAR)$(LMONITOR)' '-D LOCKOUT_ADDRESS=$(DOLLAR)$(LLOCKOUT)' '-D BASIC_ADDRESS=$(DOLLAR)$(LBASIC)' \
			 '-D SOURCE_ADDRESS=$(DOLLAR)$(LSOURCE)' '-D SPRITE_ADDRESS=$(DOLLAR)$(LSPRITES)' \
			 '-D TILEMAP_ADDRESS=$(DOLLAR)$(LTILEMAP)' '-D TILEIMAGES_ADDRESS=$(DOLLAR)$(LTILEIMAGES)'
#
#		Serial port
#
TTYPORT = /dev/ttyUSB0
