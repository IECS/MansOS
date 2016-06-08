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

#ifndef MANSOS_CC2420_H
#define MANSOS_CC2420_H

#include <defines.h>
#include "cc2420_const.h"

//
// Configuration constants
//
#define WITH_SEND_CCA 1
#define CC2420_CONF_CHECKSUM 0
#define CC2420_CONF_AUTOACK 0
#define CC2420_CONF_AUTOCRC 1

#if CC2420_CONF_CHECKSUM
#define CHECKSUM_LEN 2
#else
#define CHECKSUM_LEN 0
#endif

#define FOOTER_LEN 2
#define AUX_LEN (CHECKSUM_LEN + FOOTER_LEN)

#define CC2420_HW_PACKET_LIMIT     127

#define CC2420_MAX_PACKET_LEN      (CC2420_HW_PACKET_LIMIT - AUX_LEN)

#define CC2420_TX_POWER_MIN        0
#define CC2420_TX_POWER_MAX        31

void cc2420Init(void) WEAK_SYMBOL;

void cc2420On(void);
void cc2420Off(void);

int_t cc2420Read(void *buf, uint16_t bufsize);
int_t cc2420Send(const void *header, uint16_t headerLen,
                 const void *data, uint16_t dataLen);

void cc2420Discard(void);

// set receive callback
typedef void (*CC2420RxHandle)(void);
CC2420RxHandle cc2420SetReceiver(CC2420RxHandle);

// radio channel control
void cc2420SetChannel(int channel);

// transmit power control (0..31)
void cc2420SetTxPower(uint8_t power);

// get RSSI (Received Signal Strength Indication) of the last received packet
int8_t cc2420GetLastRSSI(void);

// get LQI (Link Quality Indication) of the last received packet. Return value is in [0..127]
uint8_t cc2420GetLastLQI(void);

// measure the current RSSI on air
int cc2420GetRSSI(void);

// returns true if CCA fails to detect radio interference;
// radio must be turned on before calling this!
bool cc2420IsChannelClear(void);

void cc2420InitSpi(void);

// Send continous wave (unmodulated signal carrier)
void cc2420ContinousWave(bool on);

// Get the CC2420 status byte. Check enum cc2420_status_byte.
uint8_t cc2420GetStatus(void);

// More API as Macro definitions
#define cc2420IsTxBusy()  ( cc2420GetStatus() & (1 << CC2420_TX_ACTIVE))

#endif
