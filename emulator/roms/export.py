# *******************************************************************************************
# *******************************************************************************************
#
#       Name :      export.py
#       Purpose :   Generate includable code for the emulator
#       Date :      19th November 2025
#       Author :    Paul Robson (paul@robsons.org.uk)
#
# *******************************************************************************************
# *******************************************************************************************

import os,pathlib

def export(sourceFile,targetFile):
	romImage = pathlib.Path(sourceFile).read_bytes()
	#print("Exporting {0} ({1} bytes)".format(sourceFile,len(romImage)))
	h = open(targetFile,"w")
	romImage = ",".join([str(x) for x in romImage])
	romImage = "{ "+romImage+" }"
	arrayName = targetFile.replace(".h","").split(os.sep)[-1]
	h.write("static const BYTE8 {0}[] = {1} ;\n\n".format(arrayName,romImage))
	h.close()

export("monitor.rom","__monitor_rom.h")
export("ega_8x8.bin","__foenix_charset.h")
