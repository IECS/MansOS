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

#define SOFT_SERIAL_BAUDRATE 9600

#define TIMER_CLOCK (CPU_HZ)
#define UART_BITTIME ((TIMER_CLOCK / SOFT_SERIAL_BAUDRATE))
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

bool serialRxEnabled[SERIAL_COUNT];
bool serialTxEnabled[SERIAL_COUNT];

// ---------------------------------------------

void serialEnableTX(uint8_t id) {
    serialTxEnabled[id] = true;
}

void serialDisableTX(uint8_t id) {
    serialTxEnabled[id] = false;
}

void serialEnableRX(uint8_t id) {
    serialRxEnabled[id] = true;
}

void serialDisableRX(uint8_t id) {
    serialRxEnabled[id] = false;
}


uint_t serialInit(uint8_t id, uint32_t speed, uint8_t conf)
{
    if (conf != 0) return -ENOSYS;
    // support only one baudrate (XXX: panic on error?)
    if (speed != SOFT_SERIAL_BAUDRATE) return -ENOSYS;

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

    // Configure Timer_B
    TBCTL = TBCLR;
    TBCTL = TBSSEL_2 + MC_2; // clock = SMCLK

    // Initialize UART
    if (id == 0) {
        pinSet(UART0_PORT, UART0_TX_PIN);
    } else {
        pinSet(UART1_PORT, UART1_TX_PIN);
    }
    TBCCTL1 = 0;

    TBCCR0 = TBR + 100;
    TBCCTL0 = CCIE;
    // TBCCTL0 = 0;

    return 0;
}

void serialSendByte(uint8_t id, uint8_t data)
{
    if (!serialTxEnabled[id]) return;

    uint16_t txd = data | 0x300;   // transmitter "shift register"

    // Start transmitter. This has to be done with interrupts disabled
    DISABLE_INTS();
    TBCCTL1 = 0;                    // transmit start bit
    if (id == 0) {
        pinClear(UART0_PORT, UART0_TX_PIN);
    } else {
        pinClear(UART1_PORT, UART1_TX_PIN);
    }
    TBCCR1 = TBR + UART_BITTIME;    // set time till the first data bit

    //XXX: the stability is MUCH worse if ints are enabled at this point
    //  ENABLE_INTS();

    // wait until the end of start bit
    while (0 == (TBCCTL1 & CCIFG));
    TBCCR1 += UART_BITTIME;

    while (txd) {
        if (txd & 1) {
            // transmit "Mark" (1) using OUTMOD=1 (Set)
            TBCCTL1 = OUTMOD_1;
            if (id == 0) {
                pinSet(UART0_PORT, UART0_TX_PIN);
            } else {
                pinSet(UART1_PORT, UART1_TX_PIN);
            }
        } else {
            // transmit "Space" (0) using OUTMOD=5 (Reset)
            TBCCTL1 = OUTMOD_5;
            if (id == 0) {
                pinClear(UART0_PORT, UART0_TX_PIN);
            } else {
                pinClear(UART1_PORT, UART1_TX_PIN);
            }
        }

        // wait for TA compare event
        while (0 == (TBCCTL1 & CCIFG));
        // set next bit time
        TBCCR1 += UART_BITTIME;

        txd >>= 1;
    }

    // all bits sent out; set TxD idle to "Mark"
    TBCCTL1 = OUT;
    if (id == 0) {
        pinSet(UART0_PORT, UART0_TX_PIN);
    } else {
        pinSet(UART1_PORT, UART1_TX_PIN);
    }

    ENABLE_INTS();
}

static void rxByte0(void)
{
    uint8_t rxByte, bitnum;
    bool rxOk;

    TBCCTL0 = OUT;
    TBCCR0 += UART_HALF_BITTIME - 100;
    while (0 == (TBCCTL0 & CCIFG));

    rxByte = 0;
    for (bitnum = 0; bitnum < 8; ++bitnum) {
        // wait for next data bit
        TBCCTL0 = OUT;
        TBCCR0 += UART_BITTIME;
        while (0 == (TBCCTL0 & CCIFG));

        rxByte |= pinRead(UART0_PORT, UART0_RX_PIN) << bitnum;
    }

    // wait for the stop bit
    TBCCTL0 = OUT;
    TBCCR0 += UART_BITTIME;
    while (0 == (TBCCTL0 & CCIFG));
    // ok if stop bit is 1
    rxOk = pinRead(UART0_PORT, UART0_RX_PIN);

    if (rxOk) serialRecvCb[0](rxByte);

    TBCCTL0 = CCIE;
}

static void rxByte1(void)
{
    uint8_t rxByte, bitnum;
    bool rxOk;

    TBCCTL0 = OUT;
    TBCCR0 += UART_HALF_BITTIME - 100;
    while (0 == (TBCCTL0 & CCIFG));

    rxByte = 0;
    for (bitnum = 0; bitnum < 8; ++bitnum) {
        // wait for next data bit
        TBCCTL0 = OUT;
        TBCCR0 += UART_BITTIME;
        while (0 == (TBCCTL0 & CCIFG));

        rxByte |= pinRead(UART1_PORT, UART1_RX_PIN) << bitnum;
    }

    // wait for the stop bit
    TBCCTL0 = OUT;
    TBCCR0 += UART_BITTIME;
    while (0 == (TBCCTL0 & CCIFG));
    // ok if stop bit is 1
    rxOk = pinRead(UART1_PORT, UART1_RX_PIN);

    if (rxOk) serialRecvCb[1](rxByte);

    TBCCTL0 = CCIE;
}

ISR(TIMERB0, serialRxTimerInterrupt)
{
    // start of reception
    if (serialRxEnabled[0]
            && serialRecvCb[0]
            && pinRead(UART0_PORT, UART0_RX_PIN) == 0) {
        // start bit detected on UART0!
        rxByte0();
    }
    else if (serialRxEnabled[1]
            && serialRecvCb[1]
            && pinRead(UART1_PORT, UART1_RX_PIN) == 0) {
        // start bit detected on UART1!
        rxByte1();
    }

    // heuristic value...
    TBCCR0 = TBR + UART_HALF_BITTIME;
}
