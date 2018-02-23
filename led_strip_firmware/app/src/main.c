#include "hal.h"
#include "lux.h"
#include "lux_device.h"
#include "strip.h"
#include "config.h"
#include <string.h>

const char lux_device_id[] =
#ifdef WS2811
    "WS2811 LED Strip";
#elif LPD6803
    "LPD6803 LED Strip";
#else
    "Lux LED Strip";
#endif

const uint16_t lux_device_id_length = sizeof(lux_device_id)-1;
const uint16_t lux_device_max_strip_length = MAX_STRIP_LENGTH;

int __attribute__((noreturn)) main(void) {
    init();

    strip_init();
    lux_device_init();

    for(;;) {
        lux_device_poll();
    }
}


static void set_frame(void) {
    uint16_t begin = lux_packet.index * LUX_PACKET_MAX_SIZE;
    uint16_t end = begin + lux_packet.payload_length;
    strip_write(lux_packet.payload, begin, end);
}

static uint8_t sync_frame(void) {
    if(!strip_ready())
        return LUX_DEVICE_REPLY_NAK;
    strip_flush();
    return LUX_DEVICE_REPLY_ACK;
}

uint8_t lux_device_handle_packet(void) {
    switch(lux_packet.command) {
    case LUX_CMD_FRAME:
        set_frame();
        sync_frame();
        return LUX_DEVICE_REPLY_NONE;
    case LUX_CMD_FRAME_HOLD:
        set_frame();
        return LUX_DEVICE_REPLY_NONE;
    case LUX_CMD_FRAME_FLIP:
        sync_frame();
        return LUX_DEVICE_REPLY_NONE;
    case LUX_CMD_FRAME_ACK:
        set_frame();
        return sync_frame();
    case LUX_CMD_FRAME_HOLD_ACK:
        set_frame();
        return LUX_DEVICE_REPLY_ACK;
    case LUX_CMD_FRAME_FLIP_ACK:
        return sync_frame();
    }
    return LUX_DEVICE_REPLY_NAK;
}

uint8_t lux_device_get_button(void) {
    return button();
}

void lux_device_set_led(uint8_t state) {
    if (state)
        led_on();
    else
        led_off();
}

void lux_device_bootloader(void) {
    bootloader();
}

void lux_device_reset(void) {
    reset();
}
