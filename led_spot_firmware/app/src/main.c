#include "hal.h"
#include "lux.h"
#include "config.h"
#include <string.h>

const char id[]="LED Spot";
#define ID_SIZE (sizeof(id)-1)

uint8_t match_destination(uint8_t* dest);
void rx_packet();

static union lux_command_frame *luxf = (union lux_command_frame *)lux_packet;

void main()
{
    init();

    lux_init();
    read_config_from_flash();

    lux_fn_match_destination = &match_destination;
    lux_fn_rx = &rx_packet;

    for(;;)
    {
        lux_codec();
    }
}

uint8_t match_destination(uint8_t* dest)
{
    //return !!(*(uint32_t*)dest & 0x01);
    uint32_t addr = *(uint32_t *)dest;
    // Never respond on 0x00000000
    if(addr == 0) return 0; 
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

static void send_ack()
{
    lux_stop_rx();
    clear_destination();

    lux_packet_length = 0;

    lux_start_tx();
}

static void send_id()
{
    lux_stop_rx();
    clear_destination();

    lux_packet_length = ID_SIZE;
    memcpy(lux_packet, id, ID_SIZE);

    lux_start_tx();
}

static uint8_t set_color()
{
    if(lux_packet_length != 4) return 1;
    spot_color(lux_packet[1], lux_packet[2], lux_packet[3]);
    return 0;
}

static void set_color_ack()
{
    if(!set_color())
    {
        send_ack();
    }
}

static uint8_t set_led()
{
    if(lux_packet_length != 2) return 1;
    if(luxf->csingle.data)
    {
        led_on();
    }
    else
    {
        led_off();
    }
    send_ack();
}

static void send_addresses()
{
    uint32_t *warray = lux_packet;
    lux_stop_rx();
    clear_destination();

    lux_packet_length = 4*(UNICAST_ADDRESS_COUNT +2);
    memcpy(warray+0, &cfg.multicast_address, 4);
    memcpy(warray+1, &cfg.multicast_address_mask, 4);
    memcpy(warray+2, &cfg.unicast_addresses, 4*UNICAST_ADDRESS_COUNT);

    lux_start_tx();
}

static void set_addresses()
{
    if(lux_packet_length != 4*(UNICAST_ADDRESS_COUNT+2)+1) return 1;

    cfg.multicast_address = luxf->warray.data[0];
    cfg.multicast_address_mask = luxf->warray.data[1];
    memcpy(&cfg.unicast_addresses, &luxf->warray.data[2], 4*UNICAST_ADDRESS_COUNT);

    SOFT_WRITE_CONFIG;
    send_ack();
}

static void set_userdata()
{
    if(lux_packet_length > USERDATA_SIZE+1) return 1;
    memcpy(cfg.userdata, luxf->carray.data, lux_packet_length-1);

    SOFT_WRITE_CONFIG;
    send_ack();
}

static void send_userdata()
{
    lux_stop_rx();
    clear_destination();

    lux_packet_length = USERDATA_SIZE;
    memcpy(lux_packet, cfg.userdata, USERDATA_SIZE);

    lux_start_tx();
}

static void send_packet_counters()
{
    uint32_t *d = (uint32_t *) lux_packet;

    lux_stop_rx();
    clear_destination();

    lux_packet_length = 4 * 5;

    *d++ = lux_good_packet_counter;
    *d++ = lux_malformed_packet_counter;
    *d++ = lux_packet_overrun_counter;
    *d++ = lux_bad_checksum_counter;
    *d++ = lux_rx_interrupted_counter;

    lux_start_tx();
}

static void reset_packet_counters()
{
    lux_reset_counters();
    send_ack();
}

void rx_packet()
{
    // Currently, the lux packet gets entirely processed in this function
    // so we can release lux_packet immediately
    lux_packet_in_memory = 0;

    if(lux_packet_length == 0)
    {
        send_id();
        return;
    }

    switch(luxf->ssingle.cmd)
    {
        case LUX_CMD_RESET:
            reset(); // Never returns
        case LUX_CMD_BOOTLOADER:
            bootloader(); // Never returns
        case LUX_CMD_COLOR:
            set_color();
            break;
        case LUX_CMD_COLOR_ACK:
            set_color_ack();
            break;
        case LUX_CMD_WRITE_CONFIG:
            write_config_to_flash();
            break;
        case LUX_CMD_GET_ID:
            send_id();
            break;
        case LUX_CMD_SET_LED:
            set_led();
            break;
        case LUX_CMD_GET_ADDR:
            send_addresses();
            break;
        case LUX_CMD_SET_ADDR:
            set_addresses();
            break;
        case LUX_CMD_SET_USERDATA:
            set_userdata();
            break;
        case LUX_CMD_GET_USERDATA:
            send_userdata();
            break;
        case LUX_CMD_GET_PKTCNT:
            send_packet_counters();
            break;
        case LUX_CMD_RESET_PKTCNT:
            reset_packet_counters();
            break;
    }
}
