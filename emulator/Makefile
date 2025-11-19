# *******************************************************************************************
# *******************************************************************************************
#
#		Name : 		Makefile
#		Purpose :	Build the emulator
#		Date :		19th November 2025
#		Author : 	Paul Robson (paul@robsons.org.uk)
#
# *******************************************************************************************
# *******************************************************************************************

include ../environment/system.make

APPNAME = $(BUILDDIR)jr256$(APPSTEM)

SOURCES = 	src$(S)sys_processor.o  framework$(S)main.o framework$(S)gfx.o framework$(S)debugger.o \
			src$(S)sys_debug_f256.o src$(S)hardware.o src$(S)hw_fifo.o
  
CC = g++

.PHONY: all clean emulator prebuild release run

all: .any emulator

emulator: prebuild announce $(APPNAME)
	
%.o:%.cpp
	$(CC) $(CADDRESSES) $(CXXFLAGS) -D INCLUDE_DEBUGGING_SUPPORT -I cpu -I framework -I include -I . -c -o $@ $<

announce:
	echo "Building Emulator"
	
clean:
	$(CDEL) $(APPNAME) 
	$(CDEL) src$(S)*.o 
	$(CDEL) framework$(S)*.o

$(APPNAME): $(SOURCES)
	$(CC) $(SOURCES) $(LDFLAGS) $(SDL_LDFLAGS) -o $@

prebuild:
	$(MAKE) -B -C processor all
	$(MAKE) -B -C roms all
	$(MAKE) -B -C keyboard all
