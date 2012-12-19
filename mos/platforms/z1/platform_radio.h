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

#ifndef PLATFORM_RADIO_H
#define PLATFORM_RADIO_H

#include "platform.h"

#if RADIO_CHIP == RADIO_CHIP_CC2420

#include <cc2420/cc2420.h>

static inline void radioInit(void) {
    if (cc2420Init) cc2420Init();
}

static inline void radioReinit(void) {
    cc2420InitSpi();
}

static inline int8_t radioSendHeader(const void *header, uint16_t headerLength,
                                     const void *data, uint16_t dataLength) {
    return cc2420Send(header, headerLength, data, dataLength);
}

static inline int16_t radioRecv(void *buffer, uint16_t bufferLength) {
    return cc2420Read(buffer, bufferLength);
}

static inline void radioDiscard(void) {
    cc2420Discard();
}

static inline RadioRecvFunction radioSetReceiveHandle(
        RadioRecvFunction functionHandle) {
    return cc2420SetReceiver(functionHandle);
}

static inline void radioOn(void) {
    cc2420On();
}

static inline void radioOff(void) {
    cc2420Off();
}

static inline int radioGetRSSI(void) {
    return cc2420GetRSSI();
}

static inline int8_t radioGetLastRSSI(void) {
    return cc2420GetLastRSSI();
}

static inline uint8_t radioGetLastLQI(void) {
    return cc2420GetLastLQI();
}

static inline void radioSetChannel(int channel) {
    cc2420SetChannel(channel);
}

static inline void radioSetTxPower(uint8_t power) {
    cc2420SetTxPower(power);
}

static inline bool radioIsChannelClear(void) {
    return cc2420IsChannelClear();
}

#define RADIO_MAX_PACKET          CC2420_MAX_PACKET_LEN
#define RADIO_TX_POWER_MIN        CC2420_TX_POWER_MIN
#define RADIO_TX_POWER_MAX        CC2420_TX_POWER_MAX

#else

#include <amb8420/amb8420.h>

static inline void radioInit(void) {
    if (amb8420Init) amb8420Init();
}

static inline void radioReinit(void) {
    amb8420InitSerial();
}

static inline int8_t radioSendHeader(const void *header, uint16_t headerLength,
                                     const void *data, uint16_t dataLength) {
    return amb8420Send(header, headerLength, data, dataLength);
}

static inline int16_t radioRecv(void *buffer, uint16_t bufferLength) {
    return amb8420Read(buffer, bufferLength);
}

static inline void radioDiscard(void) {
    amb8420Discard();
}

static inline RadioRecvFunction radioSetReceiveHandle(
        RadioRecvFunction functionHandle) {
    return amb8420SetReceiver(functionHandle);
}

static inline void radioOn(void) {
    amb8420On();
}

static inline void radioOff(void) {
    amb8420Off();
}

static inline int radioGetRSSI(void) {
    return amb8420GetRSSI();
}

static inline int8_t radioGetLastRSSI(void) {
    return amb8420GetLastRSSI();
}

static inline uint8_t radioGetLastLQI(void) {
    return amb8420GetLastLQI();
}

static inline void radioSetChannel(int channel) {
    amb8420SetChannel(channel);
}

static inline void radioSetTxPower(uint8_t power) {
    amb8420SetTxPower(power);
}

static inline bool radioIsChannelClear(void) {
    return amb8420IsChannelClear();
}

#define RADIO_MAX_PACKET          AMB8420_MAX_PACKET_LEN
#define RADIO_TX_POWER_MIN        AMB8420_TX_POWER_MIN
#define RADIO_TX_POWER_MAX        AMB8420_TX_POWER_MAX

#endif

#endif
