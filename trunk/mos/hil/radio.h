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

/*
 * Radio chip API
 */

#ifndef MANSOS_RADIO_H
#define MANSOS_RADIO_H

#include <kernel/defines.h>

//===========================================================
// Data types and constants
//===========================================================

//
// List all supported radio chips here (before including radio_hal.h)
//
#define RADIO_CHIP_CC2420    1
#define RADIO_CHIP_MRF24J40  2

typedef void (*RadioRecvFunction)(void);

//
// Include the HAL file, where radio functions and macros are defined
//
#include "radio_hal.h"

#ifndef RADIO_MAX_PACKET
#warning Radio constants not defined for this platform!
#define RADIO_MAX_PACKET        0
#define RADIO_TX_POWER_MIN      0
#define RADIO_TX_POWER_MAX      0
#endif

#ifndef RADIO_CHANNEL
#define RADIO_CHANNEL 26
#endif

#ifndef RADIO_TX_POWER
#define RADIO_TX_POWER 31
#endif

//===========================================================
// Procedures
//===========================================================

//
// Initialize the radio
//
void radioInit(void);

//
// Send two data buffers to radio; either of the pointers is allowed to be NULL
// (except both at the same time) in case the respective length paramater is 0
//
int8_t radioSendHeader(const void *header, uint16_t headerLength,
                       const void *data, uint16_t dataLength);

//
// Convenience function for sending just one buffer to radio
//
static inline int8_t radioSend(const void *data, uint16_t dataLength) {
    return radioSendHeader(NULL, 0, data, dataLength);
}

//
// Convenience function for sending one byte to radio
//
static inline int8_t radioSendByte(uint8_t data) {
    return radioSendHeader(NULL, 0, &data, 1);
}

//
// Receive data; should be called on after the radio driver has signalled
// data availability. 
//
int16_t radioRecv(void *buffer, uint16_t bufferLength);

//
// Similar as radioRecv(), but does not keep the result, just clear the rx buffer.
//
void radioDiscard(void);

//
// Set a new radio callback function.
// The callback function is called when a packet becomes available.
// In general, the callback function should call either radioRecv() to read the packet.
// Returns: old callack function, if any
//
RadioRecvFunction radioSetReceiveHandle(RadioRecvFunction functionHandle);

//
// Turn the radio listening on. Note that listening is not required
// to be turned on if radio is used only to send data
//
void radioOn(void);

//
// Turn radio listening off (for example, to save energy)
//
void radioOff(void);

//
// Measure the current RSSI on air
//
int radioGetRSSI(void);

//
// Get RSSI (Received Signal Strength Indication) of the last received packet
//
int8_t radioGetLastRSSI(void);

//
// Get LQI (Link Quality Indication) of the last received packet
//
uint8_t radioGetLastLQI(void);

//
// Radio channel control.
// The exact behaviour of these functions is platform- and chip-dependent.
// For IEEE 802.15.4 compatible radios (such as the CC2420)
// there are 16 channels available in the 2.4 GHz band
// in 5 MHz steps, numbered 11 through 26
//
void radioSetChannel(int channel);

//
// Transmit power control.
// The exact behaviour of these functions is platform- and chip-dependent.
// For CC2420, a value in range [0 .. 31] is expected,
// where 0 corresponds to the minimal transmit power and 31 - to the maximum
//
void radioSetTxPower(uint8_t power);

//
// Returns true if CCA detects that transmission medium is free.
// Always returns true if the chip does not havbe support for hardware CCA.
//
bool radioIsChannelClear(void);

#endif
