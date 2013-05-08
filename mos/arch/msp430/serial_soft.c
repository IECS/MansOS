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
#if USE_THREADS
#include <kernel/threads/threads.h>
#endif

// only this baudrate is supported
#define SOFT_SERIAL_BAUDRATE 9600

#define TIMER_CLOCK (CPU_HZ)
#define UART_BITTIME ((TIMER_CLOCK / SOFT_SERIAL_BAUDRATE))
#define UART_HALF_BITTIME   UART_BITTIME / 2


#ifndef UART0_HW_TX_PORT
#define UART0_HW_TX_PORT UART0_TX_PORT
#define UART0_HW_TX_PIN  UART0_TX_PIN
#define UART0_HW_RX_PORT UART0_RX_PORT
#define UART0_HW_RX_PIN  UART0_RX_PIN
#endif

// ---------------------------------------------

SerialCallback_t serialRecvCb[SERIAL_COUNT];
volatile Serial_t serial[SERIAL_COUNT];

volatile bool serialRxEnabled[SERIAL_COUNT];
volatile bool serialTxEnabled[SERIAL_COUNT];

static void rxByte0(void) UNUSED;
static void rxByte1(void) UNUSED;

static void softSerialStop(void) UNUSED;
static void softSerialRestart(void) UNUSED;

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

static inline void softSerialTimerStart(void) {
    // wait for timer to stop in case it is used
    // while (TBCTL & MC_3);
    // start the timer
    TBCTL = TBSSEL_2 + MC_2; // clock = SMCLK
}

static inline void softSerialTimerStop(void) {
    TBCTL &= ~MC_3;
}

static void softSerialStop(void)
{
    // disable serial, so that we wont even try to send anything,
    // if woken in an intrrupt
    serialTxEnabled[0] = false;
#if SERIAL_COUNT > 1
    serialTxEnabled[1] = false;
#endif
    // disable serial rx interrupt
#if USE_SW_SERIAL_INTERRUPTS
    pinDisableInt(UART0_RX_PORT, UART0_RX_PIN);
#endif
    // stop timer
    TBCTL &= ~MC_3;
}

static void softSerialRestart(void)
{
    // XXX: this is not correct if the user wants it disabled fro some reason
    // enable back serial
    serialTxEnabled[0] = true;
#if SERIAL_COUNT > 1
    serialTxEnabled[1] = true;
#endif
    // enable back serial rx interrupt
#if USE_SW_SERIAL_INTERRUPTS
    pinEnableInt(UART0_RX_PORT, UART0_RX_PIN);
#endif
    // start timer
    TBCTL |= MC_2;
}

uint_t serialInit(uint8_t id, uint32_t speed, uint8_t conf)
{
    if (conf != 0) return -ENOSYS;
    // support only one baudrate (XXX: panic on error?)
    if (speed != SOFT_SERIAL_BAUDRATE) return -ENOSYS;

    if (id == 0) {
        pinAsData(UART0_TX_PORT, UART0_TX_PIN);
        pinAsOutput(UART0_TX_PORT, UART0_TX_PIN);

        pinAsData(UART0_RX_PORT, UART0_RX_PIN);
        pinAsInput(UART0_RX_PORT, UART0_RX_PIN);

#if UART0_HW_TX_PORT != UART0_TX_PORT
        // -- also setup HW serial ports in the same mode
        pinAsData(UART0_HW_TX_PORT, UART0_HW_TX_PIN);
        pinAsInput(UART0_HW_TX_PORT, UART0_HW_TX_PIN);


        pinAsData(UART0_HW_RX_PORT, UART0_HW_RX_PIN);
        pinAsInput(UART0_HW_RX_PORT, UART0_HW_RX_PIN);
#endif

#if USE_SW_SERIAL_INTERRUPTS
        // PRINTF("serial int init\n");
        pinEnableInt(UART0_RX_PORT, UART0_RX_PIN);
        pinIntFalling(UART0_RX_PORT, UART0_RX_PIN);
        pinClearIntFlag(UART0_RX_PORT, UART0_RX_PIN);
#endif
    }
    else {
        pinAsData(UART1_TX_PORT, UART1_TX_PIN);
        pinAsOutput(UART1_TX_PORT, UART1_TX_PIN);

        pinAsData(UART1_RX_PORT, UART1_RX_PIN);
        pinAsInput(UART1_RX_PORT, UART1_RX_PIN);
    }

    // Configure Timer_B
    TBCTL = TBCLR;
//    TBCTL = TBSSEL_2 + MC_2; // clock = SMCLK

    // Initialize UART
    if (id == 0) {
        pinSet(UART0_TX_PORT, UART0_TX_PIN);
//        pinSet(UART0_HW_TX_PORT, UART0_HW_TX_PIN);
    } else {
        pinSet(UART1_TX_PORT, UART1_TX_PIN);
    }

    TBCCTL0 = 0;
    TBCCTL1 = 0;
    TBCCTL2 = 0;
#ifdef TBCCTL3_
    TBCCTL3 = 0;
    TBCCTL4 = 0;
    TBCCTL5 = 0;
    TBCCTL6 = 0;
#endif

//    TACTL = TACLR;

// #ifndef USE_SW_SERIAL_INTERRUPTS
//     TBCCR2 = TBR + 100;
//     TBCCTL2 = CCIE;
// #endif

    return 0;
}

void serialSendByte(uint8_t id, uint8_t data)
{
    if (!serialTxEnabled[id]) return;

    uint16_t txd = data | 0x300;   // transmitter "shift register"

    // Start transmitter. This has to be done with interrupts disabled
    Handle_t h;
    ATOMIC_START(h);

    softSerialTimerStart();

    TBCCTL1 = 0;                    // transmit start bit
    if (id == 0) {
        pinClear(UART0_TX_PORT, UART0_TX_PIN);
//        pinClear(UART0_HW_TX_PORT, UART0_HW_TX_PIN);
    } else {
        pinClear(UART1_TX_PORT, UART1_TX_PIN);
    }
    TBCCR1 = TBR + UART_BITTIME;    // set time till the first data bit

    //XXX: the stability is MUCH worse if ints are enabled at this point
    // ATOMIC_END(h);

    // wait until the end of start bit
    while (0 == (TBCCTL1 & CCIFG));
    TBCCR1 += UART_BITTIME;

    while (txd) {
        if (txd & 1) {
            // transmit "Mark" (1) using OUTMOD=1 (Set)
            TBCCTL1 = OUTMOD_1;
            if (id == 0) {
                pinSet(UART0_TX_PORT, UART0_TX_PIN);
//                pinSet(UART0_HW_TX_PORT, UART0_HW_TX_PIN);
            } else {
                pinSet(UART1_TX_PORT, UART1_TX_PIN);
            }
        } else {
            // transmit "Space" (0) using OUTMOD=5 (Reset)
            TBCCTL1 = OUTMOD_5;
            if (id == 0) {
                pinClear(UART0_TX_PORT, UART0_TX_PIN);
//                pinClear(UART0_HW_TX_PORT, UART0_HW_TX_PIN);
            } else {
                pinClear(UART1_TX_PORT, UART1_TX_PIN);
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
        pinSet(UART0_TX_PORT, UART0_TX_PIN);
//        pinSet(UART0_HW_TX_PORT, UART0_HW_TX_PIN);
    } else {
        pinSet(UART1_TX_PORT, UART1_TX_PIN);
    }

    softSerialTimerStop();

    ATOMIC_END(h);
}

static void rxByte0(void)
{
    uint8_t rxByte, bitnum;
    bool rxOk;
    uint16_t i;

#if USE_SW_SERIAL_INTERRUPTS
    udelay(75);
#endif

  restart:
    rxByte = 0;
    TBCCR2 = TBR;
    for (bitnum = 0; bitnum < 8; ++bitnum) {
        // wait for next data bit
        TBCCTL2 = OUT;
        TBCCR2 += UART_BITTIME;
        while (0 == (TBCCTL2 & CCIFG));

        rxByte |= pinRead(UART0_RX_PORT, UART0_RX_PIN) << bitnum;
    }

    // wait for the stop bit
    TBCCTL2 = OUT;
    TBCCR2 += UART_BITTIME;
    while (0 == (TBCCTL2 & CCIFG));
    // ok if stop bit is 1
    rxOk = pinRead(UART0_RX_PORT, UART0_RX_PIN);

    if (rxOk) serialRecvCb[0](rxByte);
//    else redLedToggle();

    // receive next byte immediately, do not wait for the next int
    for (i = 0; i < 1000; ++i) {
        if (!pinRead(UART0_RX_PORT, UART0_RX_PIN)) {
            udelay(50);
            goto restart;
        }
    }
}

static void rxByte1(void)
{
    uint8_t rxByte, bitnum;
    bool rxOk;
    uint16_t i;

#if USE_SW_SERIAL_INTERRUPTS
    udelay(75);
#endif

  restart:
    rxByte = 0;
    TBCCR2 = TBR;
    for (bitnum = 0; bitnum < 8; ++bitnum) {
        // wait for next data bit
        TBCCTL2 = OUT;
        TBCCR2 += UART_BITTIME;
        while (0 == (TBCCTL2 & CCIFG));

        rxByte |= pinRead(UART1_RX_PORT, UART1_RX_PIN) << bitnum;
    }

    // wait for the stop bit
    TBCCTL2 = OUT;
    TBCCR2 += UART_BITTIME;
    while (0 == (TBCCTL2 & CCIFG));
    // ok if stop bit is 1
    rxOk = pinRead(UART1_RX_PORT, UART1_RX_PIN);

    if (rxOk) serialRecvCb[1](rxByte);
//    else redLedToggle();

    // receive next byte immediately, do not wait for the next int
    for (i = 0; i < 1000; ++i) {
        if (!pinRead(UART0_RX_PORT, UART0_RX_PIN)) {
            udelay(50);
            goto restart;
        }
    }
}

volatile bool appRunning;

#ifdef USE_SW_SERIAL_INTERRUPTS

// on msp430 interrupt vectors are present only for port1 and port2
#if UART0_RX_PORT <= 2 

XISR(UART0_RX_PORT, serialRxInterrupt)
{
    if (!pinReadIntFlag(UART0_RX_PORT, UART0_RX_PIN)) return;

    pinClearIntFlag(UART0_RX_PORT, UART0_RX_PIN);

    if (serialRxEnabled[0]
            && serialRecvCb[0]) {

        softSerialTimerStart();

        pinDisableInt(UART0_RX_PORT, UART0_RX_PIN);

        // read the whole byte
        rxByte0();

        pinEnableInt(UART0_RX_PORT, UART0_RX_PIN);
        pinClearIntFlag(UART0_RX_PORT, UART0_RX_PIN);

        softSerialTimerStop();

#if RADIO_ON_UART0 && USE_THREADS
        // wake up the kernel thread in case radio packet is received
        if (processFlags.value) {
            EXIT_SLEEP_MODE();
        }
#endif
    }
}

#endif // UART0_RX_PORT <= 2 

// TODO: UART1 ?

#else

// use timer interrupts

ISR(TIMERB1, serialRxTimerInterrupt)
{
    // start of reception
    if (serialRxEnabled[0]
            && serialRecvCb[0]
            && pinRead(UART0_RX_PORT, UART0_RX_PIN) == 0) {
        // start bit detected on UART0!

        TBCCTL2 = OUT;
        TBCCR2 += UART_HALF_BITTIME - 100;
        while (0 == (TBCCTL2 & CCIFG));

        rxByte0();

#if RADIO_ON_UART0 && USE_THREADS
        // wake up the kernel thread in case radio packet is received
        if (processFlags.value) {
            EXIT_SLEEP_MODE();
        }
#endif
    }
    else if (serialRxEnabled[1]
            && serialRecvCb[1]
            && pinRead(UART1_RX_PORT, UART1_RX_PIN) == 0) {
        // start bit detected on UART1!

        TBCCTL2 = OUT;
        TBCCR2 += UART_HALF_BITTIME - 100;
        while (0 == (TBCCTL2 & CCIFG));

        rxByte1();

#if RADIO_ON_UART1 && USE_THREADS
        // wake up the kernel thread in case radio packet is received
        if (processFlags.value) {
            EXIT_SLEEP_MODE();
        }
#endif
    }

    // heuristic value...
    TBCCR2 = TBR + UART_HALF_BITTIME;
    TBCCTL2 = CCIE;
}

#endif
