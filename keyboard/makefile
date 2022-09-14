ifeq ($(OS),Windows_NT)
include ..\documents\common.make
else
include ../documents/common.make
endif

all: 
	python ps2codes.py >..$(S)emulator$(S)include$(S)sdltops2.h
	python scancodes.py >..$(S)newmon$(S)ps2convert.inc