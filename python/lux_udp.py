#!/usr/bin/env python

import lux
import socket
import select
import struct

def run_lux_udp(host, port, dev):
    with lux.Bus(dev) as bus:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
        sock.bind((host, port))
        last_addr = None

        while True:
            inputs, outputs, errors = select.select([sock.fileno(), bus.s.fileno()], [], [])
            if sock.fileno() in inputs:
                packet, last_addr = sock.recvfrom(1024)
                if len(packet) < 4: 
                    print("warning: short packet")
                    continue
                #print("> {}".format(len(packet)))
                destination, body = struct.unpack("<I", packet[:4])[0], packet[4:]
                bus.write(destination, body)
            if bus.s.fileno() in inputs:
                packet = bus.read()
                if packet is None: continue

                #print("< @{} {}".format(packet[0], len(packet[1])))
                #print(packet[0])
                destination, body = packet
                sock.sendto(struct.pack("<I", destination) + body, 0, last_addr)

if __name__ == "__main__":
    run_lux_udp(host="0.0.0.0", port=1365, dev="/dev/ttyACM0")
