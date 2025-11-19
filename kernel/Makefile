include ../environment/system.make

all: monitor

monitor: 
	$(MAKE) -C keyboard all
	64tass  $(AADDRESSES) -q -c -b -o $(BUILDDIR)monitor.rom -L $(BUILDDIR)__newmonitor.lst newmonitor.asm
	64tass  $(AADDRESSES) -q -c -b -o $(BUILDDIR)lockout.rom -L output$(BUILDDIR)__lockout.lst src$(S)lockout.asm

