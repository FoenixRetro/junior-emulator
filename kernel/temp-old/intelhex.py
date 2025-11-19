import re

class HexFile:
    """Read information from an Intel Hex file."""

    file = 0
    base_address = 0
    handler = 0

    def __init__(self):
        self.file = 0
        self.base_address = 0
        self.handler = 0
    
    def open(self, filename):
        self.file = open(filename, 'r')

    def set_handler(self, proc):
        self.handler = proc

    def close(self):
        self.file.close()

    def read_lines(self):
        line = self.file.readline()
        while line:
            self.parse_line(line)
            line = self.file.readline()

    def parse_line(self, line):
        m = re.match("^:([0-9a-fA-F]{2})([0-9a-fA-F]{4})([0-9a-fA-F]{2})([0-9a-fA-F]*)([0-9a-fA-F]{2})", line)
        size = int(m.group(1), 16)
        address = int(m.group(2), 16)
        code = int(m.group(3), 16)
        data = m.group(4)
        crc = int(m.group(5), 16)
        if code == 0:
            if self.handler:
                # print('Sending record to {:X}'.format(self.base_address + address))
                self.handler(self.base_address + address, data)

        elif code == 2:
            # Set the base address based on a segment
            self.base_address = int(data, 16) << 4 # shity 80x86 real mode addressing : take the address an do *16 to get the final address
            # print('Setting base address to {:X}'.format(self.base_address))

        elif code == 4:
            # Set the base address given on address[31..16]
            self.base_address = int(data, 16) << 16
            # print('Setting base address to {:X}'.format(self.base_address))


