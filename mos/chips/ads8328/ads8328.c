/*
 * Copyright (c) 2013 OSW. All rights reserved.
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

#include "ads8328.h"
#include <spi.h>
#include <delay.h>
#include <print.h>
#include <serial.h>

// SCLK frequency can be up to 50 MHz

#define ADS8328_SPI_ENABLE()   spiSlaveEnable(ADS8328_CS_PORT, ADS8328_CS_PIN)
#define ADS8328_SPI_DISABLE()  spiSlaveDisable(ADS8328_CS_PORT, ADS8328_CS_PIN)
#define ADS8328_REINIT() do {                       \
        UCB1CTL1 |= UCSWRST;                        \
        UCB1CTL0 &= ~UCCKPH;                        \
        UCB1CTL1 &= ~UCSWRST;                       \
    } while (0)

static void spiExchange(uint8_t busId, void *buf_, uint16_t len) {
    uint8_t *buf = (uint8_t *) buf_;
    uint8_t *end = buf + len;
    while (buf < end) {
        *buf = spiExchByte(busId, *buf);
        buf++;
    }
}


uint16_t ads8328ReadData(void)
{
    uint16_t result;
    ADS8328_SPI_ENABLE();
    result = spiReadByte(ADS8328_SPI_ID);
    result <<= 8;
    result |= spiReadByte(ADS8328_SPI_ID);
    ADS8328_SPI_DISABLE();

    return result;
}

uint16_t ads8328ConfigRegRead(void)
{ 
    uint16_t result;

    uint8_t array[2];
    array[0] = ADS8328_REG_CONFIG_READ << 4;
    array[1] = 0;
    ADS8328_SPI_ENABLE();
    spiExchange(ADS8328_SPI_ID, array, sizeof(array));
    ADS8328_SPI_DISABLE();

    result = array[0];
    result <<= 8;
    result |= array[1];

    // use 12 least-significant bits
    result &= 0xfff;
    return result;
}

void ads8328RegWrite4(uint8_t address)
{
#if 1
    uint8_t array[2];
    array[0] = address << 4;
    array[1] = 0;
    ADS8328_SPI_ENABLE();
    spiExchange(ADS8328_SPI_ID, array, sizeof(array));
    ADS8328_SPI_DISABLE();
#else
    ADS8328_SPI_ENABLE();
    sw_spiWriteNibble(address);
    ADS8328_SPI_DISABLE();
#endif
}

void ads8328RegWrite16(uint8_t address, uint16_t data)
{
    uint8_t array[2];
    array[0] = (address << 4) | ((data >> 8) & 0xf);
    array[1] = data & 0xff;
    ADS8328_SPI_ENABLE();
    spiExchange(ADS8328_SPI_ID, array, sizeof(array));
    ADS8328_SPI_DISABLE();
}

/*
ADS8328 does not work with default MansOS HW SPI configuration, the phase needs to be changed
Example for testbed2 platform:
    UCA1CTL1 = UCSWRST;               
    UCA1CTL1 |= UCSSEL_2;                        
    UCA1BR0 = (CPU_HZ / SPI_SPEED) & 0xFF; 
    UCA1BR1 = (CPU_HZ / SPI_SPEED) >> 8;   
    UCA1CTL0 = (UCMSB | UCMST | UCMODE_2 | UCSYNC);            
    UC1IE &= ~UCA1RXIE;             
	UCA1CTL1 &= ~UCSWRST;   
*/
void ads8328Init(void)
{
    spiBusInit(ADS8328_SPI_ID, SPI_MODE_MASTER);
    pinAsOutput(ADS8328_CS_PORT, ADS8328_CS_PIN);
    
    ADS8328_REINIT();
    // setup Conversion Start pin
    pinAsOutput(ADS8328_CONVST_PORT, ADS8328_CONVST_PIN);
    pinSet(ADS8328_CONVST_PORT, ADS8328_CONVST_PIN);
    
    // setup End of Conversion pin (low while conversion in progress)
    pinAsInput(ADS8328_EOC_PORT, ADS8328_EOC_PIN);
    
    // select manual channel select mode
    uint16_t config = ads8328ConfigRegRead();
    config &= ~ADS8328_CFR_AUTO_CHANSEL;
    ads8328RegWrite16(ADS8328_REG_CONFIG_WRITE, config);

    // channel 0 active by default
}

void ads8328SelectChannel(uint8_t channel)
{
    // select channel
    ads8328RegWrite4(channel == ADS8638_CHANNEL_0 ?
                     ADS8328_REG_CHANNEL0 :
                     ADS8328_REG_CHANNEL1);
}

bool ads8328Read(uint16_t *value)
{
    // wait for previous conversion to end
    while (!pinRead(ADS8328_EOC_PORT, ADS8328_EOC_PIN));

    // start the conversion
    pinClear(ADS8328_CONVST_PORT, ADS8328_CONVST_PIN);
    NOP1(); // minimum 40 ns delay
    pinSet(ADS8328_CONVST_PORT, ADS8328_CONVST_PIN);

    // wait for end of conversion
    while (!pinRead(ADS8328_EOC_PORT, ADS8328_EOC_PIN));

    uint8_t array[2];
    array[0] = ADS8328_REG_DATA << 4;
    array[1] = 0;
    ADS8328_SPI_ENABLE();
    spiExchange(ADS8328_SPI_ID, array, sizeof(array));
    ADS8328_SPI_DISABLE();

    *value = array[0];
    *value <<= 8;
    *value |= array[1];

    return true;
}
