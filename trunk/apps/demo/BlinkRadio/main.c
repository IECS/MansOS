/**
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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

// --------------------------------------------------------------
// This application displays RSSI (radio signal strength) on LEDs.
// Green LED is blinked in frequency depending on received signal strength.
// Red LED is turned on if there is no signal at all.
//
// This is calibrated for CC2420 on Tmote Sky.
// For use on other platforms it must be recalibrated.
// --------------------------------------------------------------

#include "mansos.h"
#include "leds.h"
#include "udelay.h"
#include "dprint.h"
#include "radio.h"
#include "timers.h"

// calibrate this - the RSSI range we are interested in
#ifndef RSSI_UPPER_BOUND
#define RSSI_UPPER_BOUND   30
#endif
#ifndef RSSI_LOWER_BOUND
#define RSSI_LOWER_BOUND (-70)
#endif
#define RSSI_RANGE (RSSI_UPPER_BOUND - RSSI_LOWER_BOUND)

#ifndef TX_POWER
#define TX_POWER 31  // max on Tmote
//#define TX_POWER 1  // min on Tmote
#endif

// variable that determines whether last receving was ok
uint8_t rxOk;

void rcvRadio(void)
{
    static uint8_t buffer[RADIO_MAX_PACKET];
    uint16_t len;
    len = radioRecv(buffer, sizeof(buffer));
    ++rxOk;
    if (rxOk > 3) rxOk = 3;
}

uint32_t getWaitInterval(int8_t rssi) {
    if (!rxOk) return 600000ul;
    int16_t normRssi = (int16_t) rssi - RSSI_LOWER_BOUND;
    if (normRssi > RSSI_RANGE) normRssi = RSSI_RANGE;
    if (normRssi < 0) normRssi = 0;
    return 1000 + 400000ul - normRssi * 400000ul / RSSI_RANGE;
}

void appMain(void)
{
    static uint8_t buffer[100];

    radioSetTxPower(TX_POWER);
    radioSetReceiveHandle(rcvRadio);
    radioOn();

    uint32_t lastTime = getRealTime();

    for (;;) {
        int8_t rssi = radioGetLastRSSI();

        if ((int32_t) getRealTime() - (int32_t) lastTime > 1000) {
            radioSend(buffer, sizeof(buffer));

            PRINTF("rssi=%d\n", rssi);
            lastTime = getRealTime();
            if (rxOk) --rxOk;

            if (!rxOk) redLedOn();
            else redLedOff();
        }

        greenLedToggle();
        busyWait(getWaitInterval(rssi));
    }
}
