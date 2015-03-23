#ifndef __STRIP_H
#define __STRIP_H

#include <stdint.h>
#include <config.h>

void strip_init();
void strip_write(uint8_t* rgb_data);
uint8_t strip_ready();

#endif
