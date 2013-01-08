/**
 * Copyright (c) 2008-2011 the MansOS team. All rights reserved.
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
/*
 * eeprom_hal.c -- non-volatile configuration memory for TelosB
 *
 * This module uses the so-called "information memory" available on MSP430
 * to store non-volatile data. On MSP430F1611 it is 256 bytes, split in two
 * 128 byte erasable segments. To emulate random access, we use only one segment
 * to store data and copy it to the other segment whenever a write request to
 * the already-written cell comes (basic idea taken from TinyOS sources).
 *
 * To remember which segment holds the actual information, we store a version
 * byte at the beginning of both segments.
 *
 * TODO: This could be non-TelosB specific. But information memory segment count
 * seems not to be uniform across MSP430 chips.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <eeprom.h>
#include <msp430/msp430_flash.h>

#define BASE     MSP430_FLASH_INFOMEM_START
#define SEGSIZE  MSP430_FLASH_INFOMEM_SEGMENT_SIZE

/* Check if the first segment's version is later than the second one's */
static inline bool firstIsValid(void)
{
    uint8_t a, b;

    msp430flashRead(BASE, &a, sizeof(a));
    msp430flashRead(BASE + SEGSIZE, &b, sizeof(b));

    return (uint8_t)(a - b) == 1; /* Exploits unsigned wrapping */
}

void eepromRead(uint16_t addr, void *buf, size_t len)
{
    FlashAddress_t base = BASE + 1 + (firstIsValid() ? 0 : SEGSIZE);

    msp430flashRead(base + addr, buf, len);
}

void eepromWrite(uint16_t addr, const void *buf, size_t len)
{
    FlashAddress_t old, new;
    uint8_t        v;

    if (firstIsValid())
    {
        old = BASE;
        new = BASE + SEGSIZE;
    }
    else
    {
        old = BASE + SEGSIZE;
        new = BASE;
    }

    msp430flashErase(new, SEGSIZE);

    /* Increment version */
    msp430flashRead(old, &v, sizeof(v));
    v++;
    msp430flashWrite(new, &v, sizeof(v));

    /* Copy data, don't use msp430flashRead() to avoid the need of a buffer */
    if (0 < addr) {
        msp430flashWrite(new + 1, (const void *)(old + 1), addr);
    }
    msp430flashWrite(new + 1 + addr, buf, len);
    if (addr + len < EEPROM_SIZE)
    {
        msp430flashWrite(new + 1 + addr + len,
                         (const void *)(old + 1 + addr + len),
                         EEPROM_SIZE - (addr + len));
    }
}
