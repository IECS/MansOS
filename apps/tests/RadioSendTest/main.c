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

//-----------------------------------------------------------------------------
//  Counter exchange application
//  Mote broadcasts its counter which gets incremented every second.
//  When receiving a counter value, mote displays it on leds.
//-----------------------------------------------------------------------------

#include "stdmansos.h"
#include <lib/random.h>
#include <string.h>

#define RECV 0

void sendCounter();
void recvCounter();

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
#if RECV
    radioSetReceiveHandle(recvCounter);
    radioOn();
#else
//  radioSetReceiveHandle(radioDiscard);
//  radioOn();
    sendCounter();
#endif
}

// receive counter and display it on leds
void recvCounter()
{
    static uint8_t buffer[128];
    int16_t len;
    uint16_t sender;
    uint8_t counter;

    redLedToggle();
    len = radioRecv(buffer, sizeof(buffer));
    if (len < 0) {
        PRINTF("radio recv failed\n");
        return;
    }
    if (len == 0) {
        PRINTF("0 len!\n");
    }
    if (len > 0) {
        memcpy(&sender, buffer + 1, sizeof(sender));
        counter = buffer[0];
        PRINTF("0x%04x: received counter %u\n", sender, counter);
        ledsSet(counter);
    }
}

void sendCounter() {
    static uint8_t sendBuffer[100];
    uint8_t *counter = &sendBuffer[0];
    memcpy(sendBuffer + 1, &localAddress, 2);
    while (1) {
        PRINTF("0x%04x: sending counter %i\n", localAddress, *counter);
        redLedToggle();
        radioSend(sendBuffer, sizeof(sendBuffer));
        msleep(3 * 100 + randomRand() % 1000);
        ++(*counter);
    }
}
