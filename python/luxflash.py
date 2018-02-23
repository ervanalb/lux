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
    BOOTLOADER_ADDR = 0x80000001
    VALID_IDS = {"WS2811 LED Strip Bootloader", "LPD6803 LED Strip Bootloader", "Lux Bootloader"}

    PAGE_SIZE = 1024

    BL_CMD_INVALIDATEAPP = 0x80
    BL_CMD_BASEADDR = 0x81
    BL_CMD_ERASE = 0x82
    BL_CMD_WRITE = 0x83
    BL_CMD_READ = 0x84

    def __init__(self, bus, *args, **kwargs):
        super(BootloaderDevice, self).__init__(bus, self.BOOTLOADER_ADDR, *args, **kwargs)

    def trigger_bootloader(self, addr = None, retry = 5):
        """ Check to see if device is in bootloader mode. """
        try:
            self.type_id = str(self.get_id())
        except lux.TimeoutError:
            self.type_id = None

        print "Found device: '{}'".format(self.type_id)

        if self.type_id not in self.VALID_IDS:
            if addr is None:
                raise lux.DeviceTypeError("Device not in bootloader mode: {} @ 0x{:08x}".format(self.type_id, self.address))
            # Try triggering bootloader
            dev = lux.Device(self.bus, addr)
            #for i in range(retry):
            dev.bootloader()
            time.sleep(0.1)
            self.type_id = str(self.get_id())
            if self.type_id not in self.VALID_IDS:
                raise lux.DeviceTypeError("Could not put device into bootloader mode: {} @ 0x{:08x}".format(self.type_id, addr))

    def trigger_reset(self, retry = 5):
        for i in range(retry):
            self.reset()
        time.sleep(0.05)

    def invalidate_app(self):
        """ Have the bootloader invalidate the app.

        The bootloader can invalidate the target app by overwriting
        the first page of flash, which contains the app's ISR table
        and reset vector. 

        Call this first before starting to flash a new image."""
        self.ack_command(self.BL_CMD_INVALIDATEAPP, b"")

    def flash_baseaddr(self, addr):
        """ Set the base address for read & write commands """
        self.ack_command(self.BL_CMD_BASEADDR, struct.pack("I", addr))

    def flash_erase(self, addr):
        """ Erase a page of flash at location `addr` """
        self.ack_command(self.BL_CMD_ERASE, struct.pack('I', addr))

    def flash_read(self, page_no):
        """ Read a chunk of memory from the device around baseaddr """
        return self.command(self.BL_CMD_READ, b"", index=page_no)

    class ValidationError(Exception):
        pass

    def flash_write(self, page_no, data):
        """ Write a chunk of data to flash memory at `addr` """
        assert len(data) <= 1024, ValueError("Invalid length, must be <=1024")

        result, crc = self.command(self.BL_CMD_WRITE, data, index=page_no, get_crc=True)

        if result != data:
            if result[1:5] == crc:
                raise lux.CommandError("Flash write command returned error code {}".format(result[0]))
            else:
                raise lux.ValidationError("Validate after write failed: {}, page #{}".format(len(data), page_no + 1))

def flash_device(dev, lux_address, binary, start_address):
    CHUNK_SIZE = 512
    data = binary.read()
    print "Loaded binary file. Len: {} (0x{:04x})".format(len(data), len(data))
    print "Start address: ", hex(start_address)
    print "End address: ", hex(start_address + len(data))

    print "Checking for bootloader on 0x{:x}...".format(lux_address)
    dev.trigger_bootloader(lux_address)
    print "Success!"

    print "Invalidating app..."
    dev.invalidate_app()

    dev.flash_baseaddr(start_address)
    PAGE_SIZE = 1024
    total_page_count = (len(data) + PAGE_SIZE - 1) / PAGE_SIZE
    index = 0
    while data:
        page, data = data[:PAGE_SIZE], data[PAGE_SIZE:]
        print "Writing flash {}/{} ({} bytes)".format(index + 1, total_page_count, len(page))
        retry = 5
        for i in range(retry):
            try:
                dev.flash_erase(start_address + PAGE_SIZE * index)
                dev.get_id()
                dev.flash_write(index, page)
                break
            except lux.CommandError as e:
                print e
                time.sleep(5)
        else:
            raise lux.CommandError("Unable to flash page #{}".format(index + 1))
        time.sleep(1)
        index += 1

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
