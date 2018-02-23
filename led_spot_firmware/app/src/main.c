#include "hal.h"
#include "lux.h"
#include "lux_device.h"

#include <string.h>

const char lux_device_id[] = "LED Spot";
const uint16_t lux_device_id_length = sizeof(lux_device_id)-1;
const uint16_t lux_device_max_strip_length = 1;

static uint8_t led_color[3] = {0, 0, 0};

int __attribute__((noreturn)) main(void) {
    init();

    lux_device_init();

    for(;;) {
        lux_device_poll();
    }
}

static void set_frame() {
    if(lux_packet.payload_length < 3)
        return;
    if(lux_packet.index != 0)
        return;
    memcpy(led_color, lux_packet.payload, 3);
}

static uint8_t sync_frame() {
    spot_color(led_color[0], led_color[1], led_color[2]);
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
    return 0;
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
