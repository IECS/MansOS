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

//
// ATMega USART
//

#include "platform.h"
#include <serial.h>
#include <avr/interrupt.h>

enum ATMegaUSARTMode_e {
    UM_DISABLED,
    UM_UART,
    UM_SPI,
} PACKED;

typedef enum ATMegaUSARTMode_e ATMegaUSARTMode_t;

// actual USARTx mode (UART/SPI/I2C)
static ATMegaUSARTMode_t usartMode[USART_COUNT] = { UM_DISABLED };

//===========================================================
// Data types and constants
//===========================================================
/** @brief UBRR calculation for desired baudrate in async mode */
enum {
#if 0
    ASYNC_UBRR_9600 = (CPU_HZ / 4 / 9600 - 1) / 2,
    ASYNC_UBRR_38400 = (CPU_HZ / 4 / 38400 - 1) / 2,
    ASYNC_UBRR_115200 = (CPU_HZ / 4 / 115200 - 1) / 2
#else
    ASYNC_UBRR_9600 = (CPU_HZ / 16) / 9600 - 1,
    ASYNC_UBRR_38400 = (CPU_HZ / 16) / 38400 - 1,
    ASYNC_UBRR_115200 = (CPU_HZ / 16) / 115200 - 1
#endif
};

#define USART_RX_READY() (UCSR0A & (1 << RXC0))
#define USART_RX_ERROR() (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))

#define USART_RX0_READY() USART_RX_READY()
#define USART_RX0_ERROR() USART_RX_ERROR()

#define USART_RX1_READY() (UCSR1A & (1 << RXC1))
#define USART_RX1_ERROR() (UCSR1A & ((1 << FE1) | (1 << DOR1) | (1 << UPE1)))

#define NOT_IMPLEMENTED -1

void (*usartRecvCb)(uint8_t) = 0;

//===========================================================
// Procedures
//===========================================================


//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTInit(uint8_t id, uint32_t speed, uint8_t conf)
{
    // USARTx not supported, x >= USART_COUNT
    if (id >= USART_COUNT) return NOT_IMPLEMENTED;

    // we could want to change UART speed - allow to call this
    // function multiple times
//    if (usartMode[id] == UM_UART) return 0; // already in UART mode

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

    usartMode[id] = UM_UART;

    return 0;
}; 


//-----------------------------------------------------------
//-----------------------------------------------------------

//uint_t USARTInitSPI(uint8_t id)
//{
//    // TODO
//    return NOT_IMPLEMENTED;
//};


//-----------------------------------------------------------
//-----------------------------------------------------------

//uint_t USARTInitI2C(uint8_t id)
//{
//    // TODO
//    return NOT_IMPLEMENTED;
//};


//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTSendByte(uint8_t id, uint8_t data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1 << UDRE0))) { }
    /* Put data into buffer, sends the data */
    UDR0 = data;
    return 0;
}; 

//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTEnableTX(uint8_t id)
{
    UCSR0B |= (1 << TXEN0);
    return 0;
}; 

//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTDisableTX(uint8_t id)
{
    UCSR0B &= ~(1 << TXEN0);
    return 0;
}; 


//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTEnableRX(uint8_t id)
{
    // enable RX and interrupt
    UCSR0B |= (1 << RXEN0) | (1 << RXCIE0);
    return 0;
}; 


//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTDisableRX(uint8_t id)
{
    UCSR0B &= ~(1 << RXEN0);
    return 0;
}; 


//-----------------------------------------------------------
//-----------------------------------------------------------

//uint_t USARTisUSART(uint8_t id)
//{
//    return 1; // so far only USART mode supported
//};


//-----------------------------------------------------------
//-----------------------------------------------------------

//uint_t USARTisSPI(uint8_t id)
//{
//    return 0;
//};


//-----------------------------------------------------------
//-----------------------------------------------------------

//uint_t USARTisI2C(uint8_t id)
//{
//    return 0;
//};

//-----------------------------------------------------------
//-----------------------------------------------------------


// USART RX interrupt handler
#if USART_COUNT > 1

ISR(USART0_RX_vect) {
    if (USART_RX0_READY() && !USART_RX0_ERROR()) {
        const uint8_t x = UDR0;
        if (usartRecvCb) usartRecvCb(x);
    } else {
        volatile uint8_t dummy;
        dummy = UDR0;   /* Clear error flags by forcing a dummy read. */
    }
}

ISR(USART1_RX_vect) {
    if (USART_RX1_READY() && !USART_RX1_ERROR()) {
        const uint8_t x = UDR1;
        if (usartRecvCb) usartRecvCb(x);
    } else {
        volatile uint8_t dummy;
        dummy = UDR1;   /* Clear error flags by forcing a dummy read. */
    }
}

#else // USART_COUNT == 1

ISR(USART_RX_vect) {
    if (USART_RX_READY() && !USART_RX_ERROR()) {
        const uint8_t x = UDR0;
        if (usartRecvCb) usartRecvCb(x);
    } else {
        volatile uint8_t dummy;
        dummy = UDR0;   /* Clear error flags by forcing a dummy read. */
    }
}

#endif // USART_COUNT

//===========================================================
//===========================================================
