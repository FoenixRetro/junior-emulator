ifeq ($(OS),Windows_NT)
include ..\documents\common.make
else
include ../documents/common.make
endif


all: monitor.rom

run: monitor.rom 
	..$(S)..$(S)bin$(S)uk101 go x

monitor.rom : newmonitor.asm vicky.inc hardware.asm
	64tass -c -b -o monitor.rom -L newmonitor.lst newmonitor.asm
	python export.py
	$(CCOPY) *.h ..$(S)emulator$(S)roms

	


	

