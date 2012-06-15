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

//
// Numonyx Forte Serial Flash Memory M25P80
//

#include "m25p80.h"
#include <m25p80_pins.h>
#include <hil/gpio.h>

#define M25P80_DEBUG 0


#if M25P80_DEBUG
#include <lib/dprint.h>
#endif

// TODO - this should be moved to dev.h
enum DEV_MODE_E {
    DEV_MODE_OFF = 0,
    // DEV_MODE_IDLE = 1,
    DEV_MODE_ON = 2
};

// Instruction codes (8 bits)
#define WREN  0x06
#define WRDI  0x04
#define RDSR  0x05
#define WRSR  0x01
#define READ  0x03
#define FREAD 0x0B
#define PP    0x02
#define SE    0xD8
#define BE    0xC7
#define DP    0xB9
#define RES   0xAB
#define DUMMY 0xAA

// Status Register Masks for the M25P80
#define WIP  0x01
#define WEL  0x02
#define BP0  0x04
#define BP1  0x08
#define BP2  0x10
#define SRWD 0x80

// actual power mode;
static uint_t m25p80_mode;

// Shortcuts
#define M25P80_WR_BYTE(b) \
    spiWriteByte(M25P80_SPI_ID, b)
#define M25P80_RD_BYTE() \
    spiReadByte(M25P80_SPI_ID)
#define M25P80_WR_MANY(buf, len) \
    spiWrite(M25P80_SPI_ID, buf, len)
#define M25P80_RD_MANY(buf, len) \
    spiRead(M25P80_SPI_ID, buf, len)


// Block while write in progress
#define M25P80_WAIT_WHILE_WIP()        \
    M25P80_SPI_ENABLE();               \
    M25P80_WR_BYTE(RDSR);              \
    do {                               \
        uint8_t dummy;                 \
        do {                           \
            dummy = M25P80_RD_BYTE();  \
        } while (dummy & WIP);         \
    } while (0);                       \
    M25P80_SPI_DISABLE();

#define M25P80_INSTR(instr)      \
    M25P80_SPI_ENABLE();         \
    M25P80_WR_BYTE(instr);       \
    M25P80_SPI_DISABLE();        \

// Instruction executed before all modifying operations
#define M25P80_WRITE_ENABLE()   \
    M25P80_INSTR(WREN);

// Send address, 24 bits, most significant bits first
#define M25P80_TX_ADDR(addr) \
    M25P80_WR_BYTE((uint8_t) ((addr >> 16) & 0xFF)); \
    M25P80_WR_BYTE(((uint8_t) (addr >>  8) & 0xFF)); \
    M25P80_WR_BYTE((uint8_t) (addr & 0xFF));

// Enter low power mode */
#define M25P80_DEEP_POWERDOWN()     \
    M25P80_INSTR(DP);               \
    m25p80_mode = DEV_MODE_OFF;


void m25p80_sleep(void)
{
    M25P80_WAIT_WHILE_WIP();
    M25P80_DEEP_POWERDOWN();
}

void m25p80_wake(void)
{
    if (m25p80_mode == DEV_MODE_OFF) {
        M25P80_INSTR(RES);
        m25p80_mode = DEV_MODE_ON;
    }
}

void m25p80_init(void) {
    spiBusInit(M25P80_SPI_ID, SPI_MODE_MASTER);

    pinAsOutput(M25P80_HOLD_PORT, M25P80_HOLD_PIN);
    pinAsOutput(M25P80_CS_PORT, M25P80_CS_PIN);
    pinSet(M25P80_CS_PORT, M25P80_CS_PIN);
    pinSet(M25P80_HOLD_PORT, M25P80_HOLD_PIN);

    // initialize into low power mode
    M25P80_DEEP_POWERDOWN();
}

static void m25p80_pageProgram(uint32_t addr, const uint8_t *buffer,
        uint16_t len)
{
    M25P80_WAIT_WHILE_WIP();
    M25P80_WRITE_ENABLE();

    M25P80_SPI_ENABLE();

    M25P80_WR_BYTE(PP);
    M25P80_TX_ADDR(addr);
    M25P80_WR_MANY(buffer, len);

    M25P80_SPI_DISABLE();
}

void m25p80_bulkErase(void)
{
    M25P80_WAIT_WHILE_WIP();
    M25P80_WRITE_ENABLE();
    M25P80_INSTR(BE);
}

void m25p80_eraseSector(uint32_t addr)
{
    M25P80_WAIT_WHILE_WIP();
    M25P80_WRITE_ENABLE();
    M25P80_SPI_ENABLE();
    M25P80_WR_BYTE(SE);
    M25P80_TX_ADDR(addr);
    M25P80_SPI_DISABLE();
}

void m25p80_read(uint32_t addr, void* buffer, uint16_t len)
{
    M25P80_WAIT_WHILE_WIP();
    M25P80_SPI_ENABLE();
    M25P80_WR_BYTE(READ);
    M25P80_TX_ADDR(addr);
    M25P80_RD_MANY(buffer, len);
    M25P80_SPI_DISABLE();
}

// Write len bytes (len <= 256) to flash at addr
void m25p80_write(uint32_t addr, const void *buf, uint16_t len)
{
#if M25P80_DEBUG
    PRINTF("m25p80_write(%llu, %u)\n", addr, len);
#endif

    if (len > M25P80_PAGE_SIZE) len = M25P80_PAGE_SIZE;

    // if buffer tail goes over page boundaries, split to two writes, because
    // otherwise the remaining bytes will be written to the beginning of
    // the same page (see M25P80 specification of Page Program instruction)
    
    const uint8_t *bufCopy = (const uint8_t *)buf;
    
    // each page is 256 byte long
    uint16_t pageOffset = (uint16_t) (addr & 0xff);
#if M25P80_DEBUG
    PRINTF("\tlen = %u, offset = %u\n", len, pageOffset);
#endif

    if (pageOffset + len > M25P80_PAGE_SIZE) {
        uint16_t safeLen = M25P80_PAGE_SIZE - pageOffset;
        m25p80_pageProgram(addr, bufCopy, safeLen);
        // write remaining bytes in the beginning of the next page
        addr = (addr & 0xffffff00) + M25P80_PAGE_SIZE;
        len -= safeLen;
        bufCopy += safeLen;

#if M25P80_DEBUG
    PRINTF("\twrite over page boundary! safeLen = %u, new addr = %llu, new len = %u\n",
            safeLen, addr, len);
#endif
    }

    m25p80_pageProgram(addr, bufCopy, len);
}
