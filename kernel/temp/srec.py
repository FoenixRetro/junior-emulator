import re

class SRECFile:
    """Read information from a Motorola SREC file."""

    file = 0
    handler = 0

    def __init__(self):
        self.file = 0
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
        # Format of line will vary based on the type, so let's get the type first
        m = re.match("^S([0-9a-fA-F])([0-9a-fA-F]+)", line)
        code = int(m.group(1), 16)
        hex_digits = m.group(2)

        # Codes...
        # 0 = Comment/header
        # 1 = Data with 16-bit address
        # 2 = Data with 24-bit address
        # 3 = Data with 32-bit address
        # 4 = Reserved
        # 5 = 16-bit record count
        # 6 = 24-bit record count
        # 7 = 32-bit start address
        # 8 = 24-bit start address
        # 9 = 16-bit start address
        #
        # This code will ignore all records by 1, 2, and 3

        if code == 1:
            # Unpack a record with a 16-bit address
            m2 = re.match("^([0-9a-fA-F]{2})([0-9a-fA-F]{4})([0-9a-fA-F]*)([0-9a-fA-F]{2})", hex_digits)
            count = int(m2.group(1), 16)
            address = int(m2.group(2), 16)
            data = m2.group(3)
            crc = int(m2.group(4), 16)
            self.handler(address, data)

        elif code == 2:
            # Unpack a record with a 24-bit address
            m2 = re.match("^([0-9a-fA-F]{2})([0-9a-fA-F]{6})([0-9a-fA-F]*)([0-9a-fA-F]{2})", hex_digits)
            count = int(m2.group(1), 16)
            address = int(m2.group(2), 16)
            data = m2.group(3)
            crc = int(m2.group(4), 16)
            self.handler(address, data)

        elif code == 3:
            # Unpack a record with a 32-bit address
            m2 = re.match("^([0-9a-fA-F]{2})([0-9a-fA-F]{8})([0-9a-fA-F]*)([0-9a-fA-F]{2})", hex_digits)
            count = int(m2.group(1), 16)
            address = int(m2.group(2), 16)
            data = m2.group(3)
            crc = int(m2.group(4), 16)
            self.handler(address, data)
