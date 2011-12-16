/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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

#include "stdmansos.h"
#include <string.h>

void sendCounter(void);
void recvCounter(void);
void ccaTest(void);

// #define this to true for CCA tests
// #define BLAST_PACKETS 1

Alarm_t timer;

void onTimer(void *param)
{
    // static bool off;
    // off = !off;
    // if (off) {
    //     PRINTF("turning radio off\n");
    //     radioOff();
    // } else {
    //     PRINTF("turning radio on\n");
    //     radioOn();
    // }

    PRINT("reset the radio\n");
    radioSetChannel(RADIO_CHANNEL);
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // ------------- setup radio
    // -- parameters
    // radioSetTxPower(30);
    // radioSetChannel(20);
    // -- set packet rx callback
    radioSetReceiveHandle(recvCounter);
    // -- turn listening on
    radioOn();
    // -------------

    // alarmInitAndRegister(&timer, onTimer, 5000, true, NULL);

    // ccaTest();
    // sendCounter();
    for (;;) {
        mdelay(500);
        redLedToggle();
        // radioOn();

        // PRINTF("rssi=%d\n", radioGetRSSI());
    }
}

void recvCounter(void)
{
    static uint8_t buffer[RADIO_MAX_PACKET];
    int16_t len;
    uint8_t *counter = &buffer[0];

    // greenLedToggle();
    len = radioRecv(buffer, sizeof(buffer));
    if (len < 0) {
        PRINTF("radio receive failed\n");
        return;
    }
    if (len == 0) {
        PRINTF("received zero length!\n");
    } else {
        PRINTF("received counter %u, len %u, rssi=%d, lqi=%u\n",
                *counter, len, radioGetLastRSSI(), radioGetLastLQI());
        debugHexdump(buffer, 12);
    }
}

void sendCounter(void)
{
    static uint8_t sendBuffer[RADIO_MAX_PACKET];
    uint8_t *counter = &sendBuffer[0];
    for (;;) {
        PRINTF("sending counter %i\n", *counter);
        redLedToggle();
        radioSend(sendBuffer, sizeof(sendBuffer));
#if !BLAST_PACKETS
        mdelay(1000);
#endif
        ++(*counter);

        // int rssi = radioGetRSSI();
        // bool ccaOk = radioIsChannelClear();
        // PRINTF("rssi=%d, ccaOk=%s\n", rssi, (ccaOk ? "true" : "false"));
    }
}

void ccaTest(void)
{
    for (;;) {
        volatile uint8_t byte = 0;
        uint16_t i;
        for (i = 1; i < 0x100; i <<= 1) {
            if (radioIsChannelClear()) byte |= i;
            udelay(40);
        }
        PRINTF("byte=0x%02x\n", byte);
    }
}
