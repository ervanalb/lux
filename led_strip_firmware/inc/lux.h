#ifndef __LUX_H
#define __LUX_H

#include <stdint.h>

// Run lux_init once at the start of the program
void lux_init();

// Run lux_codec frequently (polling)
void lux_codec();

// Run lux_stop_rx before filling the packet memory with data to transmit
void lux_stop_rx();

// Run lux_send to send a packet
void lux_start_tx();

#define LUX_CHECKSUM_SIZE 4
#define LUX_DESTINATION_SIZE 4
#define LUX_PACKET_MEMORY_SIZE 1024
#define LUX_PACKET_MEMORY_ALLOCATED_SIZE (LUX_PACKET_MEMORY_SIZE+LUX_CHECKSUM_SIZE)

extern uint8_t lux_destination[LUX_DESTINATION_SIZE];
extern uint8_t lux_packet[LUX_PACKET_MEMORY_ALLOCATED_SIZE];

extern int lux_packet_length;

extern int lux_malformed_packet_counter;
extern int lux_packet_overrun_counter;
extern int lux_bad_checksum_counter;
extern int lux_rx_while_tx_counter;

extern uint8_t lux_packet_in_memory;

// Functions that need to be provided (in addition to lux_hal.h)
uint8_t lux_fn_match_destination(); // Called to see if a packet destination matches this device's
void lux_fn_rx(); // Called when a packet has been received

#endif
