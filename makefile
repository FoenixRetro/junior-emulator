ifeq ($(OS),Windows_NT)
include documents\common.make
else
include documents/common.make
endif

all:emulator

emulator: 
	make -C emulator		
	$(CCOPY) emulator$(S)jr256$(APPSTEM) .

run:emulator
	$(CCOPY) ..$(S)superbasic$(S)bin$(S)basic.rom .
	.$(S)jr256$(APPSTEM) basic.rom@B dummy.code@x
