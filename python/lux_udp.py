#!/usr/bin/env python

import lux
import socket
import select
import struct

def run_lux_udp(host, port, dev):
    bus = lux.LuxBus(dev)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
    sock.bind((host, port))

    while True:
        inputs, outputs, errors = select.select([sock.fileno(), bus.s.fileno()], [], [])
        if sock.fileno() in inputs:
            packet = sock.recv(1024)
            if len(packet) < 4: 
                print "warning: short packet"
                continue
            destination, body = struct.unpack("<I", packet[:4])[0], packet[4:]
            bus.send_packet(destination, body)
        if bus.s.fileno() in inputs:
            packet = bus.read()
            if packet is None: continue

            destination, body = struct.pack("<I", packet[:4]), packet[4:]
            sock.send(packet)

if __name__ == "__main__":
    run_lux_udp(host="0.0.0.0", port=1365, dev="/dev/ttyACM0")
