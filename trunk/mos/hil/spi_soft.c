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

//==============================================================================
// Software controlled SPI bus implementation, master mode only
//==============================================================================

#include <hil/spi_soft.h>

#if SW_SPI_PIN_ERROR != 1

// Send MSB bit to MOSI pin, shift the whole byte left
#define SW_SPI_PROPAGATE_BIT(txByte) \
    pinWrite(SW_MOSI_PORT, SW_MOSI_PIN, (txByte & 0x80)); \
    txByte <<= 1;

// Store MISO value in the LSB bit of byte
#define SW_SPI_RECV_BIT(rxByte) \
     rxByte |= SW_SPI_MISO_GET();


/*
 * From Wikipedia:
 * http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Clock_polarity_and_phase
 *  For CPHA=0, data are captured on the clock's leading edge and data are
 *      propagated on a trailing edge
 *  For CPHA=1, data are captured on the clock's trailing edge and data are
 *      propagated on a leading edge
 *
 *  Leading edge is rising if CPOL=0, falling otherwise
 *  Trailing edge is falling if CPOL=1, rising otherwise
 */

//------------------------------------------------------------------------------
// Exchange byte with a slave: write a byte to SPI and returns response,
// received from the slave in full-duplex mode
// Sends MST bit from the byte, shifts it left and stores received byte in
// LSB bit of the same byte
// Does not change any Slave-Select pin!
//
// Returns byte received from the slave
//------------------------------------------------------------------------------
uint8_t sw_spiExchByte(uint8_t byte) {
    uint_t bit;
    for (bit = 0; bit < 8; ++bit) {

#if SW_SPI_CPHA == 0
        // CPHA == 0

        // write MOSI on trailing edge of previous clock
        SW_SPI_PROPAGATE_BIT(byte);

        // half a clock cycle before leading edge
        SW_SPI_SLEEP_HALF_CYCLE();

        // read MISO on leading edge
        SW_SPI_SCLK_LE();
        SW_SPI_RECV_BIT(byte);

        // half a clock cycle before trailing edge
        SW_SPI_SLEEP_HALF_CYCLE();

        // trailing edge
        SW_SPI_SCLK_TR();
#else
        // CPHA == 1

        // half a clock cycle before leading edge
        SW_SPI_SLEEP_HALF_CYCLE();

        // write MOSI on leading edge
        SW_SPI_PROPAGATE_BIT(byte);
        SW_SPI_SCLK_LE();

        // half a clock cycle before trailing edge
        SW_SPI_SLEEP_HALF_CYCLE();

        // read MISO on trailing edge
        SW_SPI_SCLK_TR();
        SW_SPI_RECV_BIT(byte);
#endif
    }

    return byte;
}

#else // SW_SPI_PIN_ERROR == 1

// no soft-SPI pins defined
uint8_t sw_spiExchByte(uint8_t byte) {
    return 0;
}

#endif // SW_SPI_PIN_ERROR != 1
