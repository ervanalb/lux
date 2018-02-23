#ifndef __STRIP_H
#define __STRIP_H

#include <stdint.h>
#include <config.h>

#define MAX_STRIP_LENGTH 340

void strip_init();
void strip_write(uint8_t* rgb_data, uint16_t begin_pos, uint16_t end_pos);
void strip_flush();
uint8_t strip_ready();

#endif
