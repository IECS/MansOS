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

#ifndef MANSOS_ADS8638_H
#define MANSOS_ADS8638_H

#include "adc_dac_pins.h"
#include <defines.h>

#define ADS8638_REG_MANUAL         0x04
#define ADS8638_REG_AUTO           0x05
#define ADS8638_REG_RESET          0x01
#define ADS8638_REG_AUX_CONFIG     0x06
#define ADS8638_REG_CHAN_SEL       0x0C // channel select for auto mode
#define ADS8638_REG_RANGE_SEL_BASE 0x10

#define ADS8638_REG_TEMP_FLAG      0x20
#define ADS8638_REG_TRIPPED_FLAG_0 0x21
#define ADS8638_REG_ALARM_FLAG_0   0x22
#define ADS8638_REG_TRIPPED_FLAG_4 0x23
#define ADS8638_REG_ALARM_FLAG_4   0x24

#define ADS8638_REG_PAGE_SEL       0x7F

#define ADS8638_READ_FLAG 0x1

#define ADS8638_RANGE_CONFIG   0x0 // Ranges as selected through the configuration registers
#define ADS8638_RANGE_10V      0x1 // Range is set to ±10V
#define ADS8638_RANGE_5V       0x2 // Range is set to ±5V
#define ADS8638_RANGE_2_5V     0x3 // Range is set to ±2.5V
#define ADS8638_RANGE_PLUS_10V 0x5 // Range is set to 0V to 10V
#define ADS8638_RANGE_PLUS_5V  0x6 // Range is set to 0V to 5V
#define ADS8638_POWER_DOWN     0x7 // Powers down the device immediately
                                   // after the 16th SCLK falling edge

#define ADS8638_INTERNAL_VREF_ON 0xC

#define ADS8638_SPI_WRITE_FLAG 0x1

enum {
    ADS8638_CHANNEL_0,
    ADS8638_CHANNEL_1,
    ADS8638_CHANNEL_2,
    ADS8638_CHANNEL_3,
    ADS8638_CHANNEL_4,
    ADS8638_CHANNEL_5,
    ADS8638_CHANNEL_6,
    ADS8638_CHANNEL_7,
};

void ads8638Init(void);

void ads8638SelectChannel(uint8_t channel, uint8_t range);

bool ads8638Read(uint16_t *value);

static inline uint16_t ads8638ReadChannel(uint8_t channel) {
    uint16_t result;
    if (!ads8638Read(&result)) {
        result = 0xffff;
    }
    return result;
}

uint8_t ads8638RegRead(uint8_t address);

void ads8638RegWrite(uint8_t address, uint8_t data);

#endif
