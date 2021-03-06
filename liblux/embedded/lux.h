#ifndef __LUX_H
#define __LUX_H

#include <stdint.h>
#include "lux_cmds.h"

// How many bytes the destination is
#define LUX_DESTINATION_SIZE 4

// How many bytes the longest packet can be
#define LUX_PACKET_MEMORY_SIZE LUX_PACKET_MAX_SIZE

// Number of bytes in the CRC (This is hard to change)
#define LUX_CRC_SIZE 4

// Extra space for CRC
#define LUX_PACKET_MEMORY_ALLOCATED_SIZE (LUX_PACKET_MEMORY_SIZE+LUX_CRC_SIZE)

// Run lux_init once at the start of the program
void lux_init(void);

// Run lux_codec frequently (polling)
void lux_codec(void);

// Run lux_stop_rx before filling the packet memory with data to transmit
void lux_stop_rx(void);

// Run lux_start_tx to send a packet
void lux_start_tx(void);

// Run lux_reset_counters to reset packet counters to zero
void lux_reset_counters(void);

// Useful buffers for the application
struct lux_packet {
    uint32_t destination;
    uint8_t command;
    uint8_t index;
    uint8_t payload[LUX_PACKET_MEMORY_ALLOCATED_SIZE];
    uint16_t payload_length;
    uint8_t crc[LUX_CRC_SIZE];
};

extern struct lux_packet lux_packet;

// Counters for problematic data
extern struct lux_counters lux_counters;

// Flag indicating whether a valid packet has been received
// (must be cleared by application before another packet can be received)
extern uint8_t lux_packet_in_memory;

// --------- Functions that need to be provided: -----------

// This function is called to see if a packet destination matches this device.
uint8_t lux_fn_match_destination(uint32_t dest);

// This function is called when a packet has been received.
// It doesn't have to do anything, but if lux_packet_in_memory
// isn't cleared in a timely fashion, you may drop packets
void lux_fn_rx(void); 


#endif
