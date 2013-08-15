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
// msp430x16x USART module
//

#define USE_SERIAL_RX 1 // XXX

#include <digital.h>
#include <serial.h>
#include <spi.h>
#include <errors.h>
#include <hil/busywait.h>
#include <defines.h>
#include "msp430_timers.h"
#include <kernel/stack.h>

#include <lib/dprint.h>

#if USE_THREADS
#include <kernel/threads/threads.h>
#endif

//===========================================================
// Data types and constants
//===========================================================

SerialCallback_t serialRecvCb[SERIAL_COUNT];

//===========================================================
// Procedures
//===========================================================

// Setting port directions & selections for RX/TX
static void serialInitPins0(void) {
    // serial0
    pinAsOutput(UTXD0_PORT, UTXD0_PIN);
    pinAsInput(URXD0_PORT, URXD0_PIN);
    pinClear(UTXD0_PORT, UTXD0_PIN);
    pinAsFunction(UTXD0_PORT, UTXD0_PIN);
    pinAsFunction(URXD0_PORT, URXD0_PIN);
}

static void serialInitPins1(void) {
    // serial 1
    pinAsOutput(UTXD1_PORT, UTXD1_PIN);
    pinAsInput(URXD1_PORT, URXD1_PIN);
    pinClear(UTXD1_PORT, UTXD1_PIN);
    pinAsFunction(UTXD1_PORT, UTXD1_PIN);
    pinAsFunction(URXD1_PORT, URXD1_PIN);
}

static void serialInitSpeed(uint32_t speed,
                            volatile uint8_t *restrict clock,
                            volatile uint8_t *restrict frequency,
                            volatile uint8_t *restrict correction) {
    
    // The values were calculated using baudrate calculator:
    // http://mspgcc.sourceforge.net/baudrate.html
    // basically: BR0 ~= CLOCK_FREQUENCY / BAUDRATE, and MCTL is correction value
    switch (speed) {

    // Added by Girts 2012-05-18 (User's guide page 265)
    case 2400:
        *clock |= SSEL_ACLK; // use ACLK
        *frequency = 0xd;
        *correction = 0x6b;
        break; 

    case 4800:
        *clock |= SSEL_ACLK; // use ACLK
        *frequency = 0x6;
        *correction = 0x77;
        break;

    default:
        // TODO: Tell somehow, that no valid speed is selected/found,
        // but use 9600 for now
    case 9600:
        *clock |= SSEL_ACLK; // use ACLK, assuming 32khz
#if ACLK_SPEED == 32768
        // assume 32'768 Hz crystal
        *frequency = 0x3;
        *correction = 0x29;
#else
        // assume 32'000 Hz crystal
        *frequency = 0x3;
        *correction = 0x4C;
#endif
        break;

    case 38400:
        *clock |= SSEL_SMCLK;    // use SMCLK
        switch (CPU_MHZ) {
        case 1:
            *frequency = 0x1B;
            *correction = 0x94;
            break;
        case 2:
            *frequency = 0x36;
            *correction = 0xB5;
            break;
        case 4:
        default:
            *frequency = 0x6D;
            *correction = 0x44;
            break;
        }
        break;
    case 115200:
        *clock |= SSEL_SMCLK;    // use SMCLK
        switch (CPU_MHZ) {
        case 1:
            *frequency = 0x09;
            *correction = 0x10;
            break;
        case 2:
            *frequency = 0x12;
            *correction = 0x64;
            break;
        case 4:
        default:
            *frequency = 0x24;
            *correction = 0x29;
            break;
        }
        break;
    }
}

// Init serial
void  msp430UsartSerialInit0(uint32_t speed)
{
    serialInitPins0();

    // Set SWRST - Software reset must be set when configuring
    U0CTL = SWRST;

    //Initialize all serial registers
    //  UxCTL, serial Control Register
    U0CTL |= CHAR;  // 8-bit char, UART-mode

    //  UxTCTL, serial Transmit Control Register
    //  SSELx Bits Source select. These bits select the BRCLK source clock.
    //  Clear all bits for initial setup
    U0TCTL &= ~(SSEL_0 | SSEL_1 | SSEL_2 | SSEL_3);

    serialInitSpeed(speed, &(U0TCTL), &(U0BR0), &(U0MCTL));
    U0BR1 = 0x00; // zero on all supported speeds

    // Enable serial module via the MEx SFRs (URXEx and/or UTXEx)
    // and disable interrupts
    IE1 &= ~(UTXIE0 | URXIE0);
    U0ME &= ~(USPIE0 | URXE0);
    U0ME |= UTXE0;

    //Clear SWRST via software - Release software reset
    U0CTL &= ~(SWRST);
}

void msp430UsartSerialInit1(uint32_t speed)
{
    serialInitPins1();

    // Set SWRST - Software reset must be set when configuring
    U1CTL = SWRST;

    //Initialize all serial registers
    //  UxCTL, serial Control Register
    U1CTL |= CHAR;  // 8-bit char, UART-mode

    //  UxTCTL, serial Transmit Control Register
    //  SSELx Bits Source select. These bits select the BRCLK source clock.
    //  Clear all bits for initial setup
    U1TCTL &= ~(SSEL_0 | SSEL_1 | SSEL_2 | SSEL_3);

    serialInitSpeed(speed, &(U1TCTL), &(U1BR0), &(U1MCTL));
    U1BR1 = 0x00; // zero on all supported speeds

    // Enable serial module via the MEx SFRs (URXEx and/or UTXEx)
    // and disable interrupts
    IE2 &= ~(UTXIE1 | URXIE1);
    U1ME &= ~(USPIE1 | URXE1);
    U1ME |= UTXE1;

    //Clear SWRST via software - Release software reset
    U1CTL &= ~(SWRST);
}


//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t msp430SerialInitI2C(uint8_t id) {
    // I2C available on USART0 only
    if (id != 0) return -ENOSYS;

    U0CTL = SWRST; // reset must be hold while configuring
    /* 8-bit transfer, I2C mode, master, 7-bit addressing */
    U0CTL |= CHAR | SYNC | MST | I2C;
    I2CTCTL = I2CSSEL_2; /* SMCLK, byte mode */

    /* Select Peripheral functionality */
    pinAsFunction(HW_SCL_PORT, HW_SCL_PIN);
    pinAsFunction(HW_SDA_PORT, HW_SDA_PIN);

    /* Configure as outputs(SIMO,CLK). */
    pinAsOutput(HW_SCL_PORT, HW_SCL_PIN);
    pinAsOutput(HW_SCL_PORT, HW_SDA_PIN);

    IE1 &= ~(UTXIE0 | URXIE0);      // interrupt disabled

    U0ME |= I2CEN;                   /* Enable I2C module */
    U0ME |= UTXE0 | URXE0;           /* Enable UART module (DO NOT REMOVE!) */
    U0CTL &= ~SWRST;                 /* Remove RESET flag */

    return 0;
}

//-----------------------------------------------------------
//-----------------------------------------------------------

#if USE_SERIAL_RX

ISR(UART0RX, UART0InterruptHandler)
{
    if (URCTL0 & RXERR) {
        volatile unsigned dummy;
        dummy = RXBUF0;   /* Clear error flags by forcing a dummy read. */
        return;
    }

    uint8_t x = U0RXBUF;

    // PRINTF("serial 0 char %#02x\n", (uint16_t) x);

    if (serialRecvCb[0]) serialRecvCb[0](x);

    // in case radio chip uses UART0 and threads are used (e.g. on SM3)...
#if RADIO_ON_UART0 && USE_THREADS
    // wake up the kernel thread in case radio packet is received
    if (processFlags.bits.radioProcess) {
        // TPRINTF("wake up kernel, packet start=%lu\n", packetStart);
        EXIT_SLEEP_MODE();
    }
#endif
}

ISR(UART1RX, UART1InterruptHandler)
{
    if (URCTL1 & RXERR) {
        volatile unsigned dummy;
        dummy = RXBUF1;   /* Clear error flags by forcing a dummy read. */
        return;
    }

    uint8_t x = U1RXBUF;

    // PRINTF("serial 1 char %#02x\n", (uint16_t) x);

    if (serialRecvCb[1]) serialRecvCb[1](x);

#if RADIO_ON_UART1 && USE_THREADS
    // wake up the kernel thread in case radio packet is received
    if (processFlags.bits.radioProcess) {
        EXIT_SLEEP_MODE();
    }
#endif
}

#endif // USE_SERIAL_RX
