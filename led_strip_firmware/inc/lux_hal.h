#ifndef __LUX_HAL_H
#define __LUX_HAL_H

#include <stdint.h>

#define LUX_SERIAL_BUFFER_SIZE 512

extern uint8_t lux_serial_buffer[LUX_SERIAL_BUFFER_SIZE];

void lux_hal_enable_rx_dma();
void lux_hal_disable_rx_dma();
uint8_t lux_hal_serial_ready();
int16_t lux_hal_bytes_to_read();
uint8_t lux_hal_read_byte();
int16_t lux_hal_bytes_to_write();
void lux_hal_write_byte(uint8_t byte);

void lux_hal_crc(uint8_t byte);
uint8_t lux_hal_crc_ok();
void lux_hal_reset_crc();
void lux_hal_write_crc(uint8_t* ptr);

#endif
