#include "debug.h"
#include "usbd_core.h"


extern void board_init(void);
extern void audio_init();
extern void audio_test();
extern bool usb_device_is_configured();

int main(void)
{
    board_init();
    audio_init();
    usbd_initialize();

	printf("SystemClk:%ld\r\n",SystemCoreClock);
	printf("ChipID:%08lx\r\n", DBGMCU_GetCHIPID());
	printf("Audio class example\r\n");

    // Wait until configured
    while (!usb_device_is_configured()) {}

    audio_test();
}

