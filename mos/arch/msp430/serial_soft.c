/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#include "serial_soft.h"
#include <serial.h>
#include <errors.h>
#include <digital.h>
#include <delay.h>
#include <leds.h>

#define SERIAL_BAUDRATE 9600

#define TIMERA_CLOCK (CPU_HZ)
#define UART_BITTIME ((TIMERA_CLOCK / SERIAL_BAUDRATE))
#define UART_HALF_BITTIME   UART_BITTIME / 2

#define UART0_PORT    3
#define UART0_TX_PIN  4
#define UART0_RX_PIN  5

#define UART1_PORT    3
#define UART1_TX_PIN  6
#define UART1_RX_PIN  7

// ---------------------------------------------

SerialCallback_t serialRecvCb[SERIAL_COUNT];
volatile Serial_t serial[SERIAL_COUNT];

// ---------------------------------------------

uint_t serialInit(uint8_t id, uint32_t speed, uint8_t conf)
{
    if (conf != 0) return -ENOSYS;
    if (speed != 9600) return -ENOSYS;

    if (id == 0) {
        pinAsData(UART0_PORT, UART0_TX_PIN);
        pinAsOutput(UART0_PORT, UART0_TX_PIN);

        pinAsData(UART0_PORT, UART0_RX_PIN);
        pinAsInput(UART0_PORT, UART0_RX_PIN);
    } else {
        pinAsData(UART1_PORT, UART1_TX_PIN);
        pinAsOutput(UART1_PORT, UART1_TX_PIN);

        pinAsData(UART1_PORT, UART1_RX_PIN);
        pinAsInput(UART1_PORT, UART1_RX_PIN);
    }

    // Configure Timer_A
    TACTL = TACLR;
    TACTL = TASSEL_2 + MC_2; // clock = SMCLK

    // Initialize UART
    if (id == 0) {
        pinSet(UART0_PORT, UART0_TX_PIN);
    } else {
        pinSet(UART1_PORT, UART1_TX_PIN);
    }
    TACCTL1 = 0;

    TACCR0 = TAR + 100;
    TACCTL0 = CCIE;

    return 0;
}

void serialSendByte(uint8_t id, uint8_t data)
{
    uint16_t txd = data | 0x300;   // transmitter "shift register"

    // Start transmitter. This has to be done with interrupts disabled
    DISABLE_INTS();
    TACCTL1 = 0;                    // transmit start bit
    if (id == 0) {
        pinClear(UART0_PORT, UART0_TX_PIN);
    } else {
        pinClear(UART1_PORT, UART1_TX_PIN);
    }
    TACCR1 = TAR + UART_BITTIME;    // set time till the first data bit

    //XXX: the stability is MUCH worse if ints are enabled at this point
    //  ENABLE_INTS();

    // wait until the end of start bit
    while (0 == (TACCTL1 & CCIFG));
    TACCR1 += UART_BITTIME;

    while (txd) {
        if (txd & 1) {
            // transmit "Mark" (1) using OUTMOD=1 (Set)
            TACCTL1 = OUTMOD_1;
            if (id == 0) {
                pinSet(UART0_PORT, UART0_TX_PIN);
            } else {
                pinSet(UART1_PORT, UART1_TX_PIN);
            }
        } else {
            // transmit "Space" (0) using OUTMOD=5 (Reset)
            TACCTL1 = OUTMOD_5;
            if (id == 0) {
                pinClear(UART0_PORT, UART0_TX_PIN);
            } else {
                pinClear(UART1_PORT, UART1_TX_PIN);
            }
        }

        // wait for TA compare event
        while (0 == (TACCTL1 & CCIFG));
        // set next bit time
        TACCR1 += UART_BITTIME;

        txd >>= 1;
    }

    // all bits sent out; set TxD idle to "Mark"
    TACCTL1 = OUT;
    if (id == 0) {
        pinSet(UART0_PORT, UART0_TX_PIN);
    } else {
        pinSet(UART1_PORT, UART1_TX_PIN);
    }

    ENABLE_INTS();
}

volatile uint8_t rxByte;
volatile bool rxOk;

ALARM_TIMER_INTERRUPT0()
{
    uint8_t bitnum;

    if (rxOk) goto end;

    // start of reception
    if (pinRead(UART1_PORT, UART1_RX_PIN)) {
        goto end;
    }

    // start bit detected!
    TACCTL0 = OUT;
    TACCR0 += UART_HALF_BITTIME - 100;
    while (0 == (TACCTL0 & CCIFG));

    rxByte = 0;
    for (bitnum = 0; bitnum < 8; ++bitnum) {
        // wait for next data bit
        TACCTL0 = OUT;
        TACCR0 += UART_BITTIME;
        while (0 == (TACCTL0 & CCIFG));

        rxByte |= pinRead(UART1_PORT, UART1_RX_PIN) << bitnum;
    }

    // wait for the stop bit
    TACCTL0 = OUT;
    TACCR0 += UART_BITTIME;
    while (0 == (TACCTL0 & CCIFG));
    // ok if stop bit is 1
    rxOk = pinRead(UART1_PORT, UART1_RX_PIN);

    TACCTL0 = CCIE;

  end:
    // heuristic...
    TACCR0 = TAR + UART_HALF_BITTIME;
}
