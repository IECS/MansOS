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

#ifndef MANSOS_RADIO_H
#define MANSOS_RADIO_H

/// \file
/// Radio chip API
///

#include <defines.h>

//===========================================================
// Data types and constants
//===========================================================

//
// List all supported radio chips here (before including radio_hal.h)
//
#define RADIO_CHIP_CC2420    1
#define RADIO_CHIP_MRF24J40  2
#define RADIO_CHIP_CC1101    3
#define RADIO_CHIP_AMB8420   4
#define RADIO_CHIP_SOFTWARE  5 // simulated radio on x86

typedef void (*RadioRecvFunction)(void);

//===========================================================
// Procedures
//===========================================================

//
// Initialize the radio
//
static inline void radioInit(void);


///
/// (Re)initialize radio communications (serial/SPI) interface
///
static inline void radioReinit(void);

///
/// Send two data buffers to radio.
///
/// Either of the pointers is allowed to be NULL (except both at once)
/// in case the respective length parameters are 0
///
static inline int8_t radioSendHeader(const void *header, uint16_t headerLength,
                                     const void *data, uint16_t dataLength);

///
/// Convenience function for sending just one buffer to radio
///
static inline int8_t radioSend(const void *data, uint16_t dataLength) {
    return radioSendHeader(NULL, 0, data, dataLength);
}

///
/// Convenience function for sending one byte to radio
///
static inline int8_t radioSendByte(uint8_t data) {
    return radioSendHeader(NULL, 0, &data, 1);
}

///
/// Receive data
///
/// Should be called on after the radio driver has signalled the availability of a received packet
///
static inline int16_t radioRecv(void *buffer, uint16_t bufferLength);

///
/// Similar as radioRecv(), but does not keep the result, just clears the receive buffer
///
static inline void radioDiscard(void);

///
/// Set a new radio callback function.
///
/// The callback function is called when a packet becomes available.
/// In general, the callback function should call either radioRecv() to read the packet.
/// Returns: old callback function, if any
///
static inline RadioRecvFunction radioSetReceiveHandle(RadioRecvFunction functionHandle);

///
/// Turn radio listening on
///
/// Note: listening is not required to be turned on to send data!
///
static inline void radioOn(void);

///
/// Turn radio listening off
///
static inline void radioOff(void);

///
/// Get RSSI (Received Signal Strength Indication) of the last received packet
///
static inline int8_t radioGetLastRSSI(void);

///
/// Get LQI (Link Quality Indication) of the last received packet
///
static inline uint8_t radioGetLastLQI(void);

///
/// Measure the current RSSI on the air
///
static inline int radioGetRSSI(void);

///
/// Radio channel control
///
/// The exact behaviour of this function is platform- and chip-dependent.
///
/// For IEEE 802.15.4 compatible radios (such as the CC2420)
/// there are 16 channels available in the 2.4 GHz band
/// in 5 MHz steps, numbered 11 through 26.
/// They have 2MHz channel bandwidth, and channel separation of 5 MHz.
/// Center frequency Fc in MHz is calculated as:\n
///    Fc = 2405 + 5 * (k – 11),                \n
/// where k is channel number.
///
/// For example, channel 11 is 2405 MHz, channel 20: 2450 MHz, channel 26: 2480 MHz.
///
static inline void radioSetChannel(int channel);

///
/// Transmit power control
///
/// The exact behaviour of this function is platform- and chip-dependent.
///
/// For CC2420, a value in range [0 .. 31] is expected,
/// where 0 corresponds to the minimal transmit power and 31 - to the maximum
///
/// On CC2420, the default value is 31. Possible other values:
/// -   31      0 dBm    (1 mW)
/// -   27     -1 dBm    (0.8 mW)
/// -   23     -3 dBm    (0.5 mW)
/// -   19     -5 dBm    (0.3 mW)
/// -   15     -7 dBm    (0.2 mW)
/// -   11    -10 dBm    (0.1 mW)
/// -    7    -15 dBm    (0.03 mW)
/// -    3    -25 dBm    (0.003 mW)
///
static inline void radioSetTxPower(uint8_t power);

///
/// Returns true if CCA detects that transmission medium is free
///
/// Always returns true if the chip doesn't have hardware CCA support
///
static inline bool radioIsChannelClear(void);


//
// Include the HAL file, where radio functions and macros are defined
//
#include <arch/radio_hal.h>

//! The maximum packet size the HW layer can transmit/receive
#ifndef RADIO_MAX_PACKET
#warning Radio constants not defined for this platform!
#define RADIO_MAX_PACKET        0
#define RADIO_TX_POWER_MIN      0
#define RADIO_TX_POWER_MAX      0
#endif

//! The default radio channel number (HW-specific)
#ifndef RADIO_CHANNEL
#define RADIO_CHANNEL 26
#endif

//! The default radio transmission power (in chip-specific units)
#ifndef RADIO_TX_POWER
#define RADIO_TX_POWER 31
#endif

#endif
