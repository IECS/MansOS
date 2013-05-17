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

//
// Numonyx Forte Serial Flash Memory M25P80 and M25P16
//

#include <serial.h>
#include <extflash.h>

#define M25PXX_DEBUG 0

#if M25PXX_DEBUG
#include <lib/dprint.h>
#endif

#define M25PXX_SERIAL_CAPTURE()                                    \
    if (serial[M25PXX_SPI_ID].function != SERIAL_FUNCTION_FLASH) { \
        spiBusInit(M25PXX_SPI_ID, SPI_MODE_MASTER);                \
    }


// actual power mode;
static bool m25pxxIsOn;

void m25pxx_sleep(void)
{
    M25PXX_SERIAL_CAPTURE();
    M25PXX_WAIT_WHILE_WIP();
    M25PXX_DEEP_POWERDOWN();
    m25pxxIsOn = false;
    serial[M25PXX_SPI_ID].function = SERIAL_FUNCTION_UNUSED;
    spiBusDisable(M25PXX_SPI_ID);
}

void m25pxx_wake(void)
{
    M25PXX_SERIAL_CAPTURE();
    if (!m25pxxIsOn) {
        M25PXX_INSTR(RES);
        m25pxxIsOn = true;
    }
}

void m25pxx_init(void) {
    m25pxx_init_low_power();
}

static void m25pxx_pageProgram(uint32_t addr, const uint8_t *buffer,
        uint16_t len)
{
    M25PXX_WAIT_WHILE_WIP();
    M25PXX_WRITE_ENABLE();

    M25PXX_SPI_ENABLE();

    M25PXX_WR_BYTE(PP);
    M25PXX_TX_ADDR(addr);
    M25PXX_WR_MANY(buffer, len);

    M25PXX_SPI_DISABLE();
}

void m25pxx_bulkErase(void)
{
    M25PXX_SERIAL_CAPTURE();
    M25PXX_WAIT_WHILE_WIP();
    M25PXX_WRITE_ENABLE();
    M25PXX_INSTR(BE);
}

void m25pxx_eraseSector(uint32_t addr)
{
    M25PXX_SERIAL_CAPTURE();
    M25PXX_WAIT_WHILE_WIP();
    M25PXX_WRITE_ENABLE();
    M25PXX_SPI_ENABLE();
    M25PXX_WR_BYTE(SE);
    M25PXX_TX_ADDR(addr);
    M25PXX_SPI_DISABLE();
}

void m25pxx_read(uint32_t addr, void* buffer, uint16_t len)
{
    M25PXX_SERIAL_CAPTURE();
    M25PXX_WAIT_WHILE_WIP();
    M25PXX_SPI_ENABLE();
    M25PXX_WR_BYTE(READ);
    M25PXX_TX_ADDR(addr);
    M25PXX_RD_MANY(buffer, len);
    M25PXX_SPI_DISABLE();
}

// Write len bytes (len <= 256) to flash at addr
void m25pxx_write(uint32_t addr, const void *buf, uint16_t len)
{
    M25PXX_SERIAL_CAPTURE();
#if M25PXX_DEBUG
    PRINTF("m25pxx_write(%llu, %u)\n", addr, len);
#endif

    if (len > M25PXX_PAGE_SIZE) len = M25PXX_PAGE_SIZE;

    // if buffer tail goes over page boundaries, split to two writes, because
    // otherwise the remaining bytes will be written to the beginning of
    // the same page (see M25PXX specification of Page Program instruction)

    const uint8_t *bufCopy = (const uint8_t *)buf;

    // each page is 256 byte long
    uint16_t pageOffset = (uint16_t) (addr & 0xff);
#if M25PXX_DEBUG
    PRINTF("\tlen = %u, offset = %u\n", len, pageOffset);
#endif

    if (pageOffset + len > M25PXX_PAGE_SIZE) {
        uint16_t safeLen = M25PXX_PAGE_SIZE - pageOffset;
        m25pxx_pageProgram(addr, bufCopy, safeLen);
        // write remaining bytes in the beginning of the next page
        addr = (addr & 0xffffff00) + M25PXX_PAGE_SIZE;
        len -= safeLen;
        bufCopy += safeLen;

#if M25PXX_DEBUG
    PRINTF("\twrite over page boundary! safeLen = %u, new addr = %llu, new len = %u\n",
            safeLen, addr, len);
#endif
    }

    m25pxx_pageProgram(addr, bufCopy, len);
}
