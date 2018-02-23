#include "hal.h"
#include "lux.h"
#include "lux_device.h"
#include <string.h>

#include "stm32f0xx.h"

extern uint32_t _sapp;
extern uint32_t _eapp;
extern uint32_t _siisr;

// Define what regions can be modified by the bootloader
#define FLASH_START ((uint32_t) (&_sapp))
#define FLASH_END   ((uint32_t) (&_eapp))
#define ISR_START   ((uint32_t) (&_siisr))

const char lux_device_id[] = "Lux Bootloader";
const uint16_t lux_device_id_length = sizeof(lux_device_id)-1;
const uint16_t lux_device_max_strip_length = 0;

//uint32_t erased_page = 0;
uint32_t flash_base_addr = 0;

int __attribute__((noreturn)) main(void) {
    init();
    lux_device_init();
    led_off();

    for(;;) {
        lux_device_poll();
    }
}

void lux_device_read_config(void) {
    lux_device_config = (struct lux_device_config) {
        .strip_length = 0,
        .addresses = {
            .multicast_mask = 0x00000000,
            .multicast      = 0xFFFFFFFF,
            .unicasts       = { LUX_ADDRESS_BL },
        },
        .userdata = {},
    };
}

uint8_t lux_device_write_config(void) {
    return 0;
}

static uint8_t flash_wait()
{
    static volatile uint32_t a;
    a = 0;
    while(!FLASH->SR) a++;
    while(FLASH->SR & FLASH_SR_BSY) a++;
    if(FLASH->SR & FLASH_SR_EOP) return 0;
    if(FLASH->SR & FLASH_SR_WRPERR) return 1;
    if(FLASH->SR & FLASH_SR_PGERR) return 2;
    return 3;
}

static uint8_t erase_flash(uint32_t addr)
{
    __disable_irq();
    FLASH_Unlock();

    //static volatile uint16_t flash;
    //flash = FLASH->SR;

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    //FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_WRPRTERR);

    FLASH_Status rc = FLASH_ErasePage(addr);

    /*
    flash = FLASH->SR;

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR  = addr;
    FLASH->CR |= FLASH_CR_STRT;
    r = flash_wait();
    FLASH->CR &= FLASH_CR_PER;

    flash = FLASH->SR;
    */

    FLASH_Lock();
    __enable_irq();

    if (rc == FLASH_COMPLETE) return 0;
    return 1; 
}

static uint8_t baseaddr_flash_cmd() {
    if(lux_packet.payload_length != 4)
        return LUX_DEVICE_REPLY_NAK;

    memcpy(&flash_base_addr, &lux_packet.payload[0], sizeof(flash_base_addr));
    return LUX_DEVICE_REPLY_ACK;
}

static uint8_t erase_flash_cmd() {
    uint32_t addr;
    memcpy(&addr, &lux_packet.payload[0], sizeof(uint32_t));

    if(lux_packet.payload_length != 4)
        return LUX_DEVICE_REPLY_NAK;
    //if((FLASH_START > addr) || (addr + 1024 > FLASH_END)) goto fail;
    if(!IS_FLASH_PROGRAM_ADDRESS(addr))
        return LUX_DEVICE_REPLY_NAK;

    if(erase_flash(addr) != 0)
        return LUX_DEVICE_REPLY_NAK;
    return LUX_DEVICE_REPLY_ACK;
}

static uint8_t write_flash(uint8_t* data, uint32_t addr, uint16_t len) {
    uint32_t i = 0;
    //uint32_t word;
    uint16_t halfword;
    FLASH_Status rc = FLASH_COMPLETE;

    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    //FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_WRPRTERR);
    //FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
    while(i + 2 <= len)
    {
        /*
        memcpy(&word, &data[i], 4);
        rc = FLASH_ProgramWord(addr + i, word);
        flash_wait();
        i += 4;
        if(rc != FLASH_COMPLETE) break;
        */
        memcpy(&halfword, &data[i], 2);
        FLASH->CR |= FLASH_CR_PG;
        *(__IO uint16_t*)(addr + i) = halfword;
        uint8_t r = flash_wait();
        FLASH->CR &= ~FLASH_CR_PG;
        i += 2;
        if (r) break;
    }

    FLASH_Lock();
    __enable_irq();

    if (rc == FLASH_COMPLETE) return 0;
    return 1;
}

static uint8_t write_flash_cmd() {
    uint32_t addr = flash_base_addr + lux_packet.index * 1024;
    uint16_t len = lux_packet.payload_length;

    if(!(IS_FLASH_PROGRAM_ADDRESS(addr) && IS_FLASH_PROGRAM_ADDRESS(addr + len - 1)))
        return LUX_DEVICE_REPLY_NAK;
    //if((FLASH_START > addr) || (addr > FLASH_END)) goto fail;
    //if((FLASH_START > (addr+len)) || ((addr+len) > FLASH_END)) goto fail;
    // I think this is redundant @zbanks
    //if((erased_page > (addr)) || ((addr+len) > (erased_page + 1024))) goto fail;

    if(write_flash(lux_packet.payload, addr, len))
        return LUX_DEVICE_REPLY_NAK;

    lux_packet.payload_length = len;
    memcpy(lux_packet.payload, (void *) addr, len);
    return LUX_DEVICE_REPLY_PACKET;
}

static uint8_t read_flash_cmd() {
    uint32_t addr = flash_base_addr + lux_packet.index * 1024;
    uint16_t len = 1024;

    if(lux_packet.payload_length != 0)
        return LUX_DEVICE_REPLY_NAK;
    if(len > LUX_PACKET_MEMORY_SIZE)
        return LUX_DEVICE_REPLY_NAK;
    if((FLASH_START > addr) || (addr > FLASH_END))
        return LUX_DEVICE_REPLY_NAK;
    if((FLASH_START > (addr+len)) || ((addr+len) > FLASH_END))
        return LUX_DEVICE_REPLY_NAK;

    lux_packet.payload_length = len;
    memcpy(lux_packet.payload, (void *) addr, len);
    return LUX_DEVICE_REPLY_PACKET;
}

static uint8_t invalidate_app_cmd() {
    if(erase_flash(FLASH_START))
        return LUX_DEVICE_REPLY_NAK;
    if(write_flash((uint8_t*)ISR_START, FLASH_START, 1024))
        return LUX_DEVICE_REPLY_NAK;
    return LUX_DEVICE_REPLY_ACK;
}

uint8_t lux_device_handle_packet() {
    switch(lux_packet.command) {
    case LUX_CMD_INVALIDATEAPP:
        return invalidate_app_cmd();
    case LUX_CMD_FLASH_BASEADDR:
        return baseaddr_flash_cmd();
    case LUX_CMD_FLASH_ERASE:
        return erase_flash_cmd();
    case LUX_CMD_FLASH_WRITE:
        return write_flash_cmd();
    case LUX_CMD_FLASH_READ:
        return read_flash_cmd();
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
    reset();
}

void lux_device_reset(void) {
    reset();
}
