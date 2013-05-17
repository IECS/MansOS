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

#ifndef MANSOS_M25PXX_H
#define MANSOS_M25PXX_H

#include <stdtypes.h>
#include <delay.h>

//
// Numonyx Forte Serial Flash Memory M25P80 and M25P16
//

// initialize pin directions and SPI in general. Enter low power mode afterwards
void m25pxx_init(void);
// Enter low power mode (wait for last instruction to complete)
void m25pxx_sleep(void);
// Exit low power mode
void m25pxx_wake(void);
// Read a block of data from addr
void m25pxx_read(uint32_t addr, void* buffer, uint16_t len);
// Write len bytes (len <= 256) to flash at addr
// Block can split over multiple sectors/pages
void m25pxx_write(uint32_t addr, const void *buf, uint16_t len);
// Erase the entire flash
void m25pxx_bulkErase(void);
// Erase on sector, containing address addr. Addr is not the number of sector,
// rather an address (any) inside the sector
void m25pxx_eraseSector(uint32_t addr);


// Wrappers, borrowed from WSN430 library
// http://senstools.gforge.inria.fr

/**
 * \brief Save a whole page of data to the memory.
 * \param page the memory page number to write to.
 * \param buffer a pointer to the data to copy
 */
#define m25pxx_savePage(page, buffer) \
    m25pxx_page_program(((uint32_t) (page)) << 8, (buffer), 256)

/**
 * \brief Read data from the memory
 * \param page the memory page number to read from.
 * \param buffer a pointer to the buffer to store the data to.
 */
#define m25pxx_loadPage(page, buffer) \
    m25pxx_read(((uint32_t) (page)) << 8, (buffer), 256)


// ----------------------------------------------

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

// Status Register Masks for the M25PXX
#define WIP  0x01
#define WEL  0x02
#define BP0  0x04
#define BP1  0x08
#define BP2  0x10
#define SRWD 0x80


// Shortcuts
#define M25PXX_WR_BYTE(b) \
    spiWriteByte(M25PXX_SPI_ID, b)
#define M25PXX_RD_BYTE() \
    spiReadByte(M25PXX_SPI_ID)
#define M25PXX_WR_MANY(buf, len) \
    spiWrite(M25PXX_SPI_ID, buf, len)
#define M25PXX_RD_MANY(buf, len) \
    spiRead(M25PXX_SPI_ID, buf, len)


// Block while write in progress
#define M25PXX_WAIT_WHILE_WIP()        \
    M25PXX_SPI_ENABLE();               \
    M25PXX_WR_BYTE(RDSR);              \
    do {                               \
        uint8_t dummy;                 \
        do {                           \
            dummy = M25PXX_RD_BYTE();  \
        } while (dummy & WIP);         \
    } while (0);                       \
    M25PXX_SPI_DISABLE();

#define M25PXX_INSTR(instr)      \
    M25PXX_SPI_ENABLE();         \
    M25PXX_WR_BYTE(instr);       \
    M25PXX_SPI_DISABLE();        \

// Instruction executed before all modifying operations
#define M25PXX_WRITE_ENABLE()   \
    M25PXX_INSTR(WREN);

// Send address, 24 bits, most significant bits first
#define M25PXX_TX_ADDR(addr) \
    M25PXX_WR_BYTE((uint8_t) ((addr >> 16) & 0xFF)); \
    M25PXX_WR_BYTE(((uint8_t) (addr >>  8) & 0xFF)); \
    M25PXX_WR_BYTE((uint8_t) (addr & 0xFF));

// Enter low power mode */
#define M25PXX_DEEP_POWERDOWN()     \
    M25PXX_INSTR(DP);               \

// ------------------------------------------------

static inline void m25pxx_init_low_power(void) 
{
    spiBusInit(M25PXX_SPI_ID, SPI_MODE_MASTER);

    pinAsOutput(M25PXX_HOLD_PORT, M25PXX_HOLD_PIN);
    pinAsOutput(M25PXX_CS_PORT, M25PXX_CS_PIN);
    pinSet(M25PXX_CS_PORT, M25PXX_CS_PIN);
    pinSet(M25PXX_HOLD_PORT, M25PXX_HOLD_PIN);

    udelay(1);

    // initialize into low power mode
    M25PXX_DEEP_POWERDOWN();
}

#endif
