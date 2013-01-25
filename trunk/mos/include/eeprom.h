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

#ifndef MANSOS_EEPROM_H
#define MANSOS_EEPROM_H

/// \file
/// eeprom.h -- access to non-volatile configuration memory.
///
/// On MSP430 the "info segments" of the internal flash are used for this purpose.
/// The amount of available EEPROM memory is a little less than half of info flash size
/// due to the buffering algorithm used.
/// EEPROM address range for this user API is [0, EEPROM_SIZE-1] on all platforms.
///

#include <stdtypes.h>

//! Read from EEPROM
void eepromRead(uint16_t addr, void *buf, size_t len);
//! Write to EEPROM
void eepromWrite(uint16_t addr, const void *buf, size_t len);

extern inline void eepromInit(void);

// implementation
#include <eeprom_hal.h> /* Will define EEPROM_SIZE */

//! Platform-specific EEPROM size in bytes
#ifndef EEPROM_SIZE
#define EEPROM_SIZE 0
#endif

#endif
