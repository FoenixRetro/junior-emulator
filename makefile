ifeq ($(OS),Windows_NT)
include documents\common.make
else
include documents/common.make
endif

all: keyboard processor newmonitor emulator

processor:
	make -C processor

newmonitor:
	make -B -C ..$(S)junior-tinykernel
	
emulator:
	make -C emulator		
	$(CCOPY) emulator$(S)jr256$(APPSTEM) .

keyboard:
	make -C keyboard

