#include "hal.h"
#include "lux.h"
#include "strip.h"

#include "stm32f0xx.h"

#define BUSYWAIT() for(volatile long i = 0; i < 1000000; i++)

const char id[]="WS2811 LED Strip";
#define ID_SIZE (sizeof(id)-1)

#define UNICAST_ADDRESS_COUNT 32
uint32_t unicast_addresses[UNICAST_ADDRESS_COUNT];
uint32_t multicast_address_mask;

uint8_t match_destination(uint8_t* dest);
void rx_packet();

// First byte of packet is command

enum lux_command {
    CMD_FRAME,
    CMD_READID,
    CMD_LED,
    CMD_BUTTON,
    CMD_SETADDR,
    CMD_BOOTLOADER,
};

// Has the button been pressed since the last query?
char button_pressed;

void main()
{
    init();
    strip_init();
    lux_init();

    lux_fn_match_destination = &match_destination;
    lux_fn_rx = &rx_packet;

    // Special Init
    button_pressed = 0;
    multicast_address_mask = 0x00000000;
    memset(unicast_addresses, 0xFF, sizeof(unicast_addresses));

    for(;;)
    {
        lux_codec();
        button_pressed |= button();
    }
}

uint8_t match_destination(uint8_t* dest)
{
    //return !!(*(uint32_t*)dest & 0x01);
    uint32_t addr = *(uint32_t *)dest;
    if(addr & multicast_address_mask)
        return 1;
    for(int i = 0; i < UNICAST_ADDRESS_COUNT; i++){
        if(addr == unicast_addresses[i])
            return 1;
    }
    return 0;
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
        BUSYWAIT();
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
        case CMD_READID:
            BUSYWAIT();
            send_id();
        break;
        case CMD_LED:
            if(lux_packet_length == 2){
                if(lux_packet[1])
                    led_on();
                else
                    led_off();
            }
            lux_packet_in_memory = 0;
        break;
        case CMD_BUTTON:
            BUSYWAIT();
            lux_stop_rx();
            clear_destination();
            lux_packet_length = 1;
            lux_packet[0] = button_pressed;
            button_pressed = 0;
            lux_packet_in_memory = 0;
            lux_start_tx();
        break;
        case CMD_SETADDR:
            if(lux_packet_length == 4*(UNICAST_ADDRESS_COUNT+1)){
                multicast_address_mask = *(uint32_t *) lux_packet;
                memcpy(unicast_addresses, lux_packet+4, 4*UNICAST_ADDRESS_COUNT);
            }
            lux_packet_in_memory = 0;
        break;
        case CMD_BOOTLOADER:
            //TODO
            lux_packet_in_memory = 0;
        break;
    }
}
