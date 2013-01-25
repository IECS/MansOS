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

#include "msp430_flash.h"
#include "msp430_int.h"

#define MSP430_FLASH_DEBUG 0

#if MSP430_FLASH_DEBUG
#include <assert.h>
#endif

//
// Flash clock frequency must be in range 257 - 476 kHz.
// Note that the divider being used is: this value + 1
//
#define FLASH_CLOCK_DIVIDER (((FN0 | FN1) * CPU_MHZ) - 1)


void RAMFUNC msp430flashWriteBlock(FlashAddress_t address_, const void *data_, uint16_t dataLength)
{
    uint16_t *address = (uint16_t *) address_;
    uint16_t *data = (uint16_t *) data_;
    uint16_t blocks = dataLength / MSP430_FLASH_BLOCK_SIZE;
    uint16_t i, j;
    Msp430InterruptContext_t interruptContext;

#if MSP430_FLASH_DEBUG
    ASSERT(IS_ALIGNED(address, MSP430_FLASH_BLOCK_SIZE));
    ASSERT(IS_ALIGNED(dataLength, MSP430_FLASH_BLOCK_SIZE));
#endif

    msp430ClearAllInterrupts(&interruptContext);
    for (i = 0; i < blocks; ++i) {
        while (FCTL3 & BUSY) nop();
      
        FCTL2 = FWKEY | FSSEL1 | FLASH_CLOCK_DIVIDER;
        FCTL3 = FWKEY;
        FCTL1 = FWKEY | BLKWRT | WRT;

        for (j = 0; j < MSP430_FLASH_BLOCK_SIZE / 2; ++j) {
            *address++ = *data++;
            while(!(FCTL3 & WAIT));
        }

        FCTL1 = FWKEY;
        while (FCTL3 & BUSY) nop();
        FCTL3 = FWKEY | LOCK;
    }
    msp430RestoreAllInterrupts(&interruptContext);
}

void msp430flashWrite(FlashAddress_t address_, const void *data_, uint16_t dataLength)
{
    uint8_t *address = (uint8_t *) address_;
    const uint8_t *data = (const uint8_t *) data_;
    uint8_t *endAddress = address + dataLength;
    Msp430InterruptContext_t interruptContext;

    msp430ClearAllInterrupts(&interruptContext);

    FCTL3 = FWKEY;
    FCTL1 = FWKEY + WRT;

    while (address < endAddress) {
        *address++ = *data++;
    }

    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;

    msp430RestoreAllInterrupts(&interruptContext);
}

void msp430flashErase(FlashAddress_t address, uint16_t byteCount)
{
    Msp430InterruptContext_t interruptContext;
    FlashAddress_t end = address + byteCount;

    do {
        msp430ClearAllInterrupts(&interruptContext);

        // select SMCLK with appropriate divider
        FCTL2 = FWKEY | FSSEL_2 | FLASH_CLOCK_DIVIDER;
        // Unlock flash
        FCTL3 = FWKEY;

        // Segment erase
        FCTL1 = FWKEY | ERASE;
        // Dummy write in 'address' to start the erase operation
        *(uint16_t *)address = 0;

        // Disable erase or write
        FCTL1 = FWKEY;
        // Lock flash
        FCTL3 = FWKEY | LOCK;

        msp430RestoreAllInterrupts(&interruptContext);

        if (address >= MSP430_FLASH_INFOMEM_START
                && address <= MSP430_FLASH_INFOMEM_END) {
            address += MSP430_FLASH_INFOMEM_SEGMENT_SIZE;
        } else {
            address += MSP430_FLASH_SEGMENT_SIZE;
        }
    } while ((int16_t)end - (int16_t)address > 0);
}
