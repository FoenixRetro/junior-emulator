# *******************************************************************************************
# *******************************************************************************************
#
#       Name :      system.make
#       Purpose :   Top level make include
#       Date :      6th November 2025
#       Author :    Paul Robson (paul@robsons.org.uk)
#
# *******************************************************************************************
# *******************************************************************************************

# Operating system detection
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    OS_NAME := linux
    BACKSLASH := 0
endif

ifeq ($(UNAME_S),Darwin)
    OS_NAME := macOS
    BACKSLASH := 0
endif

ifeq ($(OS),Windows_NT)
    OS_NAME := Windows
    BACKSLASH := 1
endif

#@echo "Operation System found: $(OS_NAME)"

#
#		Load Addresses
#
LMONITOR = E000
LLOCKOUT = F000
LTILEMAP = 24000
LTILEIMAGES = 26000
LSOURCE = 28000
LSPRITES = 30000
LBASIC = 34000
#
#		Expanded for command lines / makefiles.
#
DOLLAR = $$
CADDRESSES = -D MONITOR_ADDRESS=0x$(LMONITOR) -D LOCKOUT_ADDRESS=0x$(LLOCKOUT) -D BASIC_ADDRESS=0x$(LBASIC) -D SOURCE_ADDRESS=0x$(LSOURCE) -D SPRITE_ADDRESS=0x$(LSPRITES) \
			 -D TILEMAP_ADDRESS=0x$(LTILEMAP) -D TILEIMAGES_ADDRESS=0x$(LTILEIMAGES)
AADDRESSES = '-D MONITOR_ADDRESS=$(DOLLAR)$(LMONITOR)' '-D LOCKOUT_ADDRESS=$(DOLLAR)$(LLOCKOUT)' '-D BASIC_ADDRESS=$(DOLLAR)$(LBASIC)' \
			 '-D SOURCE_ADDRESS=$(DOLLAR)$(LSOURCE)' '-D SPRITE_ADDRESS=$(DOLLAR)$(LSPRITES)' \
			 '-D TILEMAP_ADDRESS=$(DOLLAR)$(LTILEMAP)' '-D TILEIMAGES_ADDRESS=$(DOLLAR)$(LTILEIMAGES)'

ROOTDIR =  $(dir $(realpath $(lastword $(MAKEFILE_LIST))))../
include $(ROOTDIR)environment/common.$(OS_NAME).make

