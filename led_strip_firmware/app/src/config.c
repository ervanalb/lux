#include "lux_device.h"
#include "stm32f0xx.h"

struct lux_device_config cfg_flash __attribute__((section(".config"))) = {
    .strip_length = 1,
    .addresses = {
        .multicast_mask = 0x00000000,
        .multicast      = 0xFFFFFFFF,
        .unicasts       = { LUX_ADDRESS_NEW },
    },
    .userdata = {}
};

void lux_device_read_config(){
    lux_device_config = cfg_flash;
}

uint8_t lux_device_write_config(){
    FLASH_Status r;
    uint32_t a;
    uint32_t *d;

    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 

    if((r = FLASH_ErasePage((uint32_t) (&cfg_flash))) != FLASH_COMPLETE)
        goto fail;

    a = (uint32_t) &cfg_flash;
    d = (uint32_t *) &lux_device_config;
    for(uint32_t i = 0; i < sizeof(cfg_flash); i += 4){
        if((r = FLASH_ProgramWord(a, *d++)) != FLASH_COMPLETE)
            goto fail;
        a += 4;
    }

fail:
    FLASH_Lock();
    __enable_irq();

    return (r == FLASH_COMPLETE) ? 0 : 1;
}
