/**
 * Copyright (c) 2011, Institute of Electronics and Computer Science
 * All rights reserved.
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
 *
 * cc1101.h -- CC1101 radio driver
 */

#ifndef _CHIPS_CC1101_H_
#define _CHIPS_CC1101_H_

#define CC1101_MAX_PACKET_LEN  62
#define CC1101_SYNC_WORD       0xAAAAU
#define CC1101_DEFAULT_CHANNEL 0
#define CC1101_TX_POWER        0x50 /* 0 dBm at 868 MHz */
#define CC1101_TX_POWER_MIN    0x03
#define CC1101_TX_POWER_MAX    0xC0

typedef void (*cc1101Callback_t)(void);

void cc1101Init(void);

void cc1101On(void);
void cc1101Off(void);

void cc1101SetChannel(int channel);
void cc1101SetTxPower(uint8_t power);
void cc1101SetAddress(uint8_t addr);
void cc1101SetRecvCallback(cc1101Callback_t func);

int8_t cc1101GetRSSI(void);
int8_t cc1101GetLastRSSI(void);
uint8_t cc1101GetLastLQI(void);

int8_t cc1101Send(const uint8_t *header, uint8_t hlen,
                  const uint8_t *data,   uint8_t dlen);
int8_t cc1101Read(uint8_t *buf, uint8_t buflen);
void cc1101Discard(void);

bool cc1101IsChannelClear(void);

#endif /* _CHIPS_CC1101_H_ */
