#ifndef __LINUX_LUX_H__
#define __LINUX_LUX_H__

#include <stdint.h>
#include "lux_cmds.h"

int lux_serial_open();
int lux_network_open(const char * address_spec, uint16_t port);
void lux_close(int fd);

struct lux_packet {
    uint32_t destination;
    enum lux_command command;
    uint8_t index;
    uint8_t payload[LUX_PACKET_MAX_SIZE];
    uint16_t payload_length;
    uint32_t crc;
};

enum lux_flags {
    LUX_ACK   = (1 << 0),
    LUX_RETRY = (1 << 1),
    LUX_BCAST = (1 << 2),
};

int lux_command(int fd, struct lux_packet * packet, struct lux_packet * response, enum lux_flags flags);

#endif
