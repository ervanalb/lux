#include "config.h"

#include "stm32f0xx.h"

struct config cfg;

struct config cfg_flash __attribute__((section(".config"))) = {
    .multicast_address_mask = 0x0,
    .multicast_address = 0x0,
    .unicast_addresses = {
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,    
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,    
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,    
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
    },
    .userdata = {}
};

void read_config_from_flash(){
    cfg = cfg_flash;
}

uint32_t write_config_to_flash(){
    uint32_t r;

    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 

    if((r = FLASH_ErasePage(&cfg_flash)) != FLASH_COMPLETE)
        goto fail;

    for(int i; i < sizeof(cfg_flash); i += 4){
        if((r = FLASH_ProgramWord(((uint32_t) (&cfg_flash)) + i, *(uint32_t *) (((uint32_t) (&cfg)) + i))) != FLASH_COMPLETE)
            goto fail;
    }

fail:
    FLASH_Lock();
    __enable_irq();

    return r;
}
