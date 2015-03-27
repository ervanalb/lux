import lux

import argparse
import cmd
import json
import os
import shutil
import struct
import sys
import time

hexi = lambda x: int(x.rpartition("0x")[2], 16)

ADDR_HOST  = 0x00000000
ADDR_BL    = 0x80000000
ADDR_NEW   = 0x7FFFFFFF
ADDR_ALL   = 0xFFFFFFFF
MASK_UNI   = 0x0000FFFF
MASK_MULTI = 0x0FFF0000

parser = argparse.ArgumentParser(description="Flash using Lux Bootloader")
parser.add_argument('bus', type=str, help="Serial port of Lux Bus", nargs="?", default="/dev/ttyUSB0")
parser.add_argument('db_json', metavar="db.json", type=str, help="JSON file with known devices", nargs="?", default="db.json")

def write_json(filename, data):
    if os.path.exists(filename):
        shutil.copy(filename, filename + ".bkp")
    with open(filename, "w") as f:
        json.dump(data, f, indent=4, sort_keys=True)
        print "Wrote JSON '{}'".format(filename)

EMPTY_DB = {"devices": []}

def read_json(filename):
    if not os.path.exists(filename):
        print "Warning: JSON file '{}' does not exist. Using empty object.".format(filename)
        return EMPTY_DB
    with open(filename) as f:
        return json.load(f)

class EnumCmd(cmd.Cmd):
    """ Enumerate available lux devices on the bus """
    prompt = "> "

    def __init__(self, lux_bus, json_filename, *args, **kwargs):
        self.lux_bus = lux_bus
        self.json_filename = json_filename
        self.db = read_json(json_filename)
        #super(EnumCmd, self).__init__(*args, **kwargs)
        cmd.Cmd.__init__(self, *args, **kwargs)

    def get_new_address(self):
        existing = {ADDR_HOST, ADDR_NEW, ADDR_BL, ADDR_ALL}
        for dev in self.db["devices"].items():
            existing = existing.union(dev["addresses"])
        for i in range(MASK_UNI):
            if i not in existing:
                return i
        raise Exception("Address space entirely in use!")

    def do_new(self, line):
        """ Check bus for any new devices """
        ld = lux.LuxDevice(self.lux_bus, ADDR_NEW)
        try:
            print "Querying bus..."
            id_str = ld.get_id()
        except lux.LuxDecodeError:
            print "No new device. (0x{:08x})".format(ADDR_NEW)
            return 
        print "Found new device. ID: '{}'".format(id_str)

        new_addr = self.get_new_address()
        multi_mask, multi_addr, uni_addrs = ld.get_address()
        uni_addrs = [new_addr] + [ADDR_ALL] * (len(uni_addrs) - 1)
        ld.set_address(multi_mask, multi_addr, uni_addrs)

        print "Configured device with primary address 0x{:08x}".format(new_addr)
        dev = {
            "pk": new_addr,
            "type": id_str,
            "multicast_mask": multi_mask,
            "multicast_addr": multi_mask,
            "addresses": uni_addrs,
        }
        self.db["devices"][new_addr] = dev
        self.do_save()

    def do_save(self, line=''):
        """ Save JSON database of devices to disk """
        if line:
            self.json_filename = line
        write_json(self.json_filename, self.db)

    def do_reload(self, line=None):
        """ Reload JSON database of devices """
        if line:
            self.json_filename = line
        self.db = read_json(self.json_filename)

    def do_EOF(self, line):
        self.do_save()
        return True

if __name__ == "__main__":
    args = parser.parse_args()
    bus = lux.LuxBus(args.bus)
    cl = EnumCmd(bus, args.db_json)
    cl.cmdloop()
