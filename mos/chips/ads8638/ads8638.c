/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
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

#include "ads8638.h"
#include <spi.h>
#include <delay.h>
#include <print.h>
#include <serial.h>

#define ADS8638_SPI_ENABLE()   spiSlaveEnable(ADS8638_CS_PORT, ADS8638_CS_PIN)
#define ADS8638_SPI_DISABLE()  spiSlaveDisable(ADS8638_CS_PORT, ADS8638_CS_PIN)

#define ADS_REINIT() do {                       \
        UCB1CTL1 |= UCSWRST;                    \
        UCB1CTL0 |= UCCKPH;                     \
        UCB1CTL1 &= ~UCSWRST;                   \
    } while (0)

static uint8_t adsChannel;
static uint8_t adsRange;

uint8_t ads8638RegRead(uint8_t address)
{
    uint8_t result;
    ADS_REINIT();
    ADS8638_SPI_ENABLE();
    spiWriteByte(ADS8638_SPI_ID, (address << 1) | ADS8638_SPI_WRITE_FLAG);
    result = spiReadByte(ADS8638_SPI_ID);
    ADS8638_SPI_DISABLE();
    return result;
}

void ads8638RegWrite(uint8_t address, uint8_t data)
{
    ADS_REINIT();
    ADS8638_SPI_ENABLE();
    spiWriteByte(ADS8638_SPI_ID, address << 1);
    spiWriteByte(ADS8638_SPI_ID, data);
    ADS8638_SPI_DISABLE();
}

void ads8638Init(void)
{
//    PRINTF("ads8638Init.\n");
    spiBusInit(ADS8638_SPI_ID, SPI_MODE_MASTER);

    pinAsOutput(ADS8638_CS_PORT, ADS8638_CS_PIN);
    ADS8638_SPI_DISABLE();

    // turn on internal ref
    ads8638RegWrite(ADS8638_REG_AUX_CONFIG, ADS8638_INTERNAL_VREF_ON);

    // select channel 0 with configured range
    ads8638SelectChannel(ADS8638_CHANNEL_0, ADS8638_RANGE_CONFIG);

//    PRINTF("..done\n");
}

void ads8638SelectChannel(uint8_t channel, uint8_t range)
{
    adsChannel = channel;
    adsRange = range;
}

bool ads8638Read(uint16_t *value)
{
    uint8_t b0, b1;

    ads8638RegWrite(ADS8638_REG_MANUAL, (adsChannel << 4) | (adsRange << 1));

    // discard the first frame
    ADS8638_SPI_ENABLE();
    b0 = spiReadByte(ADS8638_SPI_ID);
    b1 = spiReadByte(ADS8638_SPI_ID);
    ADS8638_SPI_DISABLE();

    ADS8638_SPI_ENABLE();
    b0 = spiReadByte(ADS8638_SPI_ID);
    b1 = spiReadByte(ADS8638_SPI_ID);
    ADS8638_SPI_DISABLE();

    *value = b0 & 0xf;
    *value <<= 8;
    *value |= b1;

    // PRINTF("got %#02x %#02x\n", b0, b1);
    // PRINTF("channel = %d\n", b0 >> 4);
    // PRINTF("value = %d\n", *value);

    return true;
}
