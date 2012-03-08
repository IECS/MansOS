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

#include "stdmansos.h"
#include <hil/errors.h>
#include <lib/random.h>

#include "lpl.h"

#define RECV 1

#define DUTY_CYCLE_LENGTH  100 // ms
#define PACKET_WAIT_TIME   100 // ms

void sendPackets(void);
void recvTest(void);
void ccaTest(void);

void appMain(void)
{
    // uint16_t t1 = TAR;
    // mdelay(100);
    // uint16_t t2 = TAR;
    // PRINTF("t1=%u t2=%u\n", t1, t2);

#if RECV
    // recvTest();
    ccaTest();
#else
    sendPackets();
#endif
}

// --------------------------------------------------
// Sender
// --------------------------------------------------

extern void cc2420ContinousWave(bool on);

int16_t lplSend(const uint8_t *data, uint16_t length)
{
    // -- send preamble
    uint32_t preambleEndTime = getRealTime() + DUTY_CYCLE_LENGTH;
    cc2420ContinousWave(true);
    while (!timeAfter32(getRealTime(), preambleEndTime));
    cc2420ContinousWave(false);

    // -- send packet
    return radioSend(data, length);
}

void sendPackets(void)
{
    static uint8_t buffer[RADIO_MAX_PACKET];
    uint16_t i;
    int16_t ret;
    radioOn(); // XXX: to avoid switching the radio on and off in cc2420Send()
    for (i = 0; ; ++i) {
        mdelay(1000 + randomRand() % 2000);
        buffer[0] = i;
        redLedOn();
        PRINTF("tx packet... (%#x)\n", *buffer);
        ret = lplSend(buffer, sizeof(buffer));
        if (ret) {
            PRINTF("lplSend failed: %s\n", strerror(-ret));
        }
        redLedOff();
    }
}

// --------------------------------------------------

static uint8_t rxBuffer[RADIO_MAX_PACKET];
static int16_t rxBufferLength;
//static bool packetRxState;

static void recvRadio(void)
{
    if (rxBufferLength == 0) {
        rxBufferLength = radioRecv(rxBuffer, sizeof(rxBuffer));
//        if (*rxBuffer != 0xff) {
//            PRINTF("%#x\n", *rxBuffer);
//        }
    } else {
        radioDiscard();
    }
    // if (len < 0) {
    //     PRINTF("radioRecv failed: %s\n", strerror(-len));
    //     return;
    // }
    // PRINTF("radioRecv %d (%#x)\n", len, *buffer);
}

static inline bool isPacketReceived(void)
{
    uint16_t i;
    if (rxBufferLength <= 0) return false;
    // PRINTF("check if packet is received, len=%d\n", rxBufferLength);
    for (i = 0; i < rxBufferLength; ++i) {
        if (rxBuffer[i] != 0xff) return true;
    }
    return false;
}

bool checkPreamble(void)
{
    uint16_t detect = 0;
    uint16_t i;
    for (i = 0; i < MAX_LPL_CCA_CHECKS; i++) {
        if (rxBufferLength != 0) {
            return true;
        }
        if (!radioIsChannelClear()) {
            ++detect;
            if (detect >= MIN_SAMPLES_BEFORE_DETECT) {
                return true;
            }
        }
        udelay(10);
    }
    return false;
}

uint8_t lastRx;

void rxPacket(void)
{
    bool yes = false;
    uint32_t end = getRealTime() + PACKET_WAIT_TIME; // + 200; // XXX
    do {
        if (rxBufferLength != 0) {
            if (isPacketReceived()) {
                yes = true;
                break;
            }
            rxBufferLength = 0;
        }
    } while (timeAfter32(end, getRealTime()));

    if (yes) {
        PRINTF("rx %d bytes (%#x)\n", rxBufferLength, *rxBuffer);
        if (*rxBuffer != lastRx + 1) {
            PRINTF("missed %d packets!\n", *rxBuffer - (lastRx + 1));
        }
        lastRx = *rxBuffer;
        rxBufferLength = 0;
    } else {
//        PRINT("failed to rx packet...\n");
    }
}

void recvTest(void)
{
    radioOn();
    radioSetReceiveHandle(recvRadio);

    for (;;) {
        greenLedOn();
        uint32_t dutyCycleEnd = getRealTime() + DUTY_CYCLE_LENGTH;
        if (checkPreamble()) {
            rxPacket();
        }
        uint32_t now = getRealTime();
        if (timeAfter32(dutyCycleEnd, now)) {
            uint16_t sleepTime = dutyCycleEnd - now;
            radioOff();
            greenLedOff();
            msleep(sleepTime);
            radioOn();
        }
    }
}


void rxAll() {
    radioRecv(rxBuffer, sizeof(rxBuffer));
}

void ccaTest(void)
{
    radioOn();
//    radioSetReceiveHandle(radioDiscard);
    radioSetReceiveHandle(rxAll);
    for (;;) {
        volatile uint8_t byte = 0;
        uint16_t i;
        for (i = 1; i < 0x100; i <<= 1) {
//            USARTSendByte(1, '+');
            if (radioIsChannelClear()) byte |= i;
//            USARTSendByte(1, '-');
//            udelay(40);
            udelay(1000);
        }
        PRINTF("byte=0x%02x\n", byte);
    }
}
