#include "hal.h"
#include "lux.h"
#include <string.h>

#include "stm32f0xx.h"

extern uint32_t _sapp;
extern uint32_t _eapp;
extern uint32_t _siisr;

// Define what regions can be modified by the bootloader
#define FLASH_START ((uint32_t) (&_sapp))
#define FLASH_END   ((uint32_t) (&_eapp))
#define ISR_START   ((uint32_t) (&_siisr))

#ifdef WS2811
const char id[]="WS2811 LED Strip Bootloader";
#elif LPD6803
const char id[]="LPD6803 LED Strip Bootloader";
#else
const char id[]="Lux Bootloader";
#endif
#define ID_SIZE (sizeof(id)-1)

uint8_t match_destination(uint8_t* dest);
void rx_packet();

uint32_t erased_page = 0;

void main()
{
    init();

    lux_init();

    lux_fn_match_destination = &match_destination;
    lux_fn_rx = &rx_packet;

    led_off();

    for(;;)
    {
        lux_codec();
    }
}

uint8_t match_destination(uint8_t* dest)
{
    return *(uint32_t*)dest == 0xFFFFFFFF || *(uint32_t*)dest == 0x80000000;
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

static uint8_t flash_wait()
{
    while(FLASH->SR & FLASH_SR_BSY);
    if(FLASH->SR & FLASH_SR_EOP) return 0;
    if(FLASH->SR & FLASH_SR_WRPERR) return 1;
    if(FLASH->SR & FLASH_SR_PGERR) return 2;
    return 3;
}

static uint8_t erase_flash(uint32_t addr)
{
    FLASH_Status r;
    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR  = addr;
    FLASH->CR |= FLASH_CR_STRT;
 
    r = flash_wait();

    FLASH_Lock();
    __enable_irq();

    if(!r) erased_page = addr;
    return r; 
}

static void erase_flash_cmd()
{
    uint32_t addr;

    memcpy(&addr, &lux_packet[1], sizeof(uint32_t));

    if(lux_packet_length != 5) return;
    if((FLASH_START > addr) || (addr + 1024 > FLASH_END)) return;

    if(!erase_flash(addr)) send_ack();
}

static uint8_t write_flash(uint8_t* data, uint32_t addr, uint16_t len)
{
    uint32_t i = 0;
    uint8_t r = 0;
    uint16_t halfword;

    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
    while(i < len)
    {
        memcpy(&halfword, &data[i], sizeof(uint16_t));
        FLASH->CR |= FLASH_CR_PG;
        *(__IO uint16_t*)(addr + i) = halfword;
        r = flash_wait();
        FLASH->CR &= ~FLASH_CR_PG;
        i += 2;
        if(r) break;
    }

    FLASH_Lock();
    __enable_irq();

    return r;
}

static void write_flash_cmd()
{
    uint32_t addr;
    uint16_t len = lux_packet_length - 5;

    memcpy(&addr, &lux_packet[1], sizeof(uint32_t));

    if(addr & 1 || len & 1) return;
    if((FLASH_START > addr) || (addr > FLASH_END)) return;
    if((FLASH_START > (addr+len)) || ((addr+len) > FLASH_END)) return;
    if((erased_page > (addr)) || ((addr+len) > (erased_page + 1024))) return;

    if(write_flash(&lux_packet[5], addr, len)) return;

    lux_stop_rx();
    clear_destination();

    lux_packet_length = len;
    memcpy(lux_packet, (uint16_t*)addr, len);

    lux_start_tx();
}

static void read_flash_cmd()
{
    uint32_t addr;
    uint16_t len;

    memcpy(&addr, &lux_packet[1], sizeof(uint32_t));
    memcpy(&len, &lux_packet[5], sizeof(uint16_t));

    if(lux_packet_length != 7) return;
    if(addr & 1 || len & 1) return;
    if(len > LUX_PACKET_MEMORY_SIZE) return;
    if((FLASH_START > addr) || (addr > FLASH_END)) return;
    if((FLASH_START > (addr+len)) || ((addr+len) > FLASH_END)) return;

    lux_stop_rx();
    clear_destination();

    lux_packet_length = len;
    memcpy(lux_packet, (uint16_t *) addr, len);

    lux_start_tx();
}

static void invalidate_app()
{
    if(erase_flash(FLASH_START)) return;
    if(write_flash((uint8_t*)ISR_START, FLASH_START, 1024)) return;
    send_ack();
}

void rx_packet()
{
    lux_packet_in_memory = 0;

    if(lux_packet_length == 0)
    {
        send_id();
        return;
    }

    switch(lux_packet[0])
    {
        case CMD_RESET:
            reset(); // Never returns
        case CMD_GET_ID:
            send_id();
            break;
        case CMD_INVALIDATEAPP:
            invalidate_app();
            break;
        case CMD_FLASH_ERASE:
            erase_flash_cmd();
            break;
        case CMD_FLASH_WRITE:
            write_flash_cmd();
            break;
        case CMD_FLASH_READ:
            read_flash_cmd();
            break;
    }
}
