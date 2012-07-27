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
// msp430x16x USART
//

#define USE_USART_RX 1 // XXX

#include <hil/gpio.h>
#include <hil/usart.h>
#include <hil/spi.h>
#include <hil/busywait.h>
#include <kernel/defines.h>
#include "msp430_timers.h"
#include <kernel/threads/threads.h>

#include <lib/dprint.h>

//===========================================================
// Data types and constants
//===========================================================

enum Msp430USARTMode_e {
    UM_DISABLED,
    UM_UART,
    UM_SPI_MASTER,
    UM_SPI_SLAVE,
    UM_I2C
} PACKED;

typedef enum Msp430USARTMode_e Msp430USARTMode_t;

USARTCallback_t usartRecvCb[USART_COUNT];

bool usartBusy[USART_COUNT];

#define NOT_IMPLEMENTED -1

//===========================================================
// Procedures
//===========================================================

//-----------------------------------------------------------
//-----------------------------------------------------------

// pointers to USARTx registers:
// TODO - change these to base + offset (base = UART0 reg, offset = 8 * id)
static volatile uint8_t * const UxCTL[USART_COUNT] = { &(U0CTL), &(U1CTL) };
static volatile uint8_t * const UxTCTL[USART_COUNT] = { &(U0TCTL), &(U1TCTL) };
static volatile uint8_t * const UxMCTL[USART_COUNT] = { &(U0MCTL), &(U1MCTL) };
static volatile uint8_t * const UxBR0[USART_COUNT] = { &(U0BR0), &(U1BR0) };
static volatile uint8_t * const UxBR1[USART_COUNT] = { &(U0BR1), &(U1BR1) };
static volatile uint8_t * const UxME[USART_COUNT] = { &(U0ME), &(U1ME) };

// buffers are not static, as they are declared in .h file
volatile uint8_t * const UxTXBUF[USART_COUNT] = { &(U0TXBUF), &(U1TXBUF) };
volatile const uint8_t * const UxRXBUF[USART_COUNT] = { &(U0RXBUF), &(U1RXBUF) };

static volatile uint8_t * const IEx[USART_COUNT] = { &(IE1), &(IE2) };
static volatile uint8_t * const IFGx[USART_COUNT] = { &(IFG1), &(IFG2) };

// USART flags
static const uint8_t USPIEx[USART_COUNT] = { USPIE0, USPIE1 };
static const uint8_t UTXEx[USART_COUNT] = { UTXE0, UTXE1 };
static const uint8_t URXEx[USART_COUNT] = { URXE0, URXE1 };
static const uint8_t URXIEx[USART_COUNT] = { URXIE0, URXIE1 };
static const uint8_t UTXIEx[USART_COUNT] = { UTXIE0, UTXIE1 };
static const uint8_t UTXIFGx[USART_COUNT] = { UTXIFG0, UTXIFG1 };
static const uint8_t URXIFGx[USART_COUNT] = { URXIFG0, URXIFG1 };

// actual USARTx mode (UART/SPI/I2C)
static Msp430USARTMode_t usartMode[USART_COUNT] = { UM_DISABLED, UM_DISABLED };


static inline void USARTInitPins(uint8_t id) {
    // Setting port directions & selections for RX/TX
    // cannot use params in macros
    if (id == 0) {
        // USART0
        pinAsOutput(UTXD0_PORT, UTXD0_PIN);
        pinAsInput(URXD0_PORT, URXD0_PIN);
        pinClear(UTXD0_PORT, UTXD0_PIN);
        pinAsFunction(UTXD0_PORT, UTXD0_PIN);
        pinAsFunction(URXD0_PORT, URXD0_PIN);
    } else if (id == 1) {
        // USART 1
        pinAsOutput(UTXD1_PORT, UTXD1_PIN);
        pinAsInput(URXD1_PORT, URXD1_PIN);
        pinClear(UTXD1_PORT, UTXD1_PIN);
        pinAsFunction(UTXD1_PORT, UTXD1_PIN);
        pinAsFunction(URXD1_PORT, URXD1_PIN);
    }
}

static inline void USARTInitSpeed(uint8_t id, uint32_t speed) {
    uint8_t clock;
    uint8_t frequency;
    uint8_t correction;

    // The values were calculated using baudrate calculator:
    // http://mspgcc.sourceforge.net/baudrate.html
    // basically: BR0 ~= CLOCK_FREQUENCY / BAUDRATE, and MCTL is correction value
    switch(speed) {

    // Added by Girts 2012-05-18 (User's guide page 265)
    case 2400:
        clock = SSEL_ACLK; // use ACLK
        frequency = 0xd;
        correction = 0x6b;
        break; 

    case 4800:
        clock = SSEL_ACLK; // use ACLK
        frequency = 0x6;
        correction = 0x77;
        break;

    default:
        // TODO: Tell somehow, that no valid speed is selected/found,
        // but use 9600 for now
    case 9600:
        clock = SSEL_ACLK; // use ACLK, assuming 32khz
#if ACLK_SPEED == 32768
        // assume 32'768 Hz crystal
        frequency = 0x3;
        correction = 0x29;
#else
        // assume 32'000 Hz crystal
        frequency = 0x3;
        correction = 0x4C;
#endif
        break;

    case 38400:
        clock = SSEL_SMCLK;    // use SMCLK
        switch (CPU_MHZ) {
        case 1:
            frequency = 0x1B;
            correction = 0x94;
            break;
        case 2:
            frequency = 0x36;
            correction = 0xB5;
            break;
        case 4:
        default:
            frequency = 0x6D;
            correction = 0x44;
            break;
        }
        break;
    case 115200:
        clock = SSEL_SMCLK;    // use SMCLK
        switch (CPU_MHZ) {
        case 1:
            frequency = 0x09;
            correction = 0x10;
            break;
        case 2:
            frequency = 0x12;
            correction = 0x64;
            break;
        case 4:
        default:
            frequency = 0x24;
            correction = 0x29;
            break;
        }
        break;
    }

    *UxTCTL[id] |= clock;
    *UxBR0[id] = frequency;
    *UxBR1[id] = 0x00;
    *UxMCTL[id] = correction;
}

// Init USART1
uint_t USARTInit(uint8_t id, uint32_t speed, uint8_t conf)
{
    // USARTx not supported, x >= USART_COUNT
    if (id >= USART_COUNT) return NOT_IMPLEMENTED;

    // we could want to change UART speed - allow to call this
    // function multiple times
//    if (usartMode[id] == UM_UART) return 0; // already in UART mode

    USARTInitPins(id);

    // Set SWRST - Software reset must be set when configuring
    *UxCTL[id] = SWRST;

    //Initialize all USART registers
    //  UxCTL, USART Control Register
    *UxCTL[id] |= CHAR;  // 8-bit char, UART-mode

    //  UxTCTL, USART Transmit Control Register
    //  SSELx Bits Source select. These bits select the BRCLK source clock.
    //  Clear all bits for initial setup
    *UxTCTL[id] &= ~(SSEL_0 | SSEL_1 | SSEL_2 | SSEL_3);

    USARTInitSpeed(id, speed);

    // Enable USART module via the MEx SFRs (URXEx and/or UTXEx)
    *UxME[id] &= ~USPIEx[id];             // USARTx SPI module disable
    *UxME[id] |= UTXEx[id] | URXEx[id];   // USARTx UART module enable

    //Clear SWRST via software - Release software reset
    *UxCTL[id] &= ~(SWRST);

    //Disable interrupts via the IEx SFRs (URXIEx and/or UTXIEx)
    //  IFGx, Interrupt Flag Register 2
    *IFGx[id] &= ~(UTXIFGx[id] | URXIFGx[id]);   // ???
    *IEx[id] &= ~(UTXIEx[id] | URXIEx[id]);      // interrupt disabled

    usartMode[id] = UM_UART;

    return 0;
}


//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t msp430USARTInitSPI(uint8_t id, uint_t spiBusMode) {
    // USARTx not supported, x >= USART_COUNT
    if (id >= USART_COUNT) return NOT_IMPLEMENTED;

    // TODO - implement slave mode

    if ((usartMode[id] == UM_SPI_MASTER && spiBusMode == SPI_MODE_MASTER)
            || (usartMode[id] == UM_SPI_SLAVE && spiBusMode == SPI_MODE_SLAVE)) {
        return 0; // already in appropriate SPI mode
    }

    *UxCTL[id] = SWRST; // reset must be hold while configuring
    *UxCTL[id] |= CHAR | SYNC | MST; /* 8-bit transfer, SPI mode, master */
    // CKPH = 1 actually corresponds to CPHA = 0
    // (MSP430 User's guide, Figure 14-9)
    *UxTCTL[id] = CKPH | SSEL1 | STC; /* Data on Rising Edge, SMCLK, 3-wire. */

    // SPI CLK = SMCLK / 2
    *UxBR0[id]  = 0x02;  /* SPICLK set baud. */
    *UxBR1[id]  = 0;     /* Dont need baud rate control register 2 - clear it */
    *UxMCTL[id] = 0;     /* Dont need modulation control. */

    /* Select Peripheral functionality */
    pinAsFunction(HW_SCK_PORT, HW_SCK_PIN);
    pinAsFunction(HW_MOSI_PORT, HW_MOSI_PIN);
    pinAsFunction(HW_MISO_PORT, HW_MISO_PIN);

    /* Configure as outputs(SIMO,CLK). */
    pinAsOutput(HW_SCK_PORT, HW_SCK_PIN);
    pinAsOutput(HW_MOSI_PORT, HW_MOSI_PIN);
    pinAsInput(HW_MISO_PORT, HW_MISO_PIN);

    *UxME[id] |= USPIE0;                   /* Enable SPI module */
    /* Enable UART module (DO NOT REMOVE!) */
    if (id == 0) {
        *UxME[id] |= UTXE0 | URXE0;
    } else {
        *UxME[id] |= UTXE1 | URXE1;
    }
    *UxCTL[id] &= ~SWRST;                  /* Remove RESET flag */

    usartMode[id] = (spiBusMode == SPI_MODE_SLAVE ? UM_SPI_SLAVE : UM_SPI_MASTER);

    return 0;
}


//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t msp430USARTInitI2C(uint8_t id) {
    // // I2C available on USART0 only
    if (id != 0) return NOT_IMPLEMENTED;

    if (usartMode[id] == UM_I2C) return 0; // already in I2C mode

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

    U0ME |= I2CEN;                   /* Enable I2C module */
    U0ME |= UTXE0 | URXE0;            /* Enable UART module (DO NOT REMOVE!) */
    U0CTL &= ~SWRST;                  /* Remove RESET flag */

    usartMode[id] = UM_I2C;

    return 0;
}

//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTSendByte(uint8_t id, uint8_t data)
{
//    STACK_GUARD();
    // USARTx not supported, x >= USART_COUNT
    if (id >= USART_COUNT) return NOT_IMPLEMENTED;
    *UxTXBUF[id] = data;
    while (((*UxTCTL[id]) & (TXEPT)) == 0);  // Is byte sent ?
    return 0;

    // uint_t ok;
    // BUSYWAIT_UNTIL(*UxTCTL[id] & TXEPT, 10, ok);
    // return ok;
}




//-----------------------------------------------------------
//-----------------------------------------------------------
uint_t USARTEnableTX(uint8_t id) {
    // USARTx not supported, x >= USART_COUNT
    if (id >= USART_COUNT) return NOT_IMPLEMENTED;
    *UxME[id] |= UTXEx[id];   // Enable TX module
    return 0;
}

//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTDisableTX(uint8_t id) {
    // USARTx not supported, x >= USART_COUNT
    if (id >= USART_COUNT) return NOT_IMPLEMENTED;
    *UxME[id] &= ~UTXEx[id];   // Disable TX module
    return 0;
}

//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTEnableRX(uint8_t id) {
    // USARTx not supported, x >= USART_COUNT
    if (id >= USART_COUNT) return NOT_IMPLEMENTED;
    *UxME[id] |= URXEx[id];   // Enable RX module
    // Enable RX interrupt only in UART mode
    if (usartMode[id] == UM_UART) {
        *IEx[id] |= URXIEx[id];
    } else {
        return -1u;
    }
    return 0;
}

//-----------------------------------------------------------
//-----------------------------------------------------------

uint_t USARTDisableRX(uint8_t id) {
    // USARTx not supported, x >= USART_COUNT
    if (id >= USART_COUNT) return NOT_IMPLEMENTED;
    *UxME[id] &= ~URXEx[id];   // Disable RX module
    *IEx[id] &= ~URXIEx[id];   // Disable RX interrupt
    return 0;
}

//-----------------------------------------------------------
//-----------------------------------------------------------

//uint_t USARTisUART(uint8_t id) {
//    return (id < USART_COUNT && usartMode[id] == UM_UART) ? 1 : 0;
//};
//
//
////-----------------------------------------------------------
////-----------------------------------------------------------
//
//uint_t USARTisSPI(uint8_t id) {
//    return (id < USART_COUNT && usartMode[id] == UM_SPI) ? 1 : 0;
//};
//
//
////-----------------------------------------------------------
////-----------------------------------------------------------
//
//uint_t USARTisI2C(uint8_t id) {
//    return (id < USART_COUNT && usartMode[id] == UM_I2C) ? 1 : 0;
//};

//-----------------------------------------------------------
//-----------------------------------------------------------

#if USE_USART_RX

ISR(UART0RX, UART0InterruptHandler)
{
    if (URCTL0 & RXERR) {
        volatile unsigned dummy;
        dummy = RXBUF0;   /* Clear error flags by forcing a dummy read. */
        return;
    }

    uint8_t x = U0RXBUF;

    if (usartRecvCb[0]) usartRecvCb[0](x);
}

ISR(UART1RX, UART1InterruptHandler)
{
    if (URCTL1 & RXERR) {
        volatile unsigned dummy;
        dummy = RXBUF1;   /* Clear error flags by forcing a dummy read. */
        return;
    }

    uint8_t x = U1RXBUF;

    if (usartRecvCb[1]) usartRecvCb[1](x);
}

#endif // USE_USART_RX

//===========================================================
//===========================================================
