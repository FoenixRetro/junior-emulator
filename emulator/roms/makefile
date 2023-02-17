ifeq ($(OS),Windows_NT)
include ..\..\documents\common.make
else
include ../../documents/common.make
endif


all: 
	$(CCOPY) ..$(S)..$(S)bin$(S)monitor.rom .
	$(PYTHON) export.py