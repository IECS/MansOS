/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of  conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//-------------------------------------------
//      Blink regression test application.
//-------------------------------------------

#include "stdmansos.h"
#include "radio.h"
#include "extflash.h"
#include "ads1115/ads1115.h"
#include <lib/energy.h>

#define PAUSE 2000

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
#if 0 // no effect
    UCTL0 = SWRST;
    ME1 &= ~(URXE0 | UTXE0 | USPIE0);
    UCTL0 &= ~SWRST;

    UCTL1 = SWRST;
    ME2 &= ~(URXE1 | UTXE1 | USPIE1);
    UCTL1 &= ~SWRST;
#endif

#if 1
    ADC12IE = 0;
    ADC12IFG = 0;
    ADC12CTL0 &= ~ENC;
    ADC12CTL0 &= ~REFON;
    ADC12CTL0 &= ~ADC12ON;

    DMA0CTL = 0;
    DMA1CTL = 0;
#endif


    pinAsData(1, 0);
    pinAsData(1, 1);
    pinAsData(1, 2);
    pinAsData(1, 3);
    pinAsData(1, 4);
    pinAsData(1, 5);
    pinAsData(1, 6);
    pinAsData(1, 7); // radio data indicate

    pinAsData(2, 0); // ADS interrupts (unused)
    pinAsData(2, 1);
    pinAsData(2, 2);
    pinAsData(2, 3); // SHT SDA + I2C soft SDA
    pinAsData(2, 4); // SHT SCL + I2C soft SCL
    pinAsData(2, 5); // sensors enable
    pinAsData(2, 6);
    pinAsData(2, 7); // uart0 rx

    pinAsData(3, 0); // SD card CS
    pinAsData(3, 1);
    pinAsData(3, 2);
    pinAsData(3, 3);
    pinAsData(3, 4); // uart0 hw tx
    pinAsData(3, 5); // uart0 hw rx
    pinAsData(3, 6); // uart1 hw tx
    pinAsData(3, 7); // uart1 hw tx

    pinAsData(4, 0); // radio data request
    pinAsData(4, 1); // radio rts
    pinAsData(4, 2); // radio config
    pinAsData(4, 3); // radio trx disable
    pinAsData(4, 4); // radio reset
    pinAsData(4, 5); // radio sleep
    // pinAsData(4, 6); // uart0 tx
    // pinAsData(4, 7);

    pinAsData(5, 0);
    pinAsData(5, 1);
    pinAsData(5, 2);
    pinAsData(5, 3);
    pinAsData(5, 4); // red LED
    pinAsData(5, 5); // green LED
    pinAsData(5, 6); // blue LED
    pinAsData(5, 7);

    pinAsData(6, 0);
    pinAsData(6, 1);
    pinAsData(6, 2);
    pinAsData(6, 3);
    pinAsData(6, 4);
    pinAsData(6, 5);
    pinAsData(6, 6);
    pinAsData(6, 7);



    pinAsInput(1, 0);
    pinAsInput(1, 1);
    pinAsInput(1, 2);
    pinAsInput(1, 3);
    pinAsInput(1, 4);
    pinAsInput(1, 5);
    pinAsInput(1, 6);
    pinAsInput(1, 7); // radio data indicate

    pinAsInput(2, 0); // ADS interrupts (unused)
    pinAsInput(2, 1);
    pinAsInput(2, 2);
    pinAsInput(2, 3); // SHT SDA + I2C soft SDA
//    pinAsInput(2, 4); // SHT SCL + I2C soft SCL
//    pinAsInput(2, 5); // sensors enable
    pinAsInput(2, 6);
    pinAsInput(2, 7); // uart0 rx

    pinAsInput(3, 0); // SD card CS
    pinAsInput(3, 1);
    pinAsInput(3, 2);
    pinAsInput(3, 3);
    pinAsInput(3, 4); // uart0 hw tx
    pinAsInput(3, 5); // uart0 hw rx
    pinAsInput(3, 6); // uart1 hw tx
    pinAsInput(3, 7); // uart1 hw tx

    pinAsInput(4, 0); // radio data request
    pinAsInput(4, 1); // radio rts
    pinAsInput(4, 2); // radio config
    // pinAsInput(4, 3); // radio trx disable
    pinAsInput(4, 4); // radio reset
    // pinAsInput(4, 5); // radio sleep
    pinAsInput(4, 6); // uart0 tx
    pinAsInput(4, 7);

    pinAsInput(5, 0);
    pinAsInput(5, 1);
    pinAsInput(5, 2);
    pinAsInput(5, 3);
//    pinAsInput(5, 4); // red LED
    pinAsInput(5, 5); // green LED
    pinAsInput(5, 6); // blue LED
    pinAsInput(5, 7);

    pinAsInput(6, 0);
    pinAsInput(6, 1);
    pinAsInput(6, 2);
    pinAsInput(6, 3);
    pinAsInput(6, 4);
    pinAsInput(6, 5);
    pinAsInput(6, 6);
    pinAsInput(6, 7);


    // --  required for better efficiency 
    // make sure I2C clock is high
    I2C_SCL_HI();
    // make sure sensors are disabled
    pinClear(SM3_SENSORS_ENABLE_PORT, SM3_SENSORS_ENABLE_PIN);
    // --


    while (1) {
        msleep(PAUSE); // sleep PAUSE seconds
        // PRINTF("%lu: hello world\n", (uint32_t) getJiffies());
        ledToggle();
    }
}
