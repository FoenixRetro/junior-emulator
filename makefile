ifeq ($(OS),Windows_NT)
include documents\common.make
else
include documents/common.make
endif

all:kernel emulator

kernel:
	make -C kernel 

emulator: 
	make -C emulator		

run:emulator
	#$(CCOPY) ..$(S)superbasic$(S)bin$(S)basic.rom .
	.$(S)bin$(S)jr256$(APPSTEM) basic.rom@b
