#
#		ROM exporter
#
import os

def export(sourceFile,targetFile):
	romImage = [x for x in open(sourceFile,"rb").read(-1)]
	print("Exporting {0} ({1} bytes)".format(sourceFile,len(romImage)))
	h = open(targetFile,"w")
	romImage = ",".join([str(x) for x in romImage])
	romImage = "{ "+romImage+" }"
	arrayName = targetFile.replace(".h","").split(os.sep)[-1]
	h.write("static const BYTE8 {0}[] = {1} ;\n\n".format(arrayName,romImage))
	h.close()

export("monitor.rom","roms/monitor_rom.h")
#export("foenix.bin","foenix_charset.h")
export("roms/Bm437_PhoenixEGA_8x8.bin","roms/foenix_charset.h")
#export("basic.rom","basic_rom.h")