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

#ifndef MANSOS_SERIAL_H
#define MANSOS_SERIAL_H

//
// Serial port interface
//

#include <platform.h>

//===========================================================
// Macros
//===========================================================


//===========================================================
// Data types and constants
//===========================================================
// callback for USART RX interrupt
typedef void (* USARTCallback_t)(uint8_t);

//===========================================================
// Procedures
//===========================================================

// Implemented in HIL
void USARTSendString(uint8_t id, char *string);
void USARTSendData(uint8_t id, uint8_t *data, uint16_t len);

/**
 * Set callback function for per-byte data receive. The callback is called
 * on every received packet
 * @param id - ID of the UART used (See MCU datasheet to get IDs)
 * @param cb - callback function: void myCallback(uint8_t byte)
 */
uint_t USARTSetReceiveHandle(uint8_t id, USARTCallback_t cb);

/**
 * Set callback for per-packet data receive. Stores the received bytes in
 * the buffer and the callback is called when either a newline is received
 * ('\n', binary value 10) or at most len bytes are received. The newline is
 * also stored in the buffer
 * Also enables USART RX automatically.
 * After the callback, buffer is reset and reception restarts.
 * Warning: Can use only one USART at a time (single buffer, single handler)!
 *
 * @param id - ID of the UART used (See MCU datasheet to get IDs)
 * @param cb - callback function: void myCallback(uint8_t bytes). Here the
 *             bytes parameter contains not the last byte received but
 *             total received byte count (i.e., bytes stored in the buffer)!
 * @param len - size of the buffer in bytes. Callback is called when len
 *              bytes are received (or when '\n' is received).
 *              When len is zero, no packet size is checked, only on newline
 *              reception the callback is called.
 */
uint_t USARTSetPacketReceiveHandle(uint8_t id, USARTCallback_t cb,
        void *buffer, uint16_t len);

// Implemented in HPL
uint_t USARTInit(uint8_t id, uint32_t speed, uint8_t conf);
// Not on all platforms SPI and I2C is part of USART
//uint_t USARTInitSPI(uint8_t id);
//uint_t USARTInitI2C(uint8_t id);

uint_t USARTSendByte(uint8_t id, uint8_t data);

uint_t USARTEnableTX(uint8_t id);
uint_t USARTDisableTX(uint8_t id);
uint_t USARTEnableRX(uint8_t id);
uint_t USARTDisableRX(uint8_t id);

//uint_t USARTisUART(uint8_t id);
//uint_t USARTisSPI(uint8_t id);
//uint_t USARTisI2C(uint8_t id);

enum {
    USART_FUNCTION_SERIAL = 1, // used as serial interface
    USART_FUNCTION_RADIO,      // used as radio chip's interface
    USART_FUNCTION_FLASH,      // used as ext flash interface
    USART_FUNCTION_SDCARD,     // used as sdcard interface
};

extern volatile bool usartBusy[USART_COUNT];
// one of usart modes
extern uint8_t usartFunction[USART_COUNT];


//===========================================================
//===========================================================

#endif
