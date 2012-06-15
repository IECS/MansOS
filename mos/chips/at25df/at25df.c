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

#include "at25df.h"
#include "at25df_pins.h"
#include <hil/gpio.h>
#include <hil/usart.h>
#include <hil/udelay.h>
//#include <msp430/msp430_int.h>

#define AT25DF_MANUFACTURER_INFO_COMMAND 0x9f
#define AT25DF_DEEP_POWER_DOWN_COMMAND 0xB9
#define AT25DF_RESUME_COMMAND 0xAB
#define AT25DF_STATUS_READ_COMMAND 0x05
#define AT25DF_STATUS_WRITE_COMMAND 0x01
#define AT25DF_CHIP_ERASE_COMMAND 0xc7
#define AT25DF_BLOCK_ERASE_4KB 0x20
#define AT25DF_BLOCK_ERASE_32KB 0x52
#define AT25DF_BLOCK_ERASE_64KB 0xD8
#define AT25DF_WRITE_ENABLE_COMMAND 0x06
#define AT25DF_WRITE_DISABLE_COMMAND 0x04
#define AT25DF_READ_ARRAY_FAST_COMMAND 0x0b
#define AT25DF_READ_ARRAY_COMMAND 0x03 // slower
#define AT25DF_BYTE_PROGRAM 0x02

#define AT25DF_STATUS_DONE_MASK 0x01

#define TIME_RDPD  30  // usec to standby mode
#define TIME_EDPD  1  // usec to deep power down

// Shortcuts
#define AT25DF_WR_BYTE(b) \
    spiWriteByte(AT25DF_SPI_ID, b)
#define AT25DF_RD_BYTE() \
    spiReadByte(AT25DF_SPI_ID)
#define AT25DF_WR_MANY(buf, len) \
    spiWrite(AT25DF_SPI_ID, buf, len)
#define AT25DF_RD_MANY(buf, len) \
    spiRead(AT25DF_SPI_ID, buf, len)

#define AT25DF_WAIT_UNTIL_DONE()        \
    while (readStatusRegister() & AT25DF_STATUS_DONE_MASK)

// Send address, 24 bits, most significant bits first
#define AT25DF_TX_ADDR(addr) do {                           \
        AT25DF_WR_BYTE((uint8_t) ((addr >> 16) & 0xFF));    \
        AT25DF_WR_BYTE(((uint8_t) (addr >>  8) & 0xFF));    \
        AT25DF_WR_BYTE((uint8_t) (addr & 0xFF));            \
    } while (0)

#define commandAndRead(cmd, x) do {                 \
        AT25DF_SPI_ENABLE();                        \
        AT25DF_WR_BYTE(cmd);                        \
        x = AT25DF_RD_BYTE();                       \
        AT25DF_SPI_DISABLE();                       \
    } while (0)

#define commandAndWrite(cmd, x) do {                \
        AT25DF_SPI_ENABLE();                        \
        AT25DF_WR_BYTE(cmd);                        \
        AT25DF_WR_BYTE(x);                          \
        AT25DF_SPI_DISABLE();                       \
    } while (0)

#define command(cmd) do {                           \
        AT25DF_SPI_ENABLE();                        \
        AT25DF_WR_BYTE(cmd);                        \
        AT25DF_SPI_DISABLE();                       \
    } while (0)

static inline void writeEnableAndUnprotect()
{
    commandAndWrite(AT25DF_STATUS_WRITE_COMMAND, 0x0);
    command(AT25DF_WRITE_ENABLE_COMMAND);
}

static inline void writeDisable()
{
    command(AT25DF_WRITE_DISABLE_COMMAND);
}

static inline uint8_t readStatusRegister()
{
    uint8_t statusReg;
    commandAndRead(AT25DF_STATUS_READ_COMMAND, statusReg);
    return statusReg;
}

void at25df_sleep(void)
{
    // enter deep sleep mode
    command(AT25DF_DEEP_POWER_DOWN_COMMAND);
    udelay(TIME_EDPD);
}

void at25df_wake(void)
{
    // exit deep sleep mode
    command(AT25DF_RESUME_COMMAND);
    udelay(TIME_RDPD);
}

void at25df_init(void) {
    spiBusInit(AT25DF_SPI_ID, SPI_MODE_MASTER);

    pinAsOutput(AT25DF_HOLD_PORT, AT25DF_HOLD_PIN);
    pinAsOutput(AT25DF_CS_PORT, AT25DF_CS_PIN);
    pinSet(AT25DF_CS_PORT, AT25DF_CS_PIN);
    pinSet(AT25DF_HOLD_PORT, AT25DF_HOLD_PIN);

    // radio
    pinAsOutput(4, 2);
    pinSet(4, 2);

    // not needed
    // pinAsOutput(AT25DF_WP_PORT, AT25DF_WP_PIN);
    // pinSet(AT25DF_WP_PORT, AT25DF_WP_PIN);

    writeEnableAndUnprotect();

    at25df_sleep();
}

static void at25df_pageProgram(uint32_t addr, const uint8_t *buffer, uint16_t len)
{
    // Handle_t h;
    // ATOMIC_START(h);
    usartBusy[AT25DF_SPI_ID] = true;
    AT25DF_WAIT_UNTIL_DONE();
    writeEnableAndUnprotect();
    AT25DF_SPI_ENABLE();
    AT25DF_WR_BYTE(AT25DF_BYTE_PROGRAM);
    AT25DF_TX_ADDR(addr);
    AT25DF_WR_MANY(buffer, len);
    AT25DF_SPI_DISABLE();
    writeDisable();
    usartBusy[AT25DF_SPI_ID] = false;
//    ATOMIC_END(h);
}

void at25df_bulkErase(void)
{
    // Handle_t h;
    // ATOMIC_START(h);
    usartBusy[AT25DF_SPI_ID] = true;
    AT25DF_WAIT_UNTIL_DONE();
    command(AT25DF_CHIP_ERASE_COMMAND);
    usartBusy[AT25DF_SPI_ID] = false;
//    ATOMIC_END(h);
}

void at25df_eraseSector(uint32_t addr)
{
    // Handle_t h;
    // ATOMIC_START(h);
    usartBusy[AT25DF_SPI_ID] = true;
    AT25DF_WAIT_UNTIL_DONE();
    writeEnableAndUnprotect();
    AT25DF_SPI_ENABLE();
    AT25DF_WR_BYTE(AT25DF_BLOCK_ERASE_4KB);
    AT25DF_TX_ADDR(addr); // the lower bits will be automatically ignored
    AT25DF_SPI_DISABLE();
    usartBusy[AT25DF_SPI_ID] = false;
//    ATOMIC_END(h);
}

void at25df_read(uint32_t addr, void* buffer, uint16_t len)
{
    // Handle_t h;
    // ATOMIC_START(h);
    usartBusy[AT25DF_SPI_ID] = true;
    AT25DF_WAIT_UNTIL_DONE();
    AT25DF_SPI_ENABLE();
    AT25DF_WR_BYTE(AT25DF_READ_ARRAY_FAST_COMMAND);
    AT25DF_TX_ADDR(addr);
    AT25DF_WR_BYTE(0); // dummy byte, needed for the fast mode
    AT25DF_RD_MANY(buffer, len);
    AT25DF_SPI_DISABLE();
    usartBusy[AT25DF_SPI_ID] = false;
//    ATOMIC_END(h);
}

// Write len bytes (len <= 256) to flash at addr
void at25df_write(uint32_t addr, const void *buf, uint16_t len)
{
    if (len > AT25DF_PAGE_SIZE) len = AT25DF_PAGE_SIZE;

    const uint8_t *bufCopy = (uint8_t *)buf;

    // if buffer tail goes over page boundaries, split to two writes
    // each page is 256 byte long
    uint16_t pageOffset = (uint16_t) (addr & 0xff);
    if (pageOffset + len > AT25DF_PAGE_SIZE) {
        uint16_t safeLen = AT25DF_PAGE_SIZE - pageOffset;
        at25df_pageProgram(addr, bufCopy, safeLen);
        // write remaining bytes in the beginning of the next page
        addr = (addr & 0xffffff00) + AT25DF_PAGE_SIZE;
        len -= safeLen;
        bufCopy += safeLen;
    }

    at25df_pageProgram(addr, bufCopy, len);
}
