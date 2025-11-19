# *******************************************************************************************
# *******************************************************************************************
#
#		Name : 		Makefile
#		Purpose :	Create kernel keyboard mapping
#		Date :		19th November 2025
#		Author : 	Paul Robson (paul@robsons.org.uk)
#
# *******************************************************************************************
# *******************************************************************************************

include ../../environment/system.make

all: .any
	$(PYTHON) scancodes.py >__ps2convert.inc
	$(CCOPY) ps2.data ..$(S)..$(S)emulator$(S)keyboard
