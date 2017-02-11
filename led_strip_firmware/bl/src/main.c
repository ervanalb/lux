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

//uint32_t erased_page = 0;
uint32_t flash_base_addr = 0;

void main()
{
    init();

    lux_init();

    led_off();

    for(;;)
    {
        lux_codec();
    }
}

uint8_t lux_fn_match_destination(uint8_t* dest)
{
    return *(uint32_t*)dest == 0xFFFFFFFF || *(uint32_t*)dest == 0x80000000;
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

static void baseaddr_flash_cmd()
{
    if(lux_packet.payload_length != 4) goto fail;

    memcpy(&flash_base_addr, &lux_packet.payload[0], sizeof(flash_base_addr));

    send_ack_crc();
    return;
fail:
    send_nak();
}

static void erase_flash_cmd()
{
    uint32_t addr;

    memcpy(&addr, &lux_packet.payload[0], sizeof(uint32_t));

    if(lux_packet.payload_length != 4) goto fail;
    //if((FLASH_START > addr) || (addr + 1024 > FLASH_END)) goto fail;
    if(!IS_FLASH_PROGRAM_ADDRESS(addr)) goto fail;

    if(erase_flash(addr) != 0) goto fail;
        
    send_ack_crc();
    return;
fail:
    send_nak();
}

static uint8_t write_flash(uint8_t* data, uint32_t addr, uint16_t len)
{
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

    if (rc != FLASH_COMPLETE) return 1;
    return 0;
}

static void write_flash_cmd()
{
    uint32_t addr = flash_base_addr + lux_packet.index * 1024;
    uint16_t len = lux_packet.payload_length;

    if(!(IS_FLASH_PROGRAM_ADDRESS(addr) && IS_FLASH_PROGRAM_ADDRESS(addr + len - 1))) goto fail;
    //if((FLASH_START > addr) || (addr > FLASH_END)) goto fail;
    //if((FLASH_START > (addr+len)) || ((addr+len) > FLASH_END)) goto fail;
    // I think this is redundant @zbanks
    //if((erased_page > (addr)) || ((addr+len) > (erased_page + 1024))) goto fail;

    if(write_flash(lux_packet.payload, addr, len)) goto fail;

    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = len;
    memcpy(lux_packet.payload, (void *) addr, len);

    lux_start_tx();
    return;
fail:
    send_nak();
}

static void read_flash_cmd()
{
    uint32_t addr = flash_base_addr + lux_packet.index * 1024;
    uint16_t len = 1024;

    if(lux_packet.payload_length != 0) goto fail;
    if(len > LUX_PACKET_MEMORY_SIZE) goto fail;
    if((FLASH_START > addr) || (addr > FLASH_END)) goto fail;
    if((FLASH_START > (addr+len)) || ((addr+len) > FLASH_END)) goto fail;

    lux_stop_rx();
    clear_destination();

    lux_packet.payload_length = len;
    memcpy(lux_packet.payload, (void *) addr, len);

    lux_start_tx();
    return;
fail:
    send_nak();
}

static void invalidate_app_cmd()
{
    if(erase_flash(FLASH_START)) goto fail;
    if(write_flash((uint8_t*)ISR_START, FLASH_START, 1024)) goto fail;

    send_ack_crc();
    return;
fail:
    send_nak();
}

void lux_fn_rx()
{
    lux_packet_in_memory = 0;

    switch(lux_packet.command)
    {
        case LUX_CMD_RESET:
            reset(); // Never returns
        case LUX_CMD_GET_ID:
            send_id();
            break;
        case LUX_CMD_INVALIDATEAPP:
            invalidate_app_cmd();
            break;
        case LUX_CMD_FLASH_BASEADDR:
            baseaddr_flash_cmd();
            break;
        case LUX_CMD_FLASH_ERASE:
            erase_flash_cmd();
            break;
        case LUX_CMD_FLASH_WRITE:
            write_flash_cmd();
            break;
        case LUX_CMD_FLASH_READ:
            read_flash_cmd();
            break;
    }
}
