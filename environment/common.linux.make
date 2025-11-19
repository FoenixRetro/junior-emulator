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
#                   Uncommenting .SILENT will shut the whole build up.
#
# *******************************************************************************************

ifndef VERBOSE
.SILENT:
endif
