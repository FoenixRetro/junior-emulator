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
	.$(S)jr256$(APPSTEM) basic.rom@B dummy.code@3000
