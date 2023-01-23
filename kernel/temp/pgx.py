#
# Loader for PGX files
#

import sys
import constants
import foenix_config
from pathlib import Path

class PGXBinFile:
    """Reads information from PGX formated file"""
    address_size = 0
    data = 0
    handler = 0
    cpu = ""

    def __init__(self):
        config = foenix_config.FoenixConfig()
        self.cpu = config.cpu()
        pass

    def open(self, filename):
        self.data = Path(filename).read_bytes()

    def close(self):
        self.data = []

    def set_handler(self, proc):
        self.handler = proc

    def read_blocks(self):
        # Check the signature
        signature = self.data[constants.PGX_OFF_SIG_START:constants.PGX_OFF_SIG_END]
        if signature != b'PGX':
            print("Bad PGX signature: {}".format(signature))
            sys.exit(1)

        # Check version tag
        pgx_ver = (self.data[constants.PGX_OFF_VERSION] >> 4) & 0x0f
        if pgx_ver > 0:
            print("Unsupported PGX version.")
            sys.exit(1)

        # Check the CPU tag
        pgx_cpu = self.data[constants.PGX_OFF_VERSION] & 0x0f
        if pgx_cpu == constants.PGX_CPU_65816:
            if self.cpu != "65816":
                print("PGX is built for the wrong CPU.")
                sys.exit(1)
        elif pgx_cpu == constants.PGX_CPU_65C02:
            if self.cpu != "65C02" and self.cpu != "65c02":
                print("PGX is built for the wrong CPU.")
                sys.exit(1)
        else:
            print("Unsupported PGX CPU.")
            sys.exit(1)

        # Get the target address of the PGX file
        addr = int.from_bytes(self.data[constants.PGX_OFF_ADDR_START:constants.PGX_OFF_ADDR_END], byteorder='little', signed=False)

        # Get the actual block of data
        block = self.data[constants.PGX_OFF_DATA:]

        # Send the data to the address
        self.handler(addr, block)

        if self.cpu == "65816":
            if addr & 0xff0000 != 0:
                # Startup code is not in bank 0, so we need a stub...
                #   clc
                #   xce
                #   jml <address>
                self.handler(0xff80, bytes([0x18, 0xfb, 0x5c, addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff]))
                # Point the reset vector to our reset routine
                self.handler(0xfffc, bytes([0x80, 0xff]))
            else:
                # Startup code is in bank 0, so we can just jump to it directly
                # Point the reset vector to the start address
                self.handler(0xfffc, bytes([addr & 0xff, (addr >> 8) & 0xff]))

        elif (self.cpu == "65c02") or (self.cpu == "65C02"):
            # Point the reset vector to our reset routine
            self.handler(0xfffc, bytes([addr & 0xff, (addr >> 8) & 0xff]))
