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

#ifndef RADIO_ATMEGA_HAL_H
#define RADIO_ATMEGA_HAL_H

// By default, no radio for ATMega platform

static inline void radioInit(void) {
}

static inline int8_t radioSendHeader(const void *header, uint16_t headerLength,
                                     const void *data, uint16_t dataLength) {
    return 0;
}

static inline int16_t radioRecv(void *buffer, uint16_t bufferLength) {
    return 0;
}

static inline void radioDiscard(void) {
}

static inline RadioRecvFunction radioSetReceiveHandle(RadioRecvFunction functionHandle) {
    return 0;
}

static inline void radioOn(void) {
}

static inline void radioOff(void) {
}

static inline int radioGetRSSI(void) {
    return 0;
}

static inline int8_t radioGetLastRSSI(void) {
    return 0;
}

static inline uint8_t radioGetLastLQI(void) {
    return 0;
}

static inline void radioSetChannel(int channel) {
}

static inline void radioSetTxPower(uint8_t power) {
}

static inline bool radioIsChannelClear(void) {
    return true;
}

#define RADIO_MAX_PACKET          0
#define RADIO_TX_POWER_MIN        0
#define RADIO_TX_POWER_MAX        0

#endif
