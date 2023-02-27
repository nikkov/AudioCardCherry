/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 USART Print debugging routine:
 USART1_Tx(PA9).
 This example demonstrates using USART1(PA9) as a print debug port output.

*/

#include "debug.h"
#include "usbd_core.h"

/* Global typedef */

/* Global define */

/* Global Variable */

extern void board_init(void);
//extern void msc_ram_init(void);
extern void audio_init();
extern void audio_test();
extern bool usb_device_is_configured();

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    board_init();
    //msc_ram_init();
    audio_init();

    // Wait until configured
    while (!usb_device_is_configured()) {
    }
	printf("SystemClk:%ld\r\n",SystemCoreClock);
	printf("ChipID:%08lx\r\n", DBGMCU_GetCHIPID() );
	printf("This is printf example\r\n");
	audio_test();
	while(1) {}
}

