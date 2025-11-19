# *******************************************************************************************
# *******************************************************************************************
#
#		Name : 		Makefile
#		Purpose :	Main makefile.
#		Date :		19th November 2025
#		Author : 	Paul Robson (paul@robsons.org.uk)
#
# *******************************************************************************************
# *******************************************************************************************

include environment/system.make

all:kernel_target emulator_target

kernel_target:
	$(MAKE) -C kernel all

emulator_target: 
	$(MAKE) -C emulator all		

run:kernel_target emulator_target
	$(BUILDDIR)jr256$(APPSTEM) basic.rom@b

clean:
	rm build/*
	$(MAKE) -C emulator clean
	$(MAKE) -C kernel clean