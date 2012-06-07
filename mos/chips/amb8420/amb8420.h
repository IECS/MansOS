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

#ifndef MANSOS_AMB8420_H
#define MANSOS_AMB8420_H

#include <kernel/defines.h>

#define AMB8420_MAX_PACKET_LEN      128

#define AMB8420_TX_POWER_MIN        0  // -38.75 dBm
#define AMB8420_TX_POWER_MAX        31 // 0 dBm

void amb8420Init(void) WEAK_SYMBOL;

void amb8420On(void);
void amb8420Off(void);

int amb8420Read(void *buf, uint16_t bufsize);
int amb8420Send(const void *header, uint16_t headerLen,
                 const void *data, uint16_t dataLen);

void amb8420Discard(void);

// set receive callback
typedef void (*AMB8420RxHandle)(void);
AMB8420RxHandle amb8420SetReceiver(AMB8420RxHandle);

// radio channel control
void amb8420SetChannel(int channel);

// transmit power control (0..31)
void amb8420SetTxPower(uint8_t power);

// get RSSI (Received Signal Strength Indication) of the last received packet
int8_t amb8420GetLastRSSI(void);

// get LQI (Link Quality Indication) of the last received packet. Return value is in [0..127]
uint8_t amb8420GetLastLQI(void);

// measure the current RSSI on air
int amb8420GetRSSI(void);

// returns true if CCA fails to detect radio interference
bool amb8420IsChannelClear(void);

void amb8420PollForPacket(void);

#endif
