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

//
// ADS8328: low power, 16-bit, 2-channel, 500-kHz analog to digital converter
//

#ifndef OSW_ADS8328_H
#define OSW_ADS8328_H

#include "adc_dac_pins.h"
#include <defines.h>

// ADS8328 commands
#define ADS8328_REG_CHANNEL0       0x00
#define ADS8328_REG_CHANNEL1       0x01
#define ADS8328_REG_CONFIG_READ    0x0C
#define ADS8328_REG_DATA           0x0D
#define ADS8328_REG_CONFIG_WRITE   0x0E
#define ADS8328_REG_CONFIG_DEFAULT 0x0F

enum {
    ADS8328_CHANNEL_0,
    ADS8328_CHANNEL_1,
};

void ads8328Init(void);

void ads8328SelectChannel(uint8_t channel);

bool ads8328Read(uint16_t *value);

uint16_t ads8328ReadData(void);

void ads8328RegWrite4(uint8_t address);

void ads8328RegWrite16(uint8_t address, uint16_t data);

#endif
