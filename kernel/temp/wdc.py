from pathlib import Path

##
# See: http://www.westerndesigncenter.com/wdc/datasheets/Assembler_Linker.pdf
# page 37
#
# Initial byte 'Z' as signature. 
#
# Then for each block: 
#       3 byte address
#       3 byte length
#       length bytes of data 
# 
# The final block has an address and length of 0.
#

class WdcBinFile:
    """Reads information from WDCTools BIN formated file"""
    data = 0
    handler = 0

    def __init__(self):
        pass

    def open(self, filename):
        self.data = Path(filename).read_bytes()

    def close(self):
        self.data = []

    def set_handler(self, proc):
        self.handler = proc

    def read_blocks(self):
        offset = 1
        while offset < len(self.data):
            (addr, block, offset) = self.__read_block(self.data, offset)
            if addr > 0:
                self.handler(addr, block)

    def __read_block(self, data, offset):
        addr = int.from_bytes(data[offset:offset+3], byteorder='little', signed=False)
        size = int.from_bytes(data[offset+3:offset+6], byteorder='little', signed=False)
        if addr == 0:
            return (0, [], offset+6)
        block = data[offset+6:offset+6+size]
        return (addr, block, offset+6+size)


