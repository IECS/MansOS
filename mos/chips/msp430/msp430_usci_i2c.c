/*
 * Copyright (c) 2011, Institute of Electronics and Computer Science
 * All rights reserved.
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

/**
 * msp430_usci_i2c.c -- USCI module on MSP430, I2C mode
 */

// FIXME? this seems to be broken, at least not working very well.

#include <digital.h>
#include <i2c.h>
#include "msp430_usci.h"


//
// Initialization
//

void i2cHwInit(uint8_t busId)
{
    // I2C mode: master, 7-bit addressing, 100 kbaud
#define I2C_MODE (UCMST | UCMODE_3 | UCSYNC)
#define I2C_SPEED 100000UL

#define SETUP_I2C_PINS(letterid)                                          \
    pinAsFunction(USCI_##letterid##_I2C_PORT, USCI_##letterid##_SDA_PIN); \
    pinAsFunction(USCI_##letterid##_I2C_PORT, USCI_##letterid##_SCL_PIN)
#define SETUP_USCI(id, letterid)                                                     \
    UC##letterid##CTL1 = UCSWRST;                    /* Hold the module in reset state */ \
    UC##letterid##CTL1 |= UCSSEL_2;                  /* SMCLK clock source */             \
    UC##letterid##BR0 = (CPU_HZ / I2C_SPEED) & 0xFF; /* Clock divider, lower part */      \
    UC##letterid##BR1 = (CPU_HZ / I2C_SPEED) >> 8;   /* Clock divider, higher part */     \
    UC##letterid##CTL0 = I2C_MODE;                   /* Set specified mode */             \
    UC##id##IE &= ~UC##letterid##RXIE;               /* Disable receive interrupt */      \
    UC##letterid##CTL1 &= ~UCSWRST;                  /* Release hold */

    if (busId == 0) {
        SETUP_I2C_PINS(B0);
        SETUP_USCI(0, B0);
    }
    else if (busId == 1) {
        SETUP_I2C_PINS(B1);
        SETUP_USCI(1, B1);
    }
}

// sending I2C stop condition
#define I2C_SEND_STOP(letterid) \
    UC##letterid##CTL1 |= UCTXSTP;            \
    while (UC##letterid##CTL1 & UCTXSTP)

/*
 * Writes a string to I2C and checks acknowledge
 * @param   addr        address of the slave receiver
 * @param   buf         the buffer containing the string
 * @param   len         buffer length in bytes
 * @return  0           on success, error code otherwise
 */
i2cError_t i2cHwWrite(uint8_t busId, uint8_t addr,
                      const void *buf, uint8_t len)
{
    size_t i;
    int    ret = I2C_OK;

#define I2C_WRITE_INIT(letterid) \
    UC##letterid##CTL1 |= UCSWRST;           /* Enable SW reset */    \
    UC##letterid##I2CSA = addr;    /* Set slave address */            \
    UC##letterid##CTL1 &= ~UCSWRST;  /* SW reset, resume operation */ \
    UC##letterid##CTL1 |= UCTR + UCTXSTT;    /* Transmit mode */      \

#define I2C_WRITE_BYTE1(letterid, id, byte) do {            \
        /* Send data */                                        \
        UC##letterid##TXBUF = byte;                            \
        /* Wait for either transmission clearance or error */  \
        while (1) {                                            \
            if (UC##letterid##STAT & UCNACKIFG) {              \
                /* No ack */                                   \
                ret = I2C_ACK_ERROR;                           \
                goto end;                                      \
            }                                                  \
            else if (UC##id##IFG & UC##letterid##TXIFG) {      \
                break;                                         \
            }                                                  \
        }                                                      \
    } while (0)

// TODO: will not work for B0?
#define I2C_WRITE_BYTE(letterid, id, byte) do {             \
        while (!(UC1IFG & UCB1TXIFG) && !(UCB1STAT & UCNACKIFG)); \
        UC##letterid##TXBUF = byte; \
    } while (0)

    // initalize Tx mode
#ifdef UCB0CTL0_
    if (busId == 0) {
        I2C_WRITE_INIT(B0);
    } else {
        I2C_WRITE_INIT(B1);
    }
#else
    I2C_WRITE_INIT(B0);
#endif

    // Send all bytes sequentially
    for (i = 0; i < len; i++) {
#ifdef UCB0CTL0_
        if (busId == 0) {
            I2C_WRITE_BYTE(B0, 0, ((const char *)buf)[i]);
        } else {
            I2C_WRITE_BYTE(B1, 1, ((const char *)buf)[i]);
        }
#else
        I2C_WRITE_BYTE(B0, 0, ((const char *)buf)[i]);
#endif
    }
end:
    return ret;
}

/*
 * Reads a message into buffer from I2C - requests it from a slave
 * @param   addr        address of the slave transmitter
 * @param   buf         the buffer to store the message
 * @param   len         buffer length in bytes
 * @return  received byte count
 */
uint8_t i2cHwRead(uint8_t busId, uint8_t addr,
                  void *buf, uint8_t len)
{
    size_t i;

#define I2C_READ_INIT(letterid) \
    UC##letterid##I2CSA = addr;    /* Set slave address */        \
    UC##letterid##CTL1 |= ~UCTR;   /* Receive mode */             \
    UC##letterid##CTL1 |= UCTXSTT  /* Generate START condition */

#define I2C_READ_BYTE(letterid, id, byte) do {             \
        /* Wait for either transmission clearance or error */   \
        while (1) {                                             \
            if (UC##letterid##STAT & UCNACKIFG) {               \
                /* No ack */                                    \
                goto end;                                       \
            }                                                   \
            else if (UC##id##IFG & UC##letterid##RXIFG) {       \
                break;                                          \
            }                                                   \
        }                                                       \
        /* Read data */                                         \
        byte = UC##letterid##RXBUF;                             \
    } while (0)

    // initalize Tx mode
#ifdef UCB0CTL0_
    if (busId == 0) {
        I2C_READ_INIT(B0);
    } else {
        I2C_READ_INIT(B1);
    }
#else
    I2C_READ_INIT(B0);
#endif

    // Send all bytes sequentially
    for (i = 0; i < len; i++) {
#ifdef UCB0CTL0_
        if (busId == 0) {
            I2C_READ_BYTE(B0, 0, ((char *)buf)[i]);
        } else {
            I2C_READ_BYTE(B1, 1, ((char *)buf)[i]);
        }
#else
        I2C_READ_BYTE(B0, 0, ((char *)buf)[i]);
#endif
    }
end:
    return i;
}
