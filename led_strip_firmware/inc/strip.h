#ifndef __STRIP_H
#define __STRIP_H

#include <stdint.h>

#define STRIP_LENGTH 50

void strip_init();
void strip_write(uint8_t* rgb_data);
uint8_t strip_ready();

#endif
