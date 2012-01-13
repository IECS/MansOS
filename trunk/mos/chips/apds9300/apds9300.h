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

#ifndef MANSOS_APDS9300_H
#define MANSOS_APDS9300_H

#include "i2c_soft.h"
#include "stdmansos.h"

#define APDS_INT_PORT      2
#define APDS_INT_PIN       5

//#define SLAVE_ADDRESS      0x39 // 0b00111001 - when "left to float"
#define SLAVE_ADDRESS      0x29

#define CONTROL_REG        0x0
#define TIMING_REG         0x1
#define THRESHLOWLOW_REG   0x2
#define THRESHLOWHIGH_REG  0x3
#define THRESHHIGHLOW_REG  0x4
#define THRESHHIGHHIGH_REG 0x5
#define INTERRUPT_REG      0x6
#define CRC_REG            0x8
#define ID_REG             0xA
#define DATA0LOW_REG       0xC
#define DATA0HIGH_REG      0xD
#define DATA1LOW_REG       0xE
#define DATA1HIGH_REG      0xF

#define COMMAND            0x80
#define INT_CLEAR          0x40
#define I2C_WORD           0x20

#define POWER_UP           0x03
#define POWER_DOWN         0x00

#define LEVEL_INTERRUPT    0x10
#define DISABLE_INTERRUPT  0x00

#define INTEGRATION_TIME_SHORT1  0x2 // 13.7 ms
#define INTEGRATION_TIME_SHORT2  0x2 // 101 ms
#define INTEGRATION_TIME_NORMAL  0x2 // 402 ms
#define INTEGRATION_TIME_MANUAL  0x3 // manually controlled

#define INTEGRATION_TIME_MS  402  // ms

void apdsInit(void);

uint8_t apdsWriteByte(uint8_t cmd, uint8_t val);

uint8_t apdsWriteWord(uint8_t cmd, uint16_t val);

uint8_t apdsReadByte(uint8_t cmd, uint8_t *value);

uint8_t apdsReadWord(uint8_t cmd, uint16_t *value);

#define apdsCommand(reg, data) apdsWriteByte((reg) | COMMAND | INT_CLEAR, data)

bool apdsOn(void);

void apdsOff(void);

ISR(PORT2, apds_interrupt);

bool apdsData0Read(uint16_t *data);

bool apdsData1Read(uint16_t *data);
#endif
