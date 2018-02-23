#include "lux_device.h"
#include <string.h>

#ifdef LUX_DEVICE_ALWAYS_WRITE_CONFIG
#   define SOFT_WRITE_CONFIG (lux_device_write_config())
#else
#   define SOFT_WRITE_CONFIG
#endif

struct lux_device_config lux_device_config;
static uint32_t button_count = 0;

void lux_device_init() {
    lux_init();
    lux_device_read_config();
}

void lux_device_poll() {
    // Increment `button_count` on rising edge of `lux_device_get_button`
    static uint8_t button_state = 0;
    uint8_t button_state_new = lux_device_get_button();
    if (button_state < button_state_new)
        button_count++;
    button_state = button_state_new;

    lux_codec();
}

uint8_t lux_fn_match_destination(uint32_t addr) {
    // Never respond on 0x00000000
    if(addr == LUX_ADDRESS_NONE) return 0; 
    // Always respond on 0xFFFFFFFF
    if(addr == LUX_ADDRESS_ALL) return 1; 
    // Check multicast mask/address
    if((addr & lux_device_config.addresses.multicast_mask) == lux_device_config.addresses.multicast)
        return 1;
    // Check unicast addresses
    for(int i = 0; i < LUX_UNICAST_ADDRESS_COUNT; i++){
        if(addr == lux_device_config.addresses.unicasts[i])
            return 1;
    }
    // Special address if button is pressed
    if(addr == LUX_ADDRESS_BUTTON && lux_device_get_button())
        return 1;
    return 0;
}

//

static uint8_t send_id() {
    lux_packet.payload_length = lux_device_id_length;
    memcpy(lux_packet.payload, lux_device_id, lux_device_id_length);
    return LUX_DEVICE_REPLY_PACKET;
}

static uint8_t send_button() {
    lux_packet.payload_length = sizeof(button_count);
    memcpy(lux_packet.payload, &button_count, sizeof(button_count));
    button_count = 0;
    return LUX_DEVICE_REPLY_PACKET;
}

static uint8_t set_led() {
    if(lux_packet.payload_length != 1)
        return LUX_DEVICE_REPLY_NAK;
    lux_device_set_led(lux_packet.payload[0]);
    return LUX_DEVICE_REPLY_ACK;
}

static uint8_t send_addresses() {
    lux_packet.payload_length = sizeof lux_device_config.addresses;
    memcpy(lux_packet.payload, &lux_device_config.addresses, sizeof lux_device_config.addresses);
    return LUX_DEVICE_REPLY_PACKET;
}

static uint8_t set_addresses() {
    if(lux_packet.payload_length != sizeof lux_device_config.addresses)
        return LUX_DEVICE_REPLY_NAK;

    memcpy(&lux_device_config.addresses, lux_packet.payload, sizeof lux_device_config.addresses);
    SOFT_WRITE_CONFIG;

    return LUX_DEVICE_REPLY_ACK;
}

static uint8_t send_userdata() {
    uint16_t start = LUX_PACKET_MAX_SIZE * lux_packet.index;
    uint16_t length = LUX_PACKET_MAX_SIZE;
    if (start + length > USERDATA_SIZE)
        length = USERDATA_SIZE - start;

    lux_packet.payload_length = length;
    memcpy(&lux_packet.payload, &lux_device_config.userdata[start], length);
    return LUX_DEVICE_REPLY_PACKET;
}

static uint8_t set_userdata() {
    uint16_t start = LUX_PACKET_MAX_SIZE * lux_packet.index;
    uint16_t length = lux_packet.payload_length;

    if(start + length > USERDATA_SIZE)
        return LUX_DEVICE_REPLY_NAK;

    memcpy(&lux_device_config.userdata[start], lux_packet.payload, length);
    SOFT_WRITE_CONFIG;

    return LUX_DEVICE_REPLY_ACK;
}

static uint8_t set_strip_length() {
    if(lux_packet.payload_length != sizeof(lux_device_config.strip_length))
        return LUX_DEVICE_REPLY_NAK;

    uint16_t l = 0;
    memcpy(&l, lux_packet.payload, sizeof(l));
    if(l > lux_device_max_strip_length)
        return LUX_DEVICE_REPLY_NAK;

    lux_device_config.strip_length = l;
    SOFT_WRITE_CONFIG;

    return LUX_DEVICE_REPLY_ACK;
}

static uint8_t send_strip_length() {
    lux_packet.payload_length = sizeof lux_device_config.strip_length;
    memcpy(lux_packet.payload, &lux_device_config.strip_length, sizeof lux_device_config.strip_length);
    return LUX_DEVICE_REPLY_PACKET;
}

static uint8_t send_packet_counters() {
    lux_packet.payload_length = sizeof(lux_counters);
    memcpy(lux_packet.payload, &lux_counters, sizeof(lux_counters));
    return LUX_DEVICE_REPLY_PACKET;
}

static uint8_t reset_packet_counters() {
    lux_reset_counters();
    return LUX_DEVICE_REPLY_ACK;
}

void lux_fn_rx(void) {
    // Currently, the lux packet gets entirely processed in this function
    // so we can release lux_packet immediately
    lux_packet_in_memory = 0;

    uint8_t reply = LUX_DEVICE_REPLY_NAK;
    switch(lux_packet.command) {
    case LUX_CMD_GET_ID:
    case LUX_CMD_GET_DESCRIPTOR:
        reply = send_id();
        break;
    case LUX_CMD_RESET:
        if (lux_packet.payload[0] & 0x1)
            lux_device_bootloader(); // Never returns
        else
            lux_device_reset(); // Never returns
        break;
    case LUX_CMD_COMMIT_CONFIG:
        reply = LUX_DEVICE_REPLY_ACK;
        if (lux_device_write_config() != 0)
            reply = LUX_DEVICE_REPLY_NAK;
        break;
    case LUX_CMD_SET_LED:
        reply = set_led();
        break;
    case LUX_CMD_GET_BUTTON_COUNT:
        reply = send_button();
        break;
    case LUX_CMD_GET_ADDR:
        reply = send_addresses();
        break;
    case LUX_CMD_SET_ADDR:
        reply = set_addresses();
        break;
    case LUX_CMD_GET_USERDATA:
        reply = send_userdata();
        break;
    case LUX_CMD_SET_USERDATA:
        reply = set_userdata();
        break;
    case LUX_CMD_SET_LENGTH:
        reply = set_strip_length();
        break;
    case LUX_CMD_GET_LENGTH:
        reply = send_strip_length();
        break;
    case LUX_CMD_GET_PKTCNT:
        reply = send_packet_counters();
        break;
    case LUX_CMD_RESET_PKTCNT:
        reply = reset_packet_counters();
        break;
    default:
        reply = lux_device_handle_packet();
        break;
    }

    if (reply == LUX_DEVICE_REPLY_NONE)
        return;

    lux_stop_rx();
    lux_packet.destination = LUX_ADDRESS_REPLY;

    if (reply == LUX_DEVICE_REPLY_ACK) {
        lux_packet.payload_length = 5;
        lux_packet.payload[0] = 0;
        memcpy(&lux_packet.payload[1], lux_packet.crc, sizeof(lux_packet.crc));
    } else if (reply == LUX_DEVICE_REPLY_NAK) {
        lux_packet.payload_length = 5;
        lux_packet.payload[0] = 255;
        memcpy(&lux_packet.payload[1], lux_packet.crc, sizeof(lux_packet.crc));
    }

    lux_start_tx();
}
