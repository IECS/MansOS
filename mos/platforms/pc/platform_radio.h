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

#ifndef PLATFORM_RADIO_H
#define PLATFORM_RADIO_H

#include <defines.h>
#include <radio_hal.h>

//===========================================================
// Data types and constants
//===========================================================
enum {
    PROXY_SERVER_PORT = 6293, // TCP port where sockets subscribe to cloud
    MAX_PACKET_SIZE = 1024,
};

typedef uint16_t PcRadioPackSize_t;

#define RADIO_MAX_PACKET     0xffff
#define RADIO_TX_POWER_MIN        0
#define RADIO_TX_POWER_MAX        0

// ----------------------------------------------

static inline void radioInit(void) {
    pcRadioInit();
}

static inline void radioReinit(void) {

}

static inline int8_t radioSendHeader(const void *header, uint16_t headerLength,
                                     const void *data, uint16_t dataLength) {
    return pcRadioSendHeader(header, headerLength, data, dataLength);
}

static inline int16_t radioRecv(void *buffer, uint16_t bufferLength) {
    return pcRadioRecv(buffer, bufferLength);
}

static inline void radioDiscard(void) {
    pcRadioDiscard();
}

static inline RadioRecvFunction radioSetReceiveHandle(
        RadioRecvFunction functionHandle) {
    return pcRadioSetReceiveHandle(functionHandle);
}

static inline void radioOn(void) {
    pcRadioOn();
}

static inline void radioOff(void) {
    pcRadioOff();
}

static inline int radioGetRSSI(void) {
    return pcRadioGetRSSI();
}

static inline int8_t radioGetLastRSSI(void) {
    return pcRadioGetLastRSSI();
}

static inline uint8_t radioGetLastLQI(void) {
    return pcRadioGetLastLQI();
}

static inline void radioSetChannel(int channel) {
    pcRadioSetChannel(channel);
}

static inline void radioSetTxPower(uint8_t power) {
    pcRadioSetTxPower(power);
}

static inline bool radioIsChannelClear(void) {
    return pcRadioIsChannelClear();
}

#endif
