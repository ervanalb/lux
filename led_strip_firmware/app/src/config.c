#include "config.h"

#include "stm32f0xx.h"

struct config cfg;

struct config cfg_flash __attribute__((section(".config"))) = {
    .strip_length = MAX_STRIP_LENGTH,
    .multicast_address_mask = 0x00000000,
    .multicast_address      = 0xFFFFFFFF,
    .unicast_addresses = {
        0x80000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,    
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,    
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,    
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
    },
    .userdata = {}
};

void read_config_from_flash(){
    cfg = cfg_flash;
}

FLASH_Status write_config_to_flash(){
    FLASH_Status r;
    uint32_t a;
    uint32_t *d;

    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 

    if((r = FLASH_ErasePage((uint32_t) (&cfg_flash))) != FLASH_COMPLETE)
        goto fail;

    a = (uint32_t) &cfg_flash;
    d = (uint32_t *) &cfg;
    for(int i = 0; i < sizeof(cfg_flash); i += 4){
        if((r = FLASH_ProgramWord(a, *d++)) != FLASH_COMPLETE)
            goto fail;
        a += 4;
    }

fail:
    FLASH_Lock();
    __enable_irq();

    return r;
}
