#ifndef __LUX_HAL_H
#define __LUX_HAL_H

#include <stdint.h>

void lux_hal_enable_rx_dma();
void lux_hal_disable_rx_dma();
int16_t lux_hal_bytes_to_read();
uint8_t lux_hal_read_byte();

#endif
