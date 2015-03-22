#include "hal.h"
#include "lux.h"
#include "strip.h"
#include "config.h"
#include <string.h>

#define BUSYWAIT() for(volatile long i = 0; i < 1000000; i++)

const char id[]="WS2811 LED Strip";
#define ID_SIZE (sizeof(id)-1)

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
    CMD_SETUSERDATA,
    CMD_GETUSERDATA,
};

// Has the button been pressed since the last query?
char button_pressed;

void main()
{
    init();

    strip_init();
    lux_init();
    read_config_from_flash();

    lux_fn_match_destination = &match_destination;
    lux_fn_rx = &rx_packet;

    // Special Init
    button_pressed = 0;

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
    if((addr & cfg.multicast_address_mask) == cfg.multicast_address)
        return 1;
    for(int i = 0; i < UNICAST_ADDRESS_COUNT; i++){
        if(addr == cfg.unicast_addresses[i])
            return 1;
    }
    return addr == 0xFFFFFFFF;
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
            if(lux_packet_length == 4*(UNICAST_ADDRESS_COUNT+2)){
                cfg.multicast_address = *(uint32_t *) lux_packet;
                cfg.multicast_address_mask = *(uint32_t *) lux_packet;
                memcpy(cfg.unicast_addresses, lux_packet+8, 4*UNICAST_ADDRESS_COUNT);
                write_config_to_flash();
            }
            lux_packet_in_memory = 0;
            break;
        case CMD_BOOTLOADER:
            bootloader(); // Never returns
        case CMD_SETUSERDATA:
            if(lux_packet_length <= USERDATA_SIZE+1){
                memcpy(cfg.userdata, lux_packet+1, lux_packet_length-1);
                write_config_to_flash();
            }
            lux_packet_in_memory = 0;
            break;
        case CMD_GETUSERDATA:
            BUSYWAIT();
            lux_stop_rx();
            clear_destination();
            lux_packet_length = USERDATA_SIZE;
            memcpy(lux_packet, cfg.userdata, USERDATA_SIZE);
            lux_packet_in_memory = 0;
            lux_start_tx();
            break;
        default:
            lux_packet_in_memory = 0;
    }
}
