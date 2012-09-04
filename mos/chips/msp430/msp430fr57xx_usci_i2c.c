/**
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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

#include <i2c.h>
#include "msp430fr57xx_usci.h"


//
// Initialization
//

void i2cInit(void)
{
     // I2C mode: master, 7-bit addressing, 100 kbaud
#define I2C_MODE (UCMST | UCMODE_3 | UCSYNC)
#define I2C_SPEED 100000UL

    P1SEL1 |= BV(USCI_B0_SDA_PIN) + BV(USCI_B0_SCL_PIN);

    UCB0CTL1 = UCSWRST;                    // Hold the module in reset state
    UCB0CTL1 |= UCSSEL_2;                  // SMCLK clock source
    UCB0BR0 = (CPU_HZ / I2C_SPEED) & 0xFF; // Clock divider, lower part
    UCB0BR1 = (CPU_HZ / I2C_SPEED) >> 8;   // Clock divider, higher part
    UCB0CTL0 = I2C_MODE;                   // Set specified mode
    UCB0IE &= ~UCB0RXIE;                   // Disable receive interrupt
    UCB0CTL1 &= ~UCSWRST;                  // Release hold
}

void i2cOn(void) { }
void i2cOff(void) { }


//
// Send/receive functions
//

uint8_t i2cWrite(uint8_t addr, const void *buf, uint8_t len)
{
    size_t i;
    int    ret = I2C_OK;

    UCB0I2CSA = addr;    // Set slave address
    UCB0CTL1 |= UCTR;    // Transmit mode
    UCB0CTL1 |= UCTXSTT; // Generate START condition

    // Send all bytes sequentially
    for (i = 0; i < len; i++)
    {
        // Wait for either transmission clearance or error
        while (1)
        {
            if (UCB0STATW & UCNACKIFG)
            {
                // No ack
                ret = I2C_ACK_ERROR;
                goto end;
            }
            else if (UC0IFG & UCB0TXIFG)
            {
                break;
            }
        }

        // Send data
        UCB0TXBUF = ((const char *)buf)[i];
    }

end:
    UCB0CTL1 |= UCTXSTP; // Issue STOP condition
    while (UCB0CTL1 & UCTXSTP);

    return ret;
}

uint8_t i2cRead(uint8_t addr, void *buf, uint8_t len)
{
    size_t i;

    UCB0I2CSA = addr;    // Set slave address
    UCB0CTL1 &= ~UCTR;   // Receive mode
    UCB0CTL1 |= UCTXSTT; // Generate START condition

    // Receive data, but watch for buffer overrun
    for (i = 0; i < len; i++)
    {
        // Wait for next character or error
        while (1)
        {
            if (UCB0STATW & UCNACKIFG) // No ack
            {
                goto end;
            }
            else if (UC0IFG & UCB0RXIFG)
            {
                break;
            }
        }

        // Read data
        ((char *)buf)[i] = UCB0RXBUF;
    }

end:
    UCB0CTL1 |= UCTXSTP; // Force STOP
    while (UCB0CTL1 & UCTXSTP);

    return i;
}

i2cError_t i2cWriteByte(uint8_t addr, uint8_t txByte)
{
    return i2cWrite(addr, &txByte, sizeof(txByte));
}

uint8_t i2cReadByte(uint8_t addr, uint8_t *rxByte)
{
    return i2cRead(addr, rxByte, sizeof(*rxByte));
}
