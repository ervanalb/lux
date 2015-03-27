#!/usr/bin/env python

import lux
import sys
import time
import struct
import argparse


hexi = lambda x: int(x.rpartition("0x")[2], 16)

#BOOTLOADER_ADDR = 0x80000000
BOOTLOADER_ADDR = 0xFFFFFFFF

parser = argparse.ArgumentParser(description="Flash using Lux Bootloader")
parser.add_argument('bus', type=str, help="Serial port of Lux Bus")
parser.add_argument('addr', type=hexi, help="Start address memory")
parser.add_argument('hexfile', type=open, help="Binary file to flash to chip")
parser.add_argument('luxaddr', type=hexi, help="Lux address to flash", nargs='?', default=BOOTLOADER_ADDR)


class LuxBootloaderDevice(lux.LuxDevice):
    VALID_IDS = {"WS2811 LED Strip Bootloader", "LPD6803 LED Strip Bootloader"}

    PAGE_SIZE = 1024

    BL_CMD_INVALIDATEAPP = 0x80
    BL_CMD_ERASE = 0x81
    BL_CMD_WRITE = 0x82
    BL_CMD_READ = 0x83

    def __init__(self, *args, **kwargs):
        super(LuxBootloaderDevice, self).__init__(*args, **kwargs)

    def trigger_bootloader(self, address=None):
        """ Cause device at specified address to jump to bootloader"""
        if address is None:
            address = self.address
        self.bus.send_command(address, self.CMD_BOOTLOADER)

    def assert_bootloader(self):
        """ Check to see if device is in bootloader mode. """
        self.type_id = self.get_id()
        if self.type_id not in self.VALID_IDS:
            # Try triggering bootloader
            self.bootloader()
            time.sleep(0.5)
            self.type_id = self.get_id()
            if self.type_id not in self.VALID_IDS:
                raise lux.DeviceTypeError("Device not in bootloader mode: {} @ 0x{:08x}".format(result, self.address))

    def invalidate_app(self):
        """ Have the bootloader invalidate the app.

        The bootloader can invalidate the target app by overwriting
        the first page of flash, which contains the app's ISR table
        and reset vector. 

        Call this first before starting to flash a new image."""
        self.command_ack(self.BL_CMD_INVALIDATEAPP)

    def flash_erase(self, addr):
        """ Erase a page of flash at location `addr` """
        raw_data = struct.pack('I', addr)
        self.command_ack(self.BL_CMD_ERASE, raw_data)

    def flash_read(self, addr, length):
        """ Read a chunk of memory from the device at `addr` """
        assert 2 <= length <= 1024, ValueError("Invalid length, must be <=1024")
        raw_data = struct.pack('IH', addr, length)
        result = self.command_response(self.BL_CMD_READ, data=raw_data)
        if len(result) == 1:
            raise lux.LuxCommandError("Unable to read flash {}@0x{:08x}, errno {}".format(length, addr, ord(result[0])))
        return result

    def flash_write(self, addr, data):
        """ Write a chunk of data to flash memory at `addr` """
        assert len(data) <= 1016, ValueError("Invalid length, must be <=1016")
        raw_data = struct.pack("IH", addr, len(data)) + data

        result = self.command_response(self.BL_CMD_WRITE, data=raw_data)

        if result[0] != '\x00':
            raise lux.LuxCommandError("Unable to write flash {}@0x{:08x}, errno {}".format(length, addr, ord(result[0])))

        if result[1:] != data:
            raise lux.LuxCommandError("Validate after write failed: {}@0x{:08x}".format(length, addr))

def flash_device(dev, binary, start_address):
    CHUNK_SIZE = 512
    data = binary.read()
    print "Loaded binary file. Len: {} (0x{:04x})".format(len(data), len(data))
    print "Start address: ", hex(start_address)
    print "End address: ", hex(start_address + len(data))

    print "Checking for bootloader..."
    dev.assert_bootloader()
    print "Success!"

    print "Invalidating app..."
    dev.invalidate_app()

    firstend = lambda x: x.append(x.pop(0))

    data_pages = zip(*[iter(data)]*dev.PAGE_SIZE)
    addresses = [a * dev.PAGE_SIZE + start_address for a in range(len(data_pages))] 

    # Move first page to end so first page is written last in case of error
    firstend(data_pages)
    firstend(addresses)

    for data_page, addr in zip(data_pages, addresses):
        print "Erasing and writing {:08x}...".format(addr)
        dev.flash_erase(addr)
        while data_page:
            chunk, data_page = data_page[:CHUNK_SIZE], data_page[CHUNK_SIZE:]
            for i in range(5):
                print "Writing flash", hex(addr), len(chunk)
                try:
                    dev.flash_write(addr, ''.join(chunk))
                except Exception as e:
                    print e
                    print "Writing flash 0x{:08x} failed (no response)".format(addr)
                    continue
                break
            else:
                raise Exception
            addr += CHUNK_SIZE

    print "Done writing; resetting"
    dev.reset()
    time.sleep(0.5)
    print "Checking response..."
    print dev.get_id()

if __name__ == "__main__":
    args = parser.parse_args()

    bus = lux.LuxBus(args.bus)
    print "Looking for LED strip..."
    try:
        ldev = lux.LEDStrip(bus, args.luxaddr)
        ldev.set_led(1)
        ldev.send_solid_frame((1,0,0))
        ldev.bootloader()
    except (lux.LuxDecodeError, lux.DeviceTypeError):
        print "Unable to find LED strip, might already be in bootloader mode"

    dev = LuxBootloaderDevice(bus, BOOTLOADER_ADDR)

    flash_device(dev, args.hexfile, args.addr)

    
