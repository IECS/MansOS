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

#include <net/addr.h>
#include <msp430/msp430_flash.h> // TODO: use hil interface
#include <lib/hash.h>
#include <print.h>
#include <serial_number.h>

#define LOCAL_ADDR_STORAGE_ADDRESS (MSP430_FLASH_INFOMEM_START + MSP430_FLASH_INFOMEM_SEGMENT_SIZE)

// generate local address from serial number
static inline uint16_t generateLocalAddress(void) 
{
    uint16_t snum[4];
    bool ok = halGetSerialNumber((uint8_t *) snum);
    (void)ok;
    // PRINTF("generateLocalAddress... ok=%s\n", (ok ? "yes" : "no"));

    uint16_t result = 0;
    int i;
    for (i = 0; i < 4; ++i) {
        // PRINTF("snum[%d]=%#04x\n", i, snum[i]);
        result += snum[i];
    }

    // XXX: do not allow the result to be 0
    if (result == 0) result = 0x1;

    // set highest bit to zero
    return result & ~0x8000;
}

static inline void setupLocalAddr(void)
{
    if (localAddress != 0 && localAddress != 0xffff) {
        // PRINTF("setupLocalAddr: address ok: %d\n", localAddress);
        return;
    }
#if USE_FLASH
    msp430flashReadWord(LOCAL_ADDR_STORAGE_ADDRESS, &localAddress);
    if (localAddress == 0 || localAddress == 0xffff) { 
        localAddress = generateLocalAddress();
        msp430flashErase(LOCAL_ADDR_STORAGE_ADDRESS, sizeof(uint16_t));
        msp430flashWriteWord(LOCAL_ADDR_STORAGE_ADDRESS, localAddress);
        // PRINTF("setupLocalAddr: written in flash address %d\n", localAddress);
    }
#else
    localAddress = generateLocalAddress();
#endif
}

void initArchComm(void) {
    setupLocalAddr();
}
