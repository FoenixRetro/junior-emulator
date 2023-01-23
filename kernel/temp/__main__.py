import intelhex
import wdc
import foenix
import foenix_config
import constants
import srec
import re
import sys
import argparse
import os
import pgz
import pgx
import csv

from serial.tools import list_ports

label_file = ""
to_send = ""
port = ""
start_address = ""
count = ""
label = ""

def confirm(question):
    return input(question).lower().strip()[:1] == "y"

def revision(port):
    """Get the version code for the debug port."""
    c256 = foenix.FoenixDebugPort()
    try:
        c256.open(port)
        c256.enter_debug()
        try:
            data = c256.get_revision()
            return "%X" % data
        finally:
            c256.exit_debug()
    finally:
        c256.close()

def upload_binary(port, filename, address):
    """Upload a binary file into the C256 memory."""
    with open(filename, "rb") as f:
        c256 = foenix.FoenixDebugPort()
        try:
            c256.open(port)
            c256.enter_debug()
            try:
                current_addr = int(address, 16)
                block = f.read(config.chunk_size())
                while block:
                    c256.write_block(current_addr, block)
                    current_addr += len(block)
                    block = f.read(config.chunk_size())
            finally:
                c256.exit_debug()
        finally:
            c256.close()

def program_flash_sector(port, filename, sector):
    """Program an 8KB sector of the flash memory using the contents of the C256's RAM."""
          
    # Sectors are always programmed from the contents of 0x00000 - 0x01FFF
    address = 0     
    sector_nbr = int(sector, 16)
    print("About to upload image to sector 0x{:02X}".format(sector_nbr), flush=True)


    if confirm("Are you sure you want to reprogram the flash sector? (y/n): "):
        with open(filename, "rb") as f:
            c256 = foenix.FoenixDebugPort()
            try:
                c256.open(port)
                c256.enter_debug()
                try:
                    block = f.read(config.chunk_size())
                    while block:
                        c256.write_block(address, block)
                        address += len(block)
                        block = f.read(config.chunk_size())

                    print("Binary file uploaded...", flush=True)
                    c256.erase_flash_sector(sector_nbr)
                    print("Flash sector erased...", flush=True)
                    c256.program_flash_sector(sector_nbr)
                    print("Flash sector programmed...")
                finally:
                    c256.exit_debug()
            finally:
                c256.close()

def program_flash_bulk(port, csv_file):
    """Program the flash sector by sector, given a CSV file mapping sectors to files."""

    with open(csv_file, "r") as bulk_mapping:
        c256 = foenix.FoenixDebugPort()
        try:
            c256.open(port)
            c256.enter_debug()
            try:
                bulk_reader = csv.reader(bulk_mapping)
                for row in bulk_reader:
                    sector_id = row[0]
                    sector_file = row[1]

                    sector_nbr = int(sector_id, 16)
                    print("Attempting to program sector 0x{0:02X} with {1}".format(sector_nbr, sector_file))

                    with open(sector_file, "rb") as f:
                        address = 0
                        block = f.read(config.chunk_size())
                        while block:
                            c256.write_block(address, block)
                            address += len(block)
                            block = f.read(config.chunk_size())

                        print("Binary file uploaded...", flush=True)
                        c256.erase_flash_sector(sector_nbr)
                        print("Flash sector erased...", flush=True)
                        c256.program_flash_sector(sector_nbr)
                        print("Flash sector programmed...")
            finally:
                c256.exit_debug()
        finally:
            c256.close()

def program_flash(port, filename, hex_address):
    """Program the flash memory using the contents of the C256's RAM."""

    base_address = int(hex_address, 16)
    address = base_address
    print("About to upload image to address 0x{:X}".format(address), flush=True)

    if os.path.getsize(filename) == config.flash_size():
        if confirm("Are you sure you want to reprogram the flash memory? (y/n): "):
            with open(filename, "rb") as f:
                c256 = foenix.FoenixDebugPort()
                try:
                    c256.open(port)
                    c256.enter_debug()
                    try:
                        block = f.read(config.chunk_size())
                        while block:
                            c256.write_block(address, block)
                            address += len(block)
                            block = f.read(config.chunk_size())

                        print("Binary file uploaded...", flush=True)
                        c256.erase_flash()
                        print("Flash memory erased...", flush=True)
                        c256.program_flash(base_address)
                        print("Flash memory programmed...")
                    finally:
                        c256.exit_debug()
                finally:
                    c256.close()
    else:
        print("The provided flash file is not the right size.")

def dereference(port, file, label):
    """Get the address contained in the pointer with the label in the label file."""
    c256 = foenix.FoenixDebugPort()
    try:
        address = lookup(file, label)
        c256.open(port)
        c256.enter_debug()
        try:
            data = c256.read_block(int(address, 16), 3)
            deref = data[2] << 16 | data[1] << 8 | data[0]
            return "%X" % deref
        finally:
            c256.exit_debug()
    finally:
        c256.close()

def lookup(file, label):
    """Return the hex address linked to the passed label in the label file."""
    with open(file) as f:
        for line in f:
            match = re.match('^(\S+)\s*\=\s*\$(\S+)', line)
            if match:
                if match.group(1) == label:
                    return match.group(2)
        sys.stderr.write("Could not find a definition for that label.\n")
        sys.exit(2)

def display(base_address, data):
    """Write a block of data to the console in a nice, hexadecimal format."""

    text_buff = ""
    for i in range(0, len(data)):
        if (i % 16) == 0:
            if text_buff != "":
                sys.stdout.write(" {}\n".format(text_buff))
            text_buff = ""
            sys.stdout.write("{:06X}: ".format(base_address + i))
        elif (i % 8) == 0:
            sys.stdout.write(" ")
        sys.stdout.write("{:02X}".format(data[i]))

        b = bytearray(1)
        b[0] = data[i]
        if (b[0] & 0x80 == 0):
            c = b.decode('ascii')
            if c.isprintable():
                text_buff = text_buff + c
            else:
                text_buff = text_buff + "."
        else:
            text_buff = text_buff + "."

    sys.stdout.write(' {}\n'.format(text_buff))

def send_pgx(port, filename):
    """Send the data in the PGX file 'filename' to the C256 on the given serial port."""
    infile = pgx.PGXBinFile()
    c256 = foenix.FoenixDebugPort()
    try:
        c256.open(port)
        infile.open(filename)
        try:
            infile.set_handler(lambda address, data: c256.write_block(address, data))
            c256.enter_debug()
            try:
                # Process the lines in the hex file
                infile.read_blocks()
            finally:
                c256.exit_debug()
        finally:
            infile.close()
    finally:
        c256.close()

def send_pgz(port, filename):
    """Send the data in the PGZ file 'filename' to the C256 on the given serial port."""
    infile = pgz.PGZBinFile()
    c256 = foenix.FoenixDebugPort()
    try:
        c256.open(port)
        infile.open(filename)
        try:
            infile.set_handler(lambda address, data: c256.write_block(address, data))
            c256.enter_debug()
            try:
                # Process the lines in the hex file
                infile.read_blocks()
            finally:
                c256.exit_debug()
        finally:
            infile.close()
    finally:
        c256.close()

def send_wdc(port, filename):
    """Send the data in the hex file 'filename' to the C256 on the given serial port."""
    infile = wdc.WdcBinFile()
    c256 = foenix.FoenixDebugPort()
    try:
        c256.open(port)
        infile.open(filename)
        try:
            infile.set_handler(lambda address, data: c256.write_block(address, data))
            c256.enter_debug()
            try:
                # Process the lines in the hex file
                infile.read_blocks()
            finally:
                c256.exit_debug()
        finally:
            infile.close()
    finally:
        c256.close()

def send_srec(port, filename):
    """Send the data in the SREC hex file 'filename' to the C256 on the given serial port."""
    infile = srec.SRECFile()
    c256 = foenix.FoenixDebugPort()
    try:
        c256.open(port)
        infile.open(filename)
        try:
            infile.set_handler(lambda address, data: c256.write_block(address, bytes.fromhex(data)))
            c256.enter_debug()
            try:
                # Process the lines in the hex file
                infile.read_lines()
            finally:
                c256.exit_debug()
        finally:
            infile.close()
    finally:
        c256.close()

def send(port, filename):
    """Send the data in the hex file 'filename' to the C256 on the given serial port."""
    infile = intelhex.HexFile()
    c256 = foenix.FoenixDebugPort()
    try:
        c256.open(port)
        infile.open(filename)
        try:
            infile.set_handler(lambda address, data: c256.write_block(address, bytes.fromhex(data)))
            c256.enter_debug()
            try:
                # Process the lines in the hex file
                infile.read_lines()
            finally:
                c256.exit_debug()
        finally:
            infile.close()
    finally:
        c256.close()

def get(port, address, length):
    """Read a block of data from the C256."""
    c256 = foenix.FoenixDebugPort()
    try:
        c256.open(port)
        c256.enter_debug()
        try:
            data = c256.read_block(int(address, 16), int(length, 16))

            display(int(address, 16), data)
        finally:
            c256.exit_debug()
    finally:
        c256.close()

def list_serial_ports():
    serial_ports = list_ports.comports()

    if len(serial_ports) == 0:
        print("No serial ports found")

    for serial_port in serial_ports:
        print(f"{serial_port.device}")
        print(f"   Description: {serial_port.description}")
        print(f"   Manufacturer: {serial_port.manufacturer}")
        print(f"   Product: {serial_port.product}")
        print()

def tcp_bridge(tcp_host_port, serial_port):
    """ Listen for TCP socket connections and relay messages to Foenix via serial port """
    parsed_host_port = tcp_host_port.split(":")
    tcp_host = parsed_host_port[0]
    tcp_port = int(parsed_host_port[1]) if len(parsed_host_port) > 0 else 2560
    tcp_listener = foenix.FoenixTcpBridge(tcp_host, tcp_port, serial_port)
    tcp_listener.listen()

def set_boot_source(port, source):
    """For F256jr RevA only: set the boot source for the machine"""
    c256 = foenix.FoenixDebugPort()
    try:
        c256.open(port)
        c256.enter_debug()
        try:
            if source == "ram":
                print("Setting boot source to RAM...")
                c256.set_boot_source(constants.BOOT_SRC_RAM)
            elif source == "flash":
                print("Setting boot source to flash...")
                c256.set_boot_source(constants.BOOT_SRC_FLASH)
            else:
                print("Unknown boot source")
            
        finally:
            c256.exit_debug()
    finally:
        c256.close()

# Load the configuration file...
config = foenix_config.FoenixConfig()

parser = argparse.ArgumentParser(description='Manage the C256 Foenix through its debug port.')
parser.add_argument("--port", dest="port", default=config.port(),
                    help="Specify the serial port to use to access the C256 debug port.")

parser.add_argument("--list-ports", dest="list_ports", action="store_true",
                    help="List available serial ports.")

parser.add_argument("--label-file", dest="label_file", default=config.label_file(),
                    help="Specify the label file to use for dereference and lookup")

parser.add_argument("--count", dest="count", default="10", help="the number of bytes to read")

parser.add_argument("--dump", metavar="ADDRESS", dest="dump_address",
                    help="Read memory from the C256's memory and display it.")

parser.add_argument("--deref", metavar="LABEL", dest="deref_name",
                    help="Lookup the address stored at LABEL and display the memory there.")

parser.add_argument("--lookup", metavar="LABEL", dest="lookup_name",
                    help="Display the memory starting at the address indicated by the label.")

parser.add_argument("--revision", action="store_true", dest="revision",
                    help="Display the revision code of the debug interface.")

parser.add_argument("--flash", metavar="BINARY FILE", dest="flash_file",
                    help="Attempt to reprogram the flash using the binary file provided.")

parser.add_argument("--flash-sector", metavar="NUMBER", dest="flash_sector",
                    help="Sector number of the 8KB sector of flash to program.")

parser.add_argument("--flash-bulk", metavar="CSV FILE", dest="bulk_file",
                    help="Program multiple flash sectors based on a CSV file")

parser.add_argument("--binary", metavar="BINARY FILE", dest="binary_file",
                    help="Upload a binary file to the C256's RAM.")

parser.add_argument("--address", metavar="ADDRESS", dest="address",
                    default=config.address(),
                    help="Provide the starting address of the memory block to use in flashing memory.")

parser.add_argument("--upload", metavar="HEX FILE", dest="hex_file",
                    help="Upload an Intel HEX file.")

parser.add_argument("--upload-wdc", metavar="BINARY FILE", dest="wdc_file",
                    help="Upload a WDCTools binary hex file. (WDCLN.EXE -HZ)")

parser.add_argument("--run-pgz", metavar="PGZ FILE", dest="pgz_file",
                    help="Upload and run a PGZ binary file.")

parser.add_argument("--run-pgx", metavar="PGX FILE", dest="pgx_file",
                    help="Upload and run a PGX binary file.")

parser.add_argument("--upload-srec", metavar="SREC FILE", dest="srec_file",
                    help="Upload a Motorola SREC hex file.")

parser.add_argument("--boot", metavar="STRING", dest="boot_source",
                    help="For F256jr RevA: set boot source: RAM or FLASH")

parser.add_argument("--tcp-bridge", metavar="HOST:PORT", dest="tcp_host_port",
                    help="Setup a TCP-serial bridge, listening on HOST:PORT and relaying messages to the Foenix via " +
                         "the configured serial port")

options = parser.parse_args()

try:
    if options.list_ports:
        list_serial_ports()

    elif options.port != "":
        if options.boot_source:
            source = options.boot_source.lower()
            set_boot_source(options.port, source)
            
        elif options.hex_file:
            send(options.port, options.hex_file)

        elif options.pgz_file:
            send_pgz(options.port, options.pgz_file)

        elif options.pgx_file:
            send_pgx(options.port, options.pgx_file)

        elif options.wdc_file:
            send_wdc(options.port, options.wdc_file)

        elif options.srec_file:
            send_srec(options.port, options.srec_file)

        elif options.deref_name and options.label_file:
            address = dereference(options.port, options.label_file, options.deref_name)
            get(options.port, address, options.count)

        elif options.lookup_name and options.label_file:
            address = lookup(options.label_file, options.lookup_name)
            get(options.port, address, options.count)

        elif options.dump_address:
            get(options.port, options.dump_address, options.count)

        elif options.revision:
            rev = revision(options.port)
            print(rev)

        elif options.address and options.binary_file:
            upload_binary(options.port, options.binary_file, options.address)

        elif options.address and options.flash_file:
            if options.flash_sector:
                # If sector number provided, program just that sector
                program_flash_sector(options.port, options.flash_file, options.flash_sector)
            else:
                # Otherwise, program the entire flash memory
                program_flash(options.port, options.flash_file, options.address)

        elif options.tcp_host_port:
            tcp_bridge(options.tcp_host_port, options.port)

        elif options.bulk_file:
            program_flash_bulk(options.port, options.bulk_file)

        else:
            parser.print_help()
    else:
        parser.print_help()
finally:
    print
