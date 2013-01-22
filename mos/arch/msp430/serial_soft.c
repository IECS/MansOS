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

//#define UART_BAUD    9600
#if PLATFORM_LAUNCHPAD
#define TIMERA_CLOCK 250000ul
#define SERIAL_BITTIME (TIMERA_CLOCK / SERIAL_BAUDRATE)
#else
//#define SERIAL_BITTIME (ACLK_SPEED / SERIAL_BAUDRATE)
#define TIMERA_CLOCK 500000ul
#define SERIAL_BITTIME (TIMERA_CLOCK / SERIAL_BAUDRATE)
#endif

// #define UART_TXD    BIT1      // P1.1 (Timer_A.OUT0) used as UART TXD
// #define 

// launchpad
//#define UART_PORT    1
//#define UART_RX_PIN  1
//#define UART_TX_PIN  2
//#define UART_TXD     BIT1

// telosb
#define UART_PORT    1
#define UART_TX_PIN  1
#define UART_RX_PIN  2
// #define UART_TX_PIN  2
// #define UART_RX_PIN  1
//#define UART_TXD     BIT1

// ---------------------------------------------

SerialCallback_t serialRecvCb[SERIAL_COUNT];
volatile Serial_t serial[SERIAL_COUNT];

// ---------------------------------------------

uint_t serialInit(uint8_t id, uint32_t speed, uint8_t conf)
{
    // if (id != 0) return -ENOSYS;
    // if (speed != 9600) return -ENOSYS;

    U0ME &= ~UTXE0;
    U1ME &= ~UTXE1;
    U0ME &= ~URXE0;
    U1ME &= ~URXE1;
    IE1 &= ~URXIE0;
    IE2 &= ~URXIE1;

#if PLATFORM_LAUNCHPAD
    /* Initialize system clock: DCO=1MHz MCLK=/1, SMCLK=/1 */
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    BCSCTL2 = 0;
#endif

    /* Configure IO pin(s) */
//    pinAsFunction(UART_PORT, UART_RX_PIN);
//    pinAsInput(UART_PORT, UART_RX_PIN);

//    pinAsFunction(UART_PORT, UART_TX_PIN);
//    pinAsOutput(UART_PORT, UART_TX_PIN);

    P1DIR |= 0x02;                            // P1.1 output
    P1SEL |= 0x02;                            // P1.1 option select

//    P1SEL |= UART_TXD; // assign TXD pin to Timer_A
//     P1DIR |= UART_TXD; // set TxD as output

     /* Configure Timer_A */
//#if PLATFORM_LAUNCHPAD
    TACTL = TACLR;
//    TACTL = TASSEL_2 + MC_2 + ID_2;  // set TimerA clock SMCLK/4 (250 kHz)
//    TACTL = TASSEL_2 + MC_1 + ID_2;  // set TimerA clock SMCLK/4 (250 kHz)
    TACTL = TASSEL_2 + MC_2 + ID_3;  // set TimerA clock SMCLK/4 (500 kHz)
//#endif

    /* Initialize UART */
    TACCTL0 = OUT;      // TxD idle as '1'

    return 0;
}

void serialSendByte(uint8_t id, uint8_t data)
{
    // pinAsFunction(UART_PORT, UART_TX_PIN);
    // pinAsOutput(UART_PORT, UART_TX_PIN);

    // XXX
    //  TACTL = TASSEL_2 + MC_2 + ID_3;  // set TimerA clock SMCLK/8 (500 KHz)

    uint16_t txd = (uint16_t) data | 0x300;   // transmitter "shift register"

    /* Start transmitter. This has to be done with interrupts disabled */
    DISABLE_INTS();
    TACCTL0 = 0;                    // transmit start bit
//    TAR = 0;
    TACCR0 = TAR + SERIAL_BITTIME;    // set time till the first data bit
//    TACCR0 = SERIAL_BITTIME;
    ENABLE_INTS();

    while (txd) {
        if (txd & 1) {
            /* transmit "Mark" (1) using OUTMOD=1 (Set) */
            TACCTL0 = OUTMOD_1;
        } else {
            /* transmit "Space" (0) using OUTMOD=5 (Reset) */
            TACCTL0 = OUTMOD_5;
        }

        /* wait for TA compare event */
        while (0 == (TACCTL0 & CCIFG));
//        TACCTL0 &= ~CCIFG;

        /* set next bit time */
        TACCR0 += SERIAL_BITTIME;

        txd >>= 1;
    }

    /* all bits sent out; set TxD idle to "Mark" */
    TACCTL0 = OUT;


    mdelay(10);

    // TAR = 0;
    // TACCR0 = PLATFORM_ALARM_TIMER_PERIOD;

//    ENABLE_INTS();
}
