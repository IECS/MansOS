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

#ifndef MANSOS_SERIAL_H
#define MANSOS_SERIAL_H

/// \file
/// Serial port (RS232) interface
///

#include <platform.h>

//===========================================================
// Macros
//===========================================================

//! The number of serial ports available on the platform
#ifndef SERIAL_COUNT
#define SERIAL_COUNT 0
#endif


//===========================================================
// Data types and constants
//===========================================================
//! callback for Serial RX interrupt
typedef void (* SerialCallback_t)(uint8_t);

//===========================================================
// Procedures
//===========================================================

///
/// Initalize serial port with specific speed
///
/// 'conf' value is not used at the moment.
///
uint_t serialInit(uint8_t id, uint32_t speed, uint8_t conf);

//! Enable transmission on a specific serial port
void serialEnableTX(uint8_t id);
//! Disable transmission on a specific serial port
void serialDisableTX(uint8_t id);
//! Enable reception on a specific serial port
void serialEnableRX(uint8_t id);
//! Disable reception on a specific serial port
void serialDisableRX(uint8_t id);

//! Send a single byte to a specific serial port
void serialSendByte(uint8_t id, uint8_t data);
//! Send binary data to a specific serial port
void serialSendData(uint8_t id, const uint8_t *data, uint16_t len);
///
/// Send ASCII string to a specific serial port.
///
/// Note: a single '\\n' symbol is converted to '\\r\\n'!
///
void serialSendString(uint8_t id, const char *string);

/**
 * Set callback function for per-byte data receive. The callback is called
 * on every received packet
 * @param id - ID of the UART used (See MCU datasheet to get IDs)
 * @param cb - callback function: void myCallback(uint8_t byte)
 */
uint_t serialSetReceiveHandle(uint8_t id, SerialCallback_t cb);

/**
 * Set callback for per-packet data receive. Stores the received bytes in
 * the buffer and the callback is called when either a newline is received
 * ('\\n', binary value 10) or at most len bytes are received. The newline is
 * also stored in the buffer.
 * Also enables Serial RX automatically.
 * After the callback, buffer is reset and reception restarts.
 * Warning: Can use only one Serial at a time (single buffer, single handler)!
 *
 * @param id - ID of the UART used (See MCU datasheet to get IDs)
 * @param cb - callback function: void myCallback(uint8_t bytes). Here the
 *             bytes parameter contains not the last byte received but
 *             total received byte count (i.e., bytes stored in the buffer)!
 * @param len - size of the buffer in bytes. Callback is called when len
 *              bytes are received (or when '\\n' is received).
 *              When len is zero, no packet size is checked, only on newline
 *              reception the callback is called.
 */
uint_t serialSetPacketReceiveHandle(uint8_t id, SerialCallback_t cb,
        void *buffer, uint16_t len);

///
/// Possible functions of a hardware serial module.
/// Only one of these functions can be active at a given time
/// for a single module (although run-time reconfiguration is possible)
///
enum {
    SERIAL_FUNCTION_UNUSED,
    SERIAL_FUNCTION_PRINT,   // used for debug output
    SERIAL_FUNCTION_RADIO,   // used as radio chip's interface
    SERIAL_FUNCTION_FLASH,   // used as flash chip's interface
    SERIAL_FUNCTION_SDCARD,  // used as sdcard interface
};

//! Serial port information
struct Serial_s {
    //! Is the port actively used at the moment?
    uint8_t busy : 1;
    //! To what function is the port used/configured for?
    uint8_t function : 3;
} PACKED;
typedef struct Serial_s Serial_t;

//! Run-time serial port configuration
extern volatile Serial_t serial[SERIAL_COUNT];

//===========================================================
//===========================================================

#endif
