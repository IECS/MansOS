/*
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
#include <random.h>
#include <string.h>

#define RECV 1
#define HW_ADDRESSING 0

#if !USE_PRINT
#define PRINTF(...)
#endif

//-------------------------------------------

void sendCounter(void);
void recvCounter(void);

void setLocalAddress(void) {
#if RADIO_CHIP==RADIO_CHIP_AMB8420
#if HW_ADDRESSING
    uint8_t localHwAddress = RECV ? 0x1 : 0x2;
    amb8420EnterAddressingMode(AMB8420_ADDR_MODE_ADDR, localHwAddress);
#else
    amb8420EnterAddressingMode(AMB8420_ADDR_MODE_NONE, 0x0);
#endif // HW_ADDRESSING
#endif // RADIO_CHIP_AMB8420
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // ------------------------- serial number
    PRINTF("%s %#04x starting...\n",
            RECV ? "Receiver" : "Sender", localAddress);

//    setLocalAddress();

#if RECV
    radioSetReceiveHandle(recvCounter);
    radioOn();

    for (;;) {
        mdelay(1000);
        blueLedOn();
        mdelay(100);
        blueLedOff();
        // radioSend("hello world\n", sizeof("hello world\n"));
        // int rssi = amb8420GetRSSI();
        // int8_t rssi = radioGetRSSI();
        // PRINTF("+++ rssi = %d\n", rssi);
        PRINTF(".\n");
    }
#else
    // radioSetReceiveHandle(radioDiscard);
    radioOn();
    sendCounter();
#endif
}

// receive counter and display it on leds
void recvCounter(void)
{
    static uint8_t buffer[128];
    int16_t len;
    uint16_t sender;
    uint8_t counter;
    int8_t rssi = 0;

    greenLedToggle();
    len = radioRecv(buffer, sizeof(buffer));
    rssi = radioGetLastRSSI();
    if (len < 0) {
        PRINTF("radio recv failed\n");
        //blueLedToggle();
        return;
    }
    if (len == 0) {
        PRINTF("0 len!\n");
    }
    // debugHexdump(buffer, len);
    if (len > 0) {
        // greenLedToggle();
        memcpy(&sender, buffer + 1, sizeof(sender));
        counter = buffer[0];
        PRINTF("received counter %u (%d bytes), rssi=%d\n", counter, len, rssi);
        // ledsSet(counter);
    }
}

void sendCounter(void) {
    // static uint8_t sendBuffer[120];
    static uint8_t sendBuffer[20] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    };

    uint8_t *counter = &sendBuffer[0];
    // memcpy(sendBuffer + 1, &localAddress, 2);
    while (1) {
        PRINTF("0x%04x: sending counter %i\n", localAddress, *counter);
        ledOn();
#if HW_ADDRESSING && RADIO_CHIP==RADIO_CHIP_AMB8420
        amb8420SetDstAddress(0x1);
#endif
        int8_t result = radioSend(sendBuffer, sizeof(sendBuffer));
        if (result != 0) {
            PRINTF("radio send failed\n"); 
        }
        mdelay(100);
        ledOff();
        mdelay(1000);
        ++(*counter);
    }
}
