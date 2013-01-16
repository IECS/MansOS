/**
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#ifndef MANSOS_AD5258_H
#define MANSOS_AD5258_H

#include <kernel/defines.h>

// I2C address when both address pins are zero
#define AD5258_ADDRESS    0x18

// traditional for I2C access; combined with the slave address
#define AD5258_WRITE_FLAG  0x0
#define AD5258_READ_FLAG   0x1

//
// AD5258 configuration commands
//
#define AD5258_CMD_ACCESS_RDAC       0x00
#define AD5258_CMD_ACCESS_EEPROM     0x20
#define AD5258_CMD_SET_WRITEPROTECT  0x40
// RDAC -> EEPROM
#define AD5258_CMD_STORE_TO_EEPROM   0xC0
// EEPROM -> RDAC
#define AD5258_CMD_STORE_TO_RDAC     0xA0
// consecutively & individually (the first byte!)
#define AD5258_CMD_READ_TOLERANCE    0x3E
#define AD5258_CMD_READ_TOLERANCE    0x3E


// Initialize the AD5258 digital potentiometer
void ad5258Init(void);

//
// Write out voltage vaue to AD5258 digital potentiometer.
// Supported values are 0..63; the chip accepts 6 config bits.
//
bool ad5258Write(uint8_t value);


#endif
