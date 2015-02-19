#include "hal.h"

void main()
{
    volatile int i;
    init();
    for(;;)
    {
        if(button())
            led_on();
        else
            led_off();
        getchar();
    }
}
