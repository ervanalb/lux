#include "hal.h"
#include "lux_hal.h"
#include "lux.h"

#include "stm32f0xx.h"

void main()
{
    volatile int i=0;

    init();
    lux_init();

/*
    lux_stop_rx();

    lux_destination[0]=0x00;
    lux_destination[1]=0x00;
    lux_destination[2]=0x00;
    lux_destination[3]=0x00;

    lux_packet[0]=0x01;
    lux_packet[1]=0x02;
    lux_packet[2]=0x03;
    lux_packet[3]=0x04;

    lux_packet_length=4;

    lux_start_tx();
*/

    for(;;)
    {
        lux_codec();
        if(lux_packet_in_memory)
        {
            i++;
        }
    }
}

uint8_t lux_fn_match_destination(uint8_t* dest)
{
    return 1;
}

void lux_fn_rx()
{
    return;
}
