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

#ifndef RADIO_HAL_MRF_H
#define RADIO_HAL_MRF_H

#include <mrf24j40/mrf24j40.h>

static inline void radioInit(void) {
    if (mrf24j40Init) mrf24j40Init();
}

static inline int8_t radioSendHeader(const void *header, uint16_t headerLength,
                                     const void *data, uint16_t dataLength) {
    return mrf24j40Send(header, headerLength, data, dataLength);
}

static inline int16_t radioRecv(void *buffer, uint16_t bufferLength) {
    return mrf24j40Read(buffer, bufferLength);
}

static inline void radioDiscard(void) {
    mrf24j40Discard();
}

static inline RadioRecvFunction radioSetReceiveHandle(
        RadioRecvFunction functionHandle) {
    return mrf24j40SetReceiver(functionHandle);
}

static inline void radioOn(void) {
    mrf24j40On();
}

static inline void radioOff(void) {
    mrf24j40Off();
}

static inline int radioGetRSSI(void) {
    return mrf24j40GetRSSI();
}

static inline int8_t radioGetLastRSSI(void) {
    return mrf24j40GetLastRSSI();
}

static inline uint8_t radioGetLastLQI(void) {
    return mrf24j40GetLastLQI();
}

static inline void radioSetChannel(int channel) {
    mrf24j40SetChannel(channel);
}

static inline void radioSetTxPower(uint8_t power) {
    mrf24j40SetTxPower(power);
}

static inline bool radioIsChannelClear(void) {
    return mrf24j40IsChannelClear();
}

#define RADIO_MAX_PACKET          MRF24J40_MAX_PACKET_LEN
#define RADIO_TX_POWER_MIN        MRF24J40_TX_POWER_MIN
#define RADIO_TX_POWER_MAX        MRF24J40_TX_POWER_MAX

#endif
