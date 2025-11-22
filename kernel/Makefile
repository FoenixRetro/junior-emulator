# *******************************************************************************************
# *******************************************************************************************
#
#		Name : 		Makefile
#		Purpose :	Build simple monitor binaries.
#		Date :		19th November 2025
#		Author : 	Paul Robson (paul@robsons.org.uk)
#
# *******************************************************************************************
# *******************************************************************************************

include ../environment/system.make

all: monitor

monitor: 
	echo "Building Kernel"
	$(MAKE) -C keyboard all
	64tass  $(AADDRESSES) -q -c -b -o $(BUILDDIR)monitor.rom -L $(BUILDDIR)__newmonitor.lst newmonitor.asm
	64tass  $(AADDRESSES) -q -c -b -o $(BUILDDIR)lockout.rom -L $(BUILDDIR)__lockout.lst src$(S)lockout.asm

