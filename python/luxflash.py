#!/usr/bin/env python

import lux
import sys
import time
import struct
import argparse


hexi = lambda x: int(x.rpartition("0x")[2], 16)


parser = argparse.ArgumentParser(description="Flash using Lux Bootloader")
parser.add_argument('bus', type=str, help="Serial port of Lux Bus")
parser.add_argument('addr', type=hexi, help="Start address memory")
parser.add_argument('hexfile', type=open, help="Binary file to flash to chip")
parser.add_argument('luxaddr', type=hexi, help="Lux address to flash", nargs='?', default=lux.Bus.MULTICAST)

class BootloaderDevice(lux.Device):
    BOOTLOADER_ADDR = 0x80000000
    VALID_IDS = {"WS2811 LED Strip Bootloader", "LPD6803 LED Strip Bootloader"}

    PAGE_SIZE = 1024

    BL_CMD_INVALIDATEAPP = chr(0x80)
    BL_CMD_ERASE = chr(0x81)
    BL_CMD_WRITE = chr(0x82)
    BL_CMD_READ = chr(0x83)

    def __init__(self, bus, *args, **kwargs):
        super(BootloaderDevice, self).__init__(bus, self.BOOTLOADER_ADDR, *args, **kwargs)

    def trigger_bootloader(self, addr = None, retry = 3):
        """ Check to see if device is in bootloader mode. """
        self.type_id = self.get_id()
        if self.type_id not in self.VALID_IDS:
            if addr is None:
                raise lux.DeviceTypeError("Device not in bootloader mode: {} @ 0x{:08x}".format(result, self.address))
            # Try triggering bootloader
            dev = lux.Device(self.bus, addr)
            for i in range(retry):
                dev.bootloader()
            time.sleep(0.05)
            self.type_id = self.get_id()
            if self.type_id not in self.VALID_IDS:
                raise lux.DeviceTypeError("Could not put device into bootloader mode: {} @ 0x{:08x}".format(result, addr))

    def trigger_reset(self, retry = 3):
        for i in range(retry):
            self.reset()
        time.sleep(0.05)

    def invalidate_app(self, *args, **kwargs):
        """ Have the bootloader invalidate the app.

        The bootloader can invalidate the target app by overwriting
        the first page of flash, which contains the app's ISR table
        and reset vector. 

        Call this first before starting to flash a new image."""
        self.ack_command(self.BL_CMD_INVALIDATEAPP, *args, **kwargs)

    def flash_erase(self, addr, *args, **kwargs):
        """ Erase a page of flash at location `addr` """
        self.ack_command(self.BL_CMD_ERASE + struct.pack('I', addr), *args, **kwargs)

    def flash_read(self, addr, length, *args, **kwargs):
        """ Read a chunk of memory from the device at `addr` """
        assert 0 <= length <= 1024, ValueError("Invalid length, must be <=1024")
        return self.command(self.BL_CMD_READ + struct.pack('IH', addr, length), *args, **kwargs)

    def flash_write(self, addr, data, *args, **kwargs):
        """ Write a chunk of data to flash memory at `addr` """
        assert len(data) <= 1016, ValueError("Invalid length, must be <=1016")

        result = self.command(self.BL_CMD_WRITE + struct.pack("IH", addr, len(data)) + data, *args, **kwargs)

        if result != data:
            raise lux.CommandError("Validate after write failed: {}@0x{:08x}".format(length, addr))

def flash_device(dev, lux_address, binary, start_address):
    CHUNK_SIZE = 512
    data = binary.read()
    print "Loaded binary file. Len: {} (0x{:04x})".format(len(data), len(data))
    print "Start address: ", hex(start_address)
    print "End address: ", hex(start_address + len(data))

    print "Checking for bootloader..."
    dev.trigger_bootloader(lux_address)
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
            print "Writing flash", hex(addr), len(chunk)
            dev.flash_write(addr, ''.join(chunk))
            addr += CHUNK_SIZE

    print "Done writing; resetting"
    dev.reset()
    print "Checking response..."
    new_device = lux.Device(dev.bus, lux_address)
    print new_device.get_id()

if __name__ == "__main__":
    args = parser.parse_args()

    with lux.Bus(args.bus) as bus:
        dev = BootloaderDevice(bus)
        flash_device(dev, args.luxaddr, args.hexfile, args.addr)
