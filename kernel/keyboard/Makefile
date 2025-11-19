include ../../environment/system.make

all: .any
	$(PYTHON) scancodes.py >__ps2convert.inc
	$(CCOPY) ps2.data ..$(S)..$(S)emulator$(S)keyboard
