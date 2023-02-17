ifeq ($(OS),Windows_NT)
include documents\common.make
else
include documents/common.make
endif

all: kernel_target emulator_target

kernel_target:
	make -C kernel 

emulator_target: 
	make -C emulator		

run: kernel_target emulator_target
	.$(S)bin$(S)jr256$(APPSTEM) basic.rom@b

clean:
	make -C kernel clean
	make -C emulator clean