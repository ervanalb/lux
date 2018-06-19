#include "lux_device.h"
#include "stm32f0xx.h"

struct lux_device_config cfg_flash __attribute__((section(".config"),aligned(4))) = {
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
    FLASH_Status r = FLASH_COMPLETE;
    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 

    // TODO: Fix this to flash properly across pages, if cfg_flash is not aligned
    uint32_t a = (uint32_t) &cfg_flash;
    if ((r = FLASH_ErasePage(a) != FLASH_COMPLETE))
        goto fail;

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
    const uint8_t * d = (const uint8_t *) &lux_device_config;
    for (uint16_t i = 0; i < 1024; i += 2) {
        uint16_t w = 0;
        if (i + 4 < sizeof(cfg_flash)) {
            w |= *d++;
            w |= *d++ << 8;
        }
        if ((r = FLASH_ProgramHalfWord(a + i, w)) != FLASH_COMPLETE)
            goto fail;
    }

fail:
    FLASH_Lock();
    __enable_irq();

    return (r == FLASH_COMPLETE) ? 0 : 1;
}
