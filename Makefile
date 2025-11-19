include environment/system.make

all:kernel_target emulator_target

kernel_target:
	make -C kernel all

emulator_target: 
	make -C emulator all		

run:kernel_target emulator_target
	.$(S)bin$(S)jr256$(APPSTEM) basic.rom@b

clean:
	rm build/*
	make -C emulator clean
	make -C kernel clean