#include "lux.h"
#include "lux_hal_unix.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint8_t lux_fn_match_destination(uint8_t * dest) {
    return 1;
}

void lux_fn_rx() {
    printf("Recieved packet for %#08X\n; cmd=%#02X; idx=%d; plen=%d\n",
            lux_packet.destination, lux_packet.command, lux_packet.index, lux_packet.payload_length);
    printf("%.*s\n", lux_packet.payload_length, lux_packet.payload);
}

int main(int argc, char ** argv) {
    if (lux_hal_init() != 0) return -1;
    lux_init();

    lux_stop_rx();
    memset(lux_packet.destination, 0xFF, 4);
    lux_packet.command = CMD_GET_ID;
    lux_packet.index = 0;
    lux_packet.payload_length = 0;
    lux_start_tx();

    for (;;) {
        lux_codec();
    }
    return 0;
}
