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

//
// ATMega USART
//

#include "platform.h"
#include <serial.h>
#include <errors.h>
#include <avr/interrupt.h>

volatile Serial_t serial[SERIAL_COUNT];

//===========================================================
// Data types and constants
//===========================================================
/** @brief UBRR calculation for desired baudrate in async mode */
enum {
#if ARDUINO_UNO // TODO: define this automatically
    ASYNC_UBRR_9600 = (CPU_HZ / 4 / 9600 - 1) / 2,
    ASYNC_UBRR_38400 = (CPU_HZ / 4 / 38400 - 1) / 2,
    ASYNC_UBRR_115200 = (CPU_HZ / 4 / 115200 - 1) / 2
#else
    ASYNC_UBRR_9600 = (CPU_HZ / 16) / 9600 - 1,
    ASYNC_UBRR_38400 = (CPU_HZ / 16) / 38400 - 1,
    ASYNC_UBRR_115200 = (CPU_HZ / 16) / 115200 - 1
#endif
};

#define SERIAL_RX_READY() (UCSR0A & (1 << RXC0))
#define SERIAL_RX_ERROR() (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))

#define SERIAL_RX0_READY() SERIAL_RX_READY()
#define SERIAL_RX0_ERROR() SERIAL_RX_ERROR()

#define SERIAL_RX1_READY() (UCSR1A & (1 << RXC1))
#define SERIAL_RX1_ERROR() (UCSR1A & ((1 << FE1) | (1 << DOR1) | (1 << UPE1)))

#define NOT_IMPLEMENTED -ENOSYS

void (*serialRecvCb[SERIAL_COUNT])(uint8_t);

//===========================================================
// Procedures
//===========================================================


uint_t serialInit(uint8_t id, uint32_t speed, uint8_t conf)
{
    // USARTx not supported, x >= USART_COUNT
    if (id >= SERIAL_COUNT) return NOT_IMPLEMENTED;

    // Use asynchronous mode

    /*Set baud rate */
    uint16_t ubrr = 0;
    switch (speed) {
    case 38400:
        ubrr = ASYNC_UBRR_38400;
        break;
    case 115200:
        ubrr = ASYNC_UBRR_115200;
        break;
    case 9600:
    default:
        ubrr = ASYNC_UBRR_9600;
        break;
    }
    if (ubrr == 0) return NOT_IMPLEMENTED;
    UBRR0H = (uint8_t) (ubrr >> 8);
    UBRR0L = (uint8_t) ubrr;

    /* Set frame format: 8data, 1stop bit, no parity check */
    UCSR0C = (0 << USBS0) | (3 << UCSZ00);

    return 0;
}

//-----------------------------------------------------------


// USART RX interrupt handler
#if SERIAL_COUNT > 1

ISR(USART0_RX_vect) {
    if (SERIAL_RX0_READY() && !SERIAL_RX0_ERROR()) {
        const uint8_t x = UDR0;
        if (serialRecvCb[0]) serialRecvCb[0](x);
    } else {
        volatile uint8_t dummy;
        dummy = UDR0;   /* Clear error flags by forcing a dummy read. */
    }
}

ISR(USART1_RX_vect) {
    if (SERIAL_RX1_READY() && !SERIAL_RX1_ERROR()) {
        const uint8_t x = UDR1;
        if (serialRecvCb[1]) serialRecvCb[1](x);
    } else {
        volatile uint8_t dummy;
        dummy = UDR1;   /* Clear error flags by forcing a dummy read. */
    }
}

#else // SERIAL_COUNT == 1

ISR(USART_RX_vect) {
    if (SERIAL_RX_READY() && !SERIAL_RX_ERROR()) {
        const uint8_t x = UDR0;
        if (serialRecvCb[0]) serialRecvCb[0](x);
    } else {
        volatile uint8_t dummy;
        dummy = UDR0;   /* Clear error flags by forcing a dummy read. */
    }
}

#endif // SERIAL_COUNT

//===========================================================
//===========================================================
