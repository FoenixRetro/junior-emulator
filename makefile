ifeq ($(OS),Windows_NT)
include documents\common.make
else
include documents/common.make
endif

all:kernel_target emulator_target

kernel_target:
	make -C kernel 

emulator_target: 
	make -C emulator		

run:kernel_target emulator_target
	.$(S)bin$(S)jr256$(APPSTEM) basic.rom@b

clean:
	$(CDEL) $(BINDIR)jr256*
	$(CDEL) jr256* 
	$(CDEL) bin$(S)jr256*
	make -C emulator clean
	make -C kernel clean