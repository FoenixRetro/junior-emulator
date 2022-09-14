ifeq ($(OS),Windows_NT)
include documents\common.make
else
include documents/common.make
endif

all: processor newmonitor emulator

processor:
	make -C processor
	$(CCOPY) emulator$(S)jr256$(APPSTEM) .

newmonitor:
	make -C newmon	
	
emulator:
	make -C emulator		



