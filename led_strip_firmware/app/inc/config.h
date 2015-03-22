#include "stm32f0xx.h"

#define UNICAST_ADDRESS_COUNT 16
#define USERDATA_SIZE 32

struct config {
    uint32_t multicast_address;
    uint32_t multicast_address_mask;
    uint32_t unicast_addresses[UNICAST_ADDRESS_COUNT];
    char userdata[USERDATA_SIZE];
};

extern struct config cfg;

void read_config_from_flash(void);
uint32_t write_config_to_flash(void);
