lux
===

Lux is a protocol for the transmission of power and data from a host device to one or more nodes. It is capable of data rates approaching 3 megabits per second and supplying 30 Watts of power over distances of up to 100 feet. It is designed to be easy to implement and uses low-cost cabling and connectors.

For tech-specs of the protocol, see SPECS.md.

In this repository, you will find:

Library
-------

The lux library is a architecture-independent implementation of the lux in C which you can use to implement your own lux nodes. Simply provide the necessary simple hardware abstraction layer, and you're good to go. The implementation fits in under 8K of program memory and 2K of RAM (1K of which is shared with the application) on a Cortex-M0.

LED strip firmware
------------------

This is a real-world instantiation of the lux protocol to control WS2811 or LPD6803 light strips. It uses a STM32F030 at 48 MHz, which has only 32K of FLASH and 4K of RAM for all of the processing. The code also implements a bootloader for firmware upgrades over lux.

Python
------

There is also a python implementation of a lux master using pyserial.
