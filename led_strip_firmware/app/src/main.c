#include "hal.h"
#include "lux.h"
#include "strip.h"
#include "config.h"
#include <string.h>

#ifdef WS2811
const char id[]="WS2811 LED Strip";
#elif LPD6803
const char id[]="LPD6803 LED Strip";
#else
const char id[]="Lux LED Strip";
#endif
#define ID_SIZE (sizeof(id)-1)

void main()
{
    init();

    strip_init();
    lux_init();
    read_config_from_flash();

    //lux_fn_match_destination = &match_destination;
    //lux_fn_rx = &rx_packet;

    for(;;)
    {
        lux_codec();
    }
}

uint8_t lux_fn_match_destination(uint8_t* dest)
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
    if(addr == 0x80000000 && button())
        return 1;
    return addr == 0xFFFFFFFF;
}

static void clear_destination()
{
    memset(lux_packet.destination, 0, sizeof(lux_packet.destination));
}

static void send_ack_crc()
{
    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = 5;
    lux_packet.payload[0] = 0;
    memcpy(&lux_packet.payload[1], lux_packet.crc, sizeof(lux_packet.crc));

    lux_start_tx();
}

static void send_nak()
{
    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = 5;
    lux_packet.payload[0] = 255;
    memcpy(&lux_packet.payload[1], lux_packet.crc, sizeof(lux_packet.crc));

    lux_start_tx();
}

static void send_id()
{
    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = ID_SIZE;
    memcpy(lux_packet.payload, id, ID_SIZE);

    lux_start_tx();
}

static void send_button()
{
    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = 4;
    uint16_t button_count = button();
    memcpy(lux_packet.payload, &button_count, sizeof(button_count));

    lux_start_tx();
}

static uint8_t set_frame()
{
    if(lux_packet.payload_length != 3*cfg.strip_length) return 1;
    if(!strip_ready()) return 2;
    strip_write(lux_packet.payload);
    return 0;
}

static void set_frame_ack()
{
    if(set_frame() == 0)
        send_ack_crc();
    else
        send_nak();
}

static void set_led()
{
    if(lux_packet.payload_length != 1) goto fail;
    if(lux_packet.payload[0])
    {
        led_on();
    }
    else
    {
        led_off();
    }
    send_ack_crc();
    return;
fail:
    send_nak();
}

static void send_addresses()
{
    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = 4*(UNICAST_ADDRESS_COUNT + 2);
    memcpy(&lux_packet.payload[0], &cfg.multicast_address, 4);
    memcpy(&lux_packet.payload[4], &cfg.multicast_address_mask, 4);
    memcpy(&lux_packet.payload[8], &cfg.unicast_addresses, 4*UNICAST_ADDRESS_COUNT);

    lux_start_tx();
}

static void set_addresses()
{
    if(lux_packet.payload_length != 4*(UNICAST_ADDRESS_COUNT+2)) goto fail;

    memcpy(&cfg.multicast_address, &lux_packet.payload[0], 4);
    memcpy(&cfg.multicast_address_mask, &lux_packet.payload[4], 4);
    memcpy(cfg.unicast_addresses, &lux_packet.payload[8], 4*UNICAST_ADDRESS_COUNT);

    SOFT_WRITE_CONFIG;
    send_ack_crc();
    return;
fail:
    send_nak();
}

static void set_strip_length()
{
    if(lux_packet.payload_length != sizeof(cfg.strip_length)) goto fail;

    uint16_t l = 0;
    memcpy(&l, lux_packet.payload, sizeof(l));
    if(l > MAX_STRIP_LENGTH) goto fail;
    cfg.strip_length = l;

    SOFT_WRITE_CONFIG;
    send_ack_crc();
    return;
fail:
    send_nak();
}

static void send_strip_length()
{
    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = sizeof(cfg.strip_length);
    memcpy(lux_packet.payload, &cfg.strip_length, sizeof(cfg.strip_length));

    lux_start_tx();
}

static void send_packet_counters()
{
    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = sizeof(lux_counters);
    memcpy(lux_packet.payload, &lux_counters, sizeof(lux_counters));

    lux_start_tx();
}

static void reset_packet_counters()
{
    lux_reset_counters();
    send_ack_crc();
}

void lux_fn_rx()
{
    // Currently, the lux packet gets entirely processed in this function
    // so we can release lux_packet immediately
    lux_packet_in_memory = 0;

    switch(lux_packet.command)
    {
        case LUX_CMD_GET_ID:
        case LUX_CMD_GET_DESCRIPTOR:
            send_id();
            break;
        case LUX_CMD_RESET:
            if (lux_packet.payload[0] & 0x1)
                bootloader(); // Never returns
            else
                reset(); // Never returns
            break;
        case LUX_CMD_FRAME:
        case LUX_CMD_FRAME_HOLD:
            set_frame();
            break;
        case LUX_CMD_FRAME_ACK:
        case LUX_CMD_FRAME_HOLD_ACK:
            set_frame_ack();
            break;
        case LUX_CMD_COMMIT_CONFIG:
            write_config_to_flash();
            send_ack_crc();
            break;
        case LUX_CMD_SET_LED:
            set_led();
            break;
        case LUX_CMD_GET_BUTTON_COUNT:
            send_button();
            break;
        case LUX_CMD_GET_ADDR:
            send_addresses();
            break;
        case LUX_CMD_SET_ADDR:
            set_addresses();
            break;
        case LUX_CMD_SET_LENGTH:
            set_strip_length();
            break;
        case LUX_CMD_GET_LENGTH:
            send_strip_length();
            break;
        case LUX_CMD_GET_PKTCNT:
            send_packet_counters();
            break;
        case LUX_CMD_RESET_PKTCNT:
            reset_packet_counters();
            break;
    }
}
