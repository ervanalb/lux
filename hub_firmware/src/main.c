#include "hal.h"
#include "usbd_cdc_core.h"
#include  "usbd_usr.h"

USB_CORE_HANDLE  USB_Device_dev ;

int main()
{
    init();
    set_enabled_channels(0x3F);
    USBD_Init(&USB_Device_dev,
              &USR_desc, 
              &USBD_CDC_cb, 
              &USR_cb);

    for(;;);
}
