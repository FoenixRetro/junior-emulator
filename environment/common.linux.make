# *******************************************************************************************
# *******************************************************************************************
# *******************************************************************************************
#
#       Name :      common.linux.make
#       Purpose :   Common make
#       Date :      6th November 2025
#       Author :    Paul Robson (paul@robsons.org.uk)
#
# *******************************************************************************************
# *******************************************************************************************

OS = linux

# *******************************************************************************************
#
#                                       Directories
#
# *******************************************************************************************
#
#       Build files target.
#
BUILDDIR = $(ROOTDIR)build/
#
#       Where the build environment files are (e.g. like this)
#
BUILDENVDIR = $(ROOTDIR)environment/
#
#		Python Paths
#
PYTHONDIRS=$(SCRIPTDIR):$(PYTHONPATH)
#
#		Script path
#
SCRIPTDIR = $(ROOTDIR)scripts/

# *******************************************************************************************
#
#                                       Configuration
#
# *******************************************************************************************

CC = gcc
PYTHON = PYTHONPATH=$(PYTHONDIRS) python3
S = /
MAKE = make --no-print-directory
CCOPY = cp
CDEL = rm -f
CDELQ = 
APPSTEM =
S = /
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs)
CXXFLAGS = $(SDL_CFLAGS) -O2 -DLINUX  -fmax-errors=5 -I.  
LDFLAGS = 

.any : 

# *******************************************************************************************
#
#                                       Build System
#
# *******************************************************************************************

ASSEMBLER = 64tass
ASMOPTS =  -q -Wall -Wno-portable
BOOTMODULES = system.config.@hardware system.boot system.hardware.@hardware
ENDMODULE = system.last.uploader system.last.vectors
NUMBERS =
INCLUDES = /usr/include

prelim:
	
buildemu: .all prelim
	rm -f memory.dump dump*.bin dap.log 
	$(PYTHON) $(SCRIPTDIR)builder.py output=$(BUILDDIR)basic65.asm dir=$(MODULEDIR) cpu=65816 hardware=emu $(BOOTMODULES) $(MODULES) $(ENDMODULE)
	$(ASSEMBLER) -x --atari-xex $(ASMOPTS) $(BUILDDIR)basic65.asm $(ASMOPTS) -o$(BUILDDIR)basic65.xex -L$(BUILDDIR)basic65.lst -l$(BUILDDIR)basic65.lbl

buildx65: .all prelim
	rm -f memory.dump dump*.bin dap.log
	$(PYTHON) $(SCRIPTDIR)builder.py output=$(BUILDDIR)basic65.asm dir=$(MODULEDIR) cpu=65816 hardware=x65 $(BOOTMODULES) $(MODULES) $(ENDMODULE)
	$(ASSEMBLER) -x --atari-xex $(ASMOPTS) $(BUILDDIR)basic65.asm $(ASMOPTS) -o$(BUILDDIR)basic65.xex -L$(BUILDDIR)basic65.lst -l$(BUILDDIR)basic65.lbl	

run: buildemu
	$(BUILDDIR)emu65816 $(BUILDDIR)basic65.xex go

runx65: buildx65
	$(BUILDDIR)emu_linux_x64 -q file=$(BUILDDIR)basic65.xex break=EA

# *******************************************************************************************
#
#                   Uncommenting .SILENT will shut the whole build up.
#
# *******************************************************************************************

ifndef VERBOSE
#.SILENT:
endif
