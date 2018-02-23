#ifndef __LUX_DEVICE_H
#define __LUX_DEVICE_H

#include "lux.h"

#define USERDATA_SIZE 900

struct __attribute__((__packed__)) lux_device_config {
    uint16_t strip_length;
    struct lux_addresses addresses;
    char userdata[USERDATA_SIZE];
};
extern struct lux_device_config lux_device_config;

void lux_device_init();
void lux_device_poll();

// Functions that need to be implemented

extern const char lux_device_id[];
extern const uint16_t lux_device_id_length;
extern const uint16_t lux_device_max_strip_length;

// Returns:
#define LUX_DEVICE_REPLY_NONE       0
#define LUX_DEVICE_REPLY_ACK        1
#define LUX_DEVICE_REPLY_NAK        2
#define LUX_DEVICE_REPLY_PACKET     3
uint8_t lux_device_handle_packet(void);

uint8_t lux_device_get_button(void);
void lux_device_set_led(uint8_t);

void lux_device_bootloader();
void lux_device_reset();
void lux_device_read_config();
uint8_t lux_device_write_config();

#endif
