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

#ifndef MANSOS_MRF24J40_H
#define MANSOS_MRF24J40_H

#include <kernel/defines.h>

//
// Configuration constants
//
#define WITH_SEND_CCA 1
#define MRF24J40_CONF_CHECKSUM 0
#define MRF24J40_CONF_AUTOACK 0
#define MRF24J40_CONF_AUTOCRC 1

#if MRF24J40_CONF_CHECKSUM
#define CHECKSUM_LEN 2
#else
#define CHECKSUM_LEN 0
#endif

#define FOOTER_LEN 2
#define AUX_LEN (CHECKSUM_LEN + FOOTER_LEN)

#define MRF24J40_MAX_PACKET_LEN      (127 - AUX_LEN)

#define MRF24J40_TX_POWER_MIN        0  // -38.75 dBm
#define MRF24J40_TX_POWER_MAX        31 // 0 dBm

void mrf24j40Init(void) WEAK_SYMBOL;

void mrf24j40On(void);
void mrf24j40Off(void);

int mrf24j40Read(void *buf, uint16_t bufsize);
int mrf24j40Send(const void *header, uint16_t headerLen,
                 const void *data, uint16_t dataLen);

void mrf24j40Discard(void);

// set receive callback
typedef void (*MRF24J40RxHandle)(void);
MRF24J40RxHandle mrf24j40SetReceiver(MRF24J40RxHandle);

// radio channel control
void mrf24j40SetChannel(int channel);

// transmit power control (0..31)
void mrf24j40SetTxPower(uint8_t power);

// get RSSI (Received Signal Strength Indication) of the last received packet
int8_t mrf24j40GetLastRSSI(void);

// get LQI (Link Quality Indication) of the last received packet. Return value is in [0..127]
uint8_t mrf24j40GetLastLQI(void);

// measure the current RSSI on air
int mrf24j40GetRSSI(void);

// returns true if CCA fails to detect radio interference
bool mrf24j40IsChannelClear(void);

// TODO: remove this and use interrupts instead!
bool mrf24j40PollForPacket(void) WEAK_SYMBOL;

#endif
