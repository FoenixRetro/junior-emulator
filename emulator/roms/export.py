#
#		ROM exporter
#
import os,pathlib

def export(sourceFile,targetFile):
	romImage = pathlib.Path(sourceFile).read_bytes()
	print("Exporting {0} ({1} bytes)".format(sourceFile,len(romImage)))
	h = open(targetFile,"w")
	romImage = ",".join([str(x) for x in romImage])
	romImage = "{ "+romImage+" }"
	arrayName = targetFile.replace(".h","").split(os.sep)[-1]
	h.write("static const BYTE8 {0}[] = {1} ;\n\n".format(arrayName,romImage))
	h.close()

export("monitor.rom","monitor_rom.h")
export("Bm437_PhoenixEGA_8x8.bin","foenix_charset.h")
