#include "hal.h"
#include "lux.h"
#include "strip.h"

#include "stm32f0xx.h"

const char id[]="WS2811 LED Strip";
#define ID_SIZE (sizeof(id)-1)

uint8_t match_destination(uint8_t* dest);
void rx_packet();

// First byte of packet is command

#define CMD_FRAME 0x00

void main()
{
    init();
    strip_init();
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
    return *(uint32_t*)dest == 0xFFFFFFFF;
}

static void clear_destination()
{
    *(uint32_t*)lux_destination = 0;
}

static void send_id()
{
    int i;
    lux_stop_rx();
    clear_destination();
    lux_packet_length = ID_SIZE;
    for(i=0;i<ID_SIZE;i++)
    {
        lux_packet[i]=id[i];
    }
    lux_packet_in_memory = 0;
    lux_start_tx();
}

void rx_packet()
{
    if(lux_packet_length == 0)
    {
        send_id();
    }

    switch(lux_packet[0])
    {
        case CMD_FRAME:
            if(lux_packet_length != 3*STRIP_LENGTH+1) goto skip_frame;
            if(!strip_ready()) goto skip_frame;
            strip_write(&lux_packet[1]);
        skip_frame:
            lux_packet_in_memory=0;
            break;
    }
}
