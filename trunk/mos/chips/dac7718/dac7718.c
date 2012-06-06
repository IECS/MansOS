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

#include "dac7718.h"
#include <hil/spi.h>
#include <lib/dprint.h>

#define DAC7718_SPI_ENABLE()   spiSlaveEnable(DAC7718_CS_PORT, DAC7718_CS_PIN)
#define DAC7718_SPI_DISABLE()  spiSlaveDisable(DAC7718_CS_PORT, DAC7718_CS_PIN)

static uint8_t dacChannel;

static void dac7718RegWrite(uint8_t address, uint16_t data)
{
    // all writes must be 24 bit
    DAC7718_SPI_ENABLE();
    spiWriteByte(DAC7718_SPI_ID, address);
    spiWriteByte(DAC7718_SPI_ID, data >> 8);
    spiWriteByte(DAC7718_SPI_ID, data & 0xff);
    DAC7718_SPI_DISABLE();
}

void dac7718Init(void)
{
    spiBusInit(DAC7718_SPI_ID, SPI_MODE_MASTER);

    pinAsOutput(DAC7718_CS_PORT, DAC7718_CS_PIN);
    pinAsOutput(DAC7718_RST_PORT, DAC7718_RST_PIN);
    pinAsOutput(DAC7718_RSTSEL_PORT, DAC7718_RSTSEL_PIN);
    pinAsOutput(DAC7718_LDAC_PORT, DAC7718_LDAC_PIN);
    pinAsOutput(DAC7718_CLR_PORT, DAC7718_CLR_PIN);
    pinAsOutput(DAC7718_WAKEUP_PORT, DAC7718_WAKEUP_PIN);
    pinAsOutput(DAC7718_BTC_PORT, DAC7718_BTC_PIN);
    // pinAsOutput(DAC7718_DGND_PORT, DAC7718_DGND_PIN);
    // pinAsOutput(DAC7718_IOVDD_PORT, DAC7718_IOVDD_PIN);

    DAC7718_SPI_DISABLE();
    pinSet(DAC7718_RST_PORT, DAC7718_RST_PIN);
    // 0V on output pins after reset
    pinSet(DAC7718_RSTSEL_PORT, DAC7718_RSTSEL_PIN);
    // async mode, update immediately
    pinClear(DAC7718_LDAC_PORT, DAC7718_LDAC_PIN);
    pinSet(DAC7718_CLR_PORT, DAC7718_CLR_PIN);
    pinClear(DAC7718_WAKEUP_PORT, DAC7718_WAKEUP_PIN);
    // use straight binary code
    pinClear(DAC7718_BTC_PORT, DAC7718_BTC_PIN);
    // pinClear(DAC7718_DGND_PORT, DAC7718_DGND_PIN);
    // pinSet(DAC7718_IOVDD_PORT, DAC7718_IOVDD_PIN);
}

void dac7718SelectChannel(uint8_t channel)
{
    dacChannel = channel;
}

void dac7718Write(uint16_t value)
{
    dac7718RegWrite(DAC7718_REG_DAC_0 + dacChannel, value << 4);
}
