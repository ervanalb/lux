#ifndef __LUX_H
#define __LUX_H

#include <stdint.h>

// How many bytes the destination is
#define LUX_DESTINATION_SIZE 4

// How many bytes the longest packet can be
#define LUX_PACKET_MEMORY_SIZE 1024

// Run lux_init once at the start of the program
void lux_init();

// Run lux_codec frequently (polling)
void lux_codec();

// Run lux_stop_rx before filling the packet memory with data to transmit
void lux_stop_rx();

// Run lux_start_tx to send a packet
void lux_start_tx();

// The packet's destination
extern uint8_t lux_destination[LUX_DESTINATION_SIZE];

// The packet's payload
extern uint8_t lux_packet[];

// The packet's length
extern int lux_packet_length;

// Flag indicating whether a valid packet has been received
// (must be cleared by application before another packet can be received)
extern uint8_t lux_packet_in_memory;

// Counters to keep track of bad things that could happen
extern int lux_malformed_packet_counter;
extern int lux_packet_overrun_counter;
extern int lux_bad_checksum_counter;
extern int lux_rx_interrupted_counter;

// Functions that need to be provided (in addition to lux_hal.h):

// This function is called to see if a packet destination matches this device.
uint8_t lux_fn_match_destination(uint8_t* dest);

// This function is called when a packet has been received.
// It doesn't have to do anything, but if lux_packet_in_memory
// isn't cleared in a timely fashion, you may drop packets
void lux_fn_rx(); 

#endif
