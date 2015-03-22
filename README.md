lux
===

Lux is a protocol for the transmission of power and data from a host device to one or more nodes. It is capable of data rates approaching 3 megabits per second and supplying 30 Watts of power over distances of up to 100 feet. It is designed to be easy to implement and uses low-cost cabling and connectors.

Physical Layer
--------------

Lux is designed with a hub-and-spoke model in mind. Each lux universe must have exactly one master but may have an arbitrary number of nodes (limited only by the bandwidth of the bus.)

The master exposes one or more RJ-11 jacks (6p4c) and each node exposes one RJ-11 jack. Devices are connected with straight-through RJ-11 cables no longer than 100 feet and no thinner than 26 AWG per conductor.

The lux pinout is as follows:
 1. GND
 2. A
 3. B
 4. PWR

The voltage that nodes see between PWR and GND is nominally 48 volts, but may be as high as 54 volts or as low as 42 volts. The master or hub provides this power, and nodes consume it. The master should not output less than 48 volts or more than 54 volts. The drop in the cabling may be as large as 6 volts.

Nodes may be bus-powered or self-powered. A bus-powered node must not draw more than 650 mA from the bus. A self-powered node must use GND as reference for the data signals (i.e. must not be floating or at a different potential from the bus.)

RS-485 is used for the data lines. Nodes and masters should implement the recommended bias resistors and a termination resistor of 120 ohms, as well as a RS-485 compatible tranceiver capable of 3 megabit per second communcation. The serial protocol uses 3 megabaud, 8 data bits, 1 start bit, 1 stop bit, and no parity. The serial link is half-duplex.
