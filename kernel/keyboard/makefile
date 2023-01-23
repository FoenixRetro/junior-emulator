ifeq ($(OS),Windows_NT)
include ..\..\documents\common.make
else
include ../../documents/common.make
endif

all: 
	python scancodes.py >..$(S)include$(S)ps2convert.inc
	$(CCOPY) ps2.data ..$(S)..$(S)emulator$(S)keyboard
