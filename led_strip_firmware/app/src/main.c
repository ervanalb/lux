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

uint8_t match_destination(uint8_t* dest);
void rx_packet();

// Has the button been pressed since the last query?
char button_pressed;

static union lux_command_frame *luxf = (union lux_command_frame *)lux_packet;

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

static void send_ack(uint8_t code){
    lux_stop_rx();
    clear_destination();

    lux_packet_length = 1;
    lux_packet[0] = code;

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

static void send_button(){
    lux_stop_rx();
    clear_destination();

    lux_packet_length = 1;
    lux_packet[0] = button_pressed;

    button_pressed = 0;

    lux_start_tx();
}

static uint8_t set_frame(){
    if(lux_packet_length != 3*cfg.strip_length+1) return 1;
    if(!strip_ready()) return 2;
    strip_write(luxf->carray.data);
    return 0;
}

static uint8_t set_led(){
    if(lux_packet_length != 2) return 1;
    if(luxf->csingle.data)
        led_on();
    else
        led_off();
    return 0;
}

static void send_addresses(){
    lux_stop_rx();
    clear_destination();

    lux_packet_length = 4*(UNICAST_ADDRESS_COUNT +2);
    luxf->warray.data[0] = cfg.multicast_address;
    luxf->warray.data[1] = cfg.multicast_address_mask;
    memcpy(&luxf->warray.data[2], &cfg.unicast_addresses, 4*UNICAST_ADDRESS_COUNT);

    lux_start_tx();
}

static uint8_t set_addresses(){
    if(lux_packet_length != 4*(UNICAST_ADDRESS_COUNT+2)+1) return 1;

    cfg.multicast_address = luxf->warray.data[0];
    cfg.multicast_address_mask = luxf->warray.data[1];
    memcpy(&cfg.unicast_addresses, &luxf->warray.data[2], 4*UNICAST_ADDRESS_COUNT);

    SOFT_WRITE_CONFIG;
    return 0;
}

static uint8_t set_userdata(){
    if(lux_packet_length > USERDATA_SIZE+1) return 1;
    memcpy(cfg.userdata, luxf->carray.data, lux_packet_length-1);

    SOFT_WRITE_CONFIG;
    return 0;
}

static void send_userdata(){
    lux_stop_rx();
    clear_destination();

    lux_packet_length = USERDATA_SIZE;
    memcpy(lux_packet, cfg.userdata, USERDATA_SIZE);

    lux_start_tx();
}

static uint8_t set_strip_length(){
    uint32_t l = 0;
    if(lux_packet_length != sizeof(cfg.strip_length) + 1) return 1;
    if(luxf->ssingle.data > MAX_STRIP_LENGTH) return 2;
    cfg.strip_length = luxf->ssingle.data;

    SOFT_WRITE_CONFIG;
    return 0;
}

static void send_strip_length(){
    lux_stop_rx();
    clear_destination();

    lux_packet_length = sizeof(cfg.strip_length);
    memcpy(lux_packet, &cfg.strip_length, sizeof(cfg.strip_length));

    lux_start_tx();
}

static void send_packet_counters(){
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


void rx_packet() {

    // Currently, the lux packet gets entirely processed in this function
    // so we can release lux_packet immediately
    lux_packet_in_memory = 0;

    if(lux_packet_length == 0){
        send_id();
        return;
    }

    switch(luxf->ssingle.cmd) {
        case CMD_RESET:
            reset(); // Never returns
        case CMD_BOOTLOADER:
            bootloader(); // Never returns
        case CMD_FRAME:
            set_frame();
            break;
        case CMD_FRAME_ACK:
            send_ack(set_frame());
            break;
        case CMD_WRITE_CONFIG:
            write_config_to_flash();
            break;
        case CMD_WRITE_CONFIG_ACK:
            write_config_to_flash();
            send_ack(0);
            break;
        case CMD_GET_ID:
            send_id();
            break;
        case CMD_SET_LED:
            set_led();
            break;
        case CMD_SET_LED_ACK:
            send_ack(set_led());
            break;
        case CMD_GET_BUTTON:
            send_button();
            break;
        case CMD_GET_ADDR:
            send_addresses();
            break;
        case CMD_SET_ADDR:
            set_addresses();
            break;
        case CMD_SET_ADDR_ACK:
            send_ack(set_addresses());
            break;
        case CMD_SET_USERDATA:
            set_userdata();
            break;
        case CMD_SET_USERDATA_ACK:
            send_ack(set_userdata());
            break;
        case CMD_GET_USERDATA:
            send_userdata();
            break;
        case CMD_SET_LENGTH:
            set_strip_length();
            break;
        case CMD_SET_LENGTH_ACK:
            send_ack(set_strip_length());
            break;
        case CMD_GET_LENGTH:
            send_strip_length();
            break;
        case CMD_GET_PKTCNT:
            send_packet_counters();
            break;
        case CMD_RESET_PKTCNT:
            lux_reset_counters();
            break;
        case CMD_RESET_PKTCNT_ACK:
            lux_reset_counters();
            send_ack(0);
            break;
    }
}
