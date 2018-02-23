#include <stdint.h>

void init(void);
void led_on(void);
void led_off(void);
uint8_t button(void);
void __attribute__((noreturn)) bootloader(void);
void __attribute__((noreturn)) reset(void);
