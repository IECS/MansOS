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

#include "adc_dac_pins.h"
#include <kernel/defines.h>

#define DAC7718_REG_CONFIG         0x0
#define DAC7718_REG_MONITOR        0x1
#define DAC7718_REG_GPIO           0x2
#define DAC7718_REG_OFFSET_DAC_A   0x3
#define DAC7718_REG_OFFSET_DAC_B   0x4
#define DAC7718_REG_SPI_MODE       0x0
#define DAC7718_REG_BROADCAST      0x7
#define DAC7718_REG_DAC_0          0x8
#define DAC7718_REG_DAC_1          0x9
#define DAC7718_REG_DAC_2          0xA
#define DAC7718_REG_DAC_3          0xB
#define DAC7718_REG_DAC_4          0xC
#define DAC7718_REG_DAC_5          0xD
#define DAC7718_REG_DAC_6          0xE
#define DAC7718_REG_DAC_7          0xF

#define DAC7718_CONFIG_AB         (1 << 15)
#define DAC7718_CONFIG_LD         (1 << 14)
#define DAC7718_CONFIG_RST        (1 << 13)
#define DAC7718_CONFIG_PDA        (1 << 12) // power down group a (regs 0 - 3)
#define DAC7718_CONFIG_PDB        (1 << 11) // power down group a (regs 4 - 7)
#define DAC7718_CONFIG_SCE        (1 << 10) // 1 = enable correction engine
#define DAC7718_CONFIG_GAINA      (1 << 8)  // 0 = gain 6, 1 = gain 4
#define DAC7718_CONFIG_GAINB      (1 << 7)
#define DAC7718_CONFIG_DSDO       (1 << 6)
#define DAC7718_CONFIG_NOP        (1 << 5)
#define DAC7718_CONFIG_W2         (1 << 4)

#define DAC7718_READ_FLAG         (1 << 7)

enum {
    DAC7718_CHANNEL_0,
    DAC7718_CHANNEL_1,
    DAC7718_CHANNEL_2,
    DAC7718_CHANNEL_3,
    DAC7718_CHANNEL_4,
    DAC7718_CHANNEL_5,
    DAC7718_CHANNEL_6,
    DAC7718_CHANNEL_7,
};

// DAC7718_CONFIG_SCE
#define DEFAULT_CONFIG   (DAC7718_CONFIG_GAINA | DAC7718_CONFIG_GAINB)

// ---------------------------------------------------
// user API

void dac7718Init(void);

void dac7718SelectChannel(uint8_t channel);

//
// Value is unsigned integer [0...0xfff],
// output voltage is [-3 * V_ref ... +3 * V_ref]
//
void dac7718Write(uint16_t value);

void dac7718WriteChannel(uint8_t channel, uint16_t value);

void dac7718WriteBroadcast(uint16_t value);
