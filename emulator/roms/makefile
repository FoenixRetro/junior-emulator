ifeq ($(OS),Windows_NT)
include ..\..\documents\common.make
else
include ../../documents/common.make
endif


all: 
	make -B -C ..$(S)..$(S)..$(S)junior-tinykernel
	$(CCOPY) ..$(S)..$(S)..$(S)junior-tinykernel$(S)monitor.rom .
	python export.py