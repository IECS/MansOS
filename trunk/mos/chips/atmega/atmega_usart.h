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

#ifndef ATMEGA_USART_H
#define ATMEGA_USART_H

// available USART count
#ifdef PLATFORM_WASPMOTE
#define SERIAL_COUNT 2
#else
#define SERIAL_COUNT 1
#endif

// use the only USART for PRINTF
#define PRINTF_SERIAL_ID 0

static inline void serialSendByte(uint8_t id, uint8_t data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1 << UDRE0))) { }
    /* Put data into buffer, sends the data */
    UDR0 = data;
}

static inline void serialEnableTX(uint8_t id)
{
    UCSR0B |= (1 << TXEN0);
}

static inline void serialDisableTX(uint8_t id)
{
    UCSR0B &= ~(1 << TXEN0);
}

static inline void serialEnableRX(uint8_t id)
{
    // enable RX and interrupt
    UCSR0B |= (1 << RXEN0) | (1 << RXCIE0);
}

static inline void serialDisableRX(uint8_t id)
{
    UCSR0B &= ~(1 << RXEN0);
}

#endif
