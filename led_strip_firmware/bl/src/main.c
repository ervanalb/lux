#include "hal.h"
#include "lux.h"
#include <string.h>

#include "stm32f0xx.h"

#define FLASH_START 0x08000000
#define FLASH_END   0x08008000

#define BUSYWAIT() for(volatile long i = 0; i < 1000000; i++)

const char id[]="LUX Bootloader";
#define ID_SIZE (sizeof(id)-1)

uint8_t match_destination(uint8_t* dest);
void rx_packet();

// First byte of packet is command

enum bootloader_command {
    CMD_RESET = 0x80,
    CMD_READID,
    CMD_INVALIDATEAPP,
    CMD_ERASE,
    CMD_WRITE,
    CMD_READ,
};

union lux_command_frame {
	struct __attribute__((__packed__)) cmd_addr_len {
        uint8_t cmd;
		uint32_t addr;
		uint16_t len;
        uint8_t *data;
	} addr_len;
	struct __attribute__((__packed__)) cmd_data {
        uint8_t cmd;
        uint8_t *data;
	} data;
};

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
    return *(uint32_t*)dest == 0xFFFFFFFF || *(uint32_t*)dest == 0x8000000;
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

static void erase_flash(uint32_t addr){
    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 

    if(FLASH_ErasePage(addr) != FLASH_COMPLETE)
        goto fail;

fail:
    FLASH_Lock();
    __enable_irq();

}

static void write_flash(uint32_t* data, uint32_t addr, uint16_t len){
    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 

    for(int i = 0; i < len; i += 4){
        if(FLASH_ProgramWord(addr, *data++) != FLASH_COMPLETE)
            goto fail;
        addr += 4;
    }

fail:
    FLASH_Lock();
    __enable_irq();
}

extern uint32_t _siisr;

static void invalidate_app(){
    erase_flash(FLASH_START);
    write_flash(&_siisr, FLASH_START, 1024);
}


void rx_packet()
{
    uint32_t addr;
    uint16_t len;
    union lux_command_frame *cf = (union lux_command_frame *)lux_packet;
    if(lux_packet_length == 0)
    {
        BUSYWAIT();
        send_id();
    }

    switch(cf->data.cmd)
    {
        case CMD_RESET:
            reset(); // Never returns
        case CMD_READID:
            BUSYWAIT();
            send_id();
            break;
        case CMD_INVALIDATEAPP:
            invalidate_app();
            lux_packet_in_memory = 0;
            break;
        case CMD_ERASE:
            if(lux_packet_length != sizeof(addr) + 1) goto erasefail;
            addr = cf->addr_len.addr;
            if((FLASH_START < addr) || (addr + 1024 > FLASH_END)) goto erasefail;
            lux_packet_in_memory = 0;
            erase_flash(addr);
            break;
        erasefail:
            lux_packet_in_memory = 0;
            led_on();
            break;
        case CMD_WRITE:
            if(lux_packet_length >= sizeof(addr) + sizeof(len) + 1){
                addr = cf->addr_len.addr;
                len = cf->addr_len.len;
                if(lux_packet_length != (sizeof(addr) + sizeof(len) + 1 + len)) goto writefail;
                if((FLASH_START < addr) || (addr > FLASH_END)) goto writefail;
                if((FLASH_START < (addr+len)) || ((addr+len) > FLASH_END)) goto writefail;
                BUSYWAIT();
                lux_stop_rx();
                clear_destination();
                lux_packet_length = len;
                write_flash((uint32_t *) cf->addr_len.data, addr, len);
                memcpy(lux_packet, (uint32_t *) addr, len);
                lux_packet_in_memory = 0;
                lux_start_tx();
                break;
            }
        writefail:
            lux_packet_in_memory = 0;
            led_on();
            break;
        case CMD_READ:
            if(lux_packet_length == sizeof(addr) + sizeof(len) + 1){
                addr = cf->addr_len.addr;
                len = cf->addr_len.len;
                if(len > LUX_PACKET_MEMORY_SIZE) goto readfail;
                if((FLASH_START > addr) || (addr > FLASH_END)) goto readfail;
                if((FLASH_START > (addr+len)) || ((addr+len) > FLASH_END)) goto readfail;
                BUSYWAIT();
                lux_stop_rx();
                clear_destination();
                lux_packet_length = len;
                memcpy(lux_packet, (uint32_t *) addr, len);
                lux_packet_in_memory = 0;
                lux_start_tx();
                break;
            }
        readfail:
            lux_packet_in_memory = 0;
            led_on();
            break;
        default:
            lux_packet_in_memory = 0;
    }
}
