#include "hal.h"
#include "lux.h"
#include <string.h>

#define BUSYWAIT() for(volatile long i = 0; i < 1000000; i++)

const char id[]="LUX Bootloader";
#define ID_SIZE (sizeof(id)-1)

uint8_t match_destination(uint8_t* dest);
void rx_packet();

// First byte of packet is command

//enum lux_command {
//};

void main()
{
    init();

    lux_init();

    lux_fn_match_destination = &match_destination;
    lux_fn_rx = &rx_packet;

    for(;;)
    {
        lux_codec();
    }
}

uint8_t match_destination(uint8_t* dest)
{
    return *(uint32_t*)dest == 0xFFFFFFFF || *(uint32_t*)dest == 0x8000000;
}

static void clear_destination()
{
    *(uint32_t*)lux_destination = 0;
}

static void send_id()
{
    lux_stop_rx();
    clear_destination();
    lux_packet_length = ID_SIZE;
    memcpy(lux_packet, id, ID_SIZE);
    lux_packet_in_memory = 0;
    lux_start_tx();
}

void rx_packet()
{
    if(lux_packet_length == 0)
    {
        BUSYWAIT();
        send_id();
    }

    switch(lux_packet[0])
    {
    }
}
