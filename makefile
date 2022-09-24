ifeq ($(OS),Windows_NT)
include documents\common.make
else
include documents/common.make
endif

all: 
	make -C emulator		
	$(CCOPY) emulator$(S)jr256$(APPSTEM) .


