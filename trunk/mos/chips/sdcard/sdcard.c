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

#include "sdcard.h"
#include "sdcard_pins.h"
#include "sdcard_const.h"
#include <hil/udelay.h>
#include <hil/timers.h>
#include <hil/blink.h>
#include <lib/assert.h>
#include <string.h>

#if DEBUG
#define SDCARD_DEBUG 1
#endif

#if SDCARD_DEBUG
#include <lib/dprint.h>
#define SPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define SPRINTF(...) do {} while (0)
#endif

// timeouts in milliseconds
#define INIT_TIMEOUT  2000
#define ERASE_TIMEOUT 10000
#define READ_TIMEOUT  300
#define WRITE_TIMEOUT 600

// same timeouts in timer A ticks
#define INIT_TIMEOUT_TICKS  (2 * TIMER_SECOND)

// timer A feeds from from ACLK (32'768 Hz)
#define TIMER_SECOND ACLK_SPEED 
#define TIMER_100_MS (ACLK_SPEED / 10 + 1)

// card types
enum {
    SD_CARD_TYPE_NONE,
    SD_CARD_TYPE_SD1,
    SD_CARD_TYPE_SD2,
    SD_CARD_TYPE_SDHC,
};

static bool initOk;

static uint8_t cardType;

static uint8_t cacheBuffer[SDCARD_SECTOR_SIZE];
static uint32_t cacheAddress;
static bool cacheChanged;

#define IN_SECTOR(address, sectorStartAdddress)                 \
    (((address) >= (sectorStartAdddress)) &&                    \
            (address) < ((sectorStartAdddress) + SDCARD_SECTOR_SIZE))

// Shortcuts
#if SDCARD_SPI_ID == 0

#warning using SadMote

// Chip Select
#define MMC_CS_PxOUT      P3OUT
#define MMC_CS_PxDIR      P3DIR
#define MMC_CS            0x01

#define SPI_PxSEL         P3SEL
#define SPI_PxDIR         P3DIR
#define SPI_PxIN          P3IN
#define SPI_PxOUT         P3OUT
#define SPI_SIMO          0x02
#define SPI_SOMI          0x04
#define SPI_UCLK          0x08

#define MMC_PxSEL         SPI_PxSEL
#define MMC_PxDIR         SPI_PxDIR
#define MMC_PxIN          SPI_PxIN
#define MMC_PxOUT         SPI_PxOUT      
#define MMC_SIMO          SPI_SIMO
#define MMC_SOMI          SPI_SOMI
#define MMC_UCLK          SPI_UCLK

void halSPISetup(void)
{
    // Init Port for MMC (default high)
    MMC_PxOUT |= MMC_SIMO + MMC_UCLK;
    MMC_PxDIR |= MMC_SIMO + MMC_UCLK;

    // Chip Select
    MMC_CS_PxOUT |= MMC_CS;
    MMC_CS_PxDIR |= MMC_CS;

    UCTL0 = SWRST;
    UCTL0 |= CHAR + SYNC + MST;               // 8-bit SPI Master **SWRST**
    UTCTL0 = CKPL + SSEL1 + SSEL0 + STC;      // SMCLK, 3-pin mode
    U0BR0 = 0x08;                             // UCLK/8
    U0BR1 = 0x00;                             // 0
    UMCTL0 = 0x00;                            // No modulation
    ME1 |= USPIE0;                            // Enable USART0 SPI mode
    UCTL0 &= ~SWRST;                          // Initialize USART state machine

    // Enable secondary function
#ifndef USE_SOFT_SPI
    MMC_PxSEL |= MMC_SIMO + MMC_SOMI + MMC_UCLK;
#endif
}

#else

#warning using CarMote

// Chip Select
#define MMC_CS_PxOUT      P5OUT
#define MMC_CS_PxDIR      P5DIR
#define MMC_CS            0x01

// Card Detect
#define MMC_CD_PxIN       P5IN
#define MMC_CD_PxDIR      P5DIR
#define MMC_CD            0x40

#define SPI_PxSEL         P5SEL
#define SPI_PxDIR         P5DIR
#define SPI_PxIN          P5IN
#define SPI_PxOUT         P5OUT
#define SPI_SIMO          0x02
#define SPI_SOMI          0x04
#define SPI_UCLK          0x08

#define MMC_PxSEL         SPI_PxSEL
#define MMC_PxDIR         SPI_PxDIR
#define MMC_PxIN          SPI_PxIN
#define MMC_PxOUT         SPI_PxOUT      
#define MMC_SIMO          SPI_SIMO
#define MMC_SOMI          SPI_SOMI
#define MMC_UCLK          SPI_UCLK

void halSPISetup(void)
{
    // Init Port for MMC (default high)
    MMC_PxOUT |= MMC_SIMO + MMC_UCLK;
    MMC_PxDIR |= MMC_SIMO + MMC_UCLK;

    // Chip Select
    MMC_CS_PxOUT |= MMC_CS;
    MMC_CS_PxDIR |= MMC_CS;

    // Card Detect
    MMC_CD_PxDIR &=  ~MMC_CD;

    UCTL1 = CHAR + SYNC + MM + SWRST;         // 8-bit SPI Master **SWRST**
    UTCTL1 = CKPL + SSEL1 + SSEL0 + STC;      // SMCLK, 3-pin mode
    UBR01 = 0x02;                             // UCLK/2
    UBR11 = 0x00;                             // 0
    UMCTL1 = 0x00;                            // No modulation
    ME2 |= USPIE1;                            // Enable USART1 SPI mode
    UCTL1 &= ~SWRST;                          // Initialize USART state machine

    // Enable secondary function
#ifndef USE_SOFT_SPI
    MMC_PxSEL |= MMC_SIMO + MMC_SOMI + MMC_UCLK;
#endif
}

#endif

#define SDCARD_WR_BYTE(b)                       \
    spiExchByte(SDCARD_SPI_ID, b)
#define SDCARD_RD_BYTE()                        \
    spiExchByte(SDCARD_SPI_ID, 0xff)
#define SDCARD_WR_LONG(l)                       \
    spiExchByte(SDCARD_SPI_ID, l >> 24);        \
    spiExchByte(SDCARD_SPI_ID, l >> 16);        \
    spiExchByte(SDCARD_SPI_ID, l >> 8);         \
    spiExchByte(SDCARD_SPI_ID, l)
#define SDCARD_WR_MANY(buf, len)                \
    spiWrite(SDCARD_SPI_ID, buf, len)
#define SDCARD_RD_MANY(buf, len)                \
    spiRead(SDCARD_SPI_ID, buf, len)

static bool sdcardReadData(void* buffer, uint16_t len);

static uint16_t timerTicksRead(void)
{
    uint16_t t1, t2;
    do {
        t1 = TAR;
        t2 = TAR;
    } while (t1 != t2);
    return t1;
}

#define BUSYWAIT_UNTIL_NOINTS(cond, maxTime, ok)                        \
    do {                                                                \
        uint16_t endTime = timerTicksRead() + maxTime;                  \
        ok = false;                                                     \
        do {                                                            \
            if (cond) {                                                 \
                ok = true;                                              \
                break;                                                  \
            }                                                           \
        } while (timeAfter16(endTime, timerTicksRead()));               \
    } while (0)

#define BUSYWAIT_UNTIL(cond, maxTime, ok)                               \
    do {                                                                \
        uint32_t endTime = getRealTime() + maxTime;                     \
        ok = false;                                                     \
        do {                                                            \
            if (cond) {                                                 \
                ok = true;                                              \
                break;                                                  \
            }                                                           \
        } while (timeAfter32(endTime, getRealTime()));                  \
    } while (0)

static bool waitCardNotBusy(uint16_t timeout)
{
    bool result;
    BUSYWAIT_UNTIL(SDCARD_RD_BYTE() == 0xff, timeout, result);
    return result;
}

static bool waitCardNotBusyNoints(uint16_t timeout)
{
    bool result;
    BUSYWAIT_UNTIL_NOINTS(SDCARD_RD_BYTE() == 0xff, timeout, result);
    return result;
}

static uint8_t sdcardCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
//    SPRINTF("sdcardCommand\n");
        
    SDCARD_SPI_ENABLE();

    // wait up to 300 ms if busy
    waitCardNotBusyNoints(3 * TIMER_100_MS);

    // send command
    SDCARD_WR_BYTE(cmd | 0x40);

    SDCARD_WR_LONG(arg);

    // send CRC
    SDCARD_WR_BYTE(crc);

    // skip stuff byte for stop read
    if (cmd == CMD_STOP_TRANSMISSION) SDCARD_RD_BYTE();

    // wait for response
    uint8_t i;
    uint8_t status;
    for (i = 0; i != 0xFF; i++) {
        status = SDCARD_RD_BYTE();
        // SPRINTF("status=%x\n", status);
        // if (!(status & 0x80)) break;
        if (status == 0 || status == 1) break;
    }
    
    // do not disable SPI here!
    return status;
}

uint8_t sdcardAppCommand(uint8_t cmd, uint32_t arg)
{
    sdcardCommand(CMD_APP_CMD, 0, 0xff);
    return sdcardCommand(cmd, arg, 0xff);
}

void sdcardSleep(void)
{
    // enter deep sleep mode
}

void sdcardWake(void)
{
    // exit deep sleep mode
}

bool sdcardInit(void)
{
    uint8_t i;
    uint8_t status;
    uint32_t arg;
    bool ok;

    SPRINTF("sdcardInit\n");

    initOk = false;

    // disable radio
    //pinAsOutput(4, 2);
    //pinSet(4, 2);

    //msp430USARTInitSPI(SDCARD_SPI_ID, SPI_MODE_MASTER);
    //spiBusInit(SDCARD_SPI_ID, SPI_MODE_MASTER);
    halSPISetup();

    SPRINTF("1\n");
    // mdelay(1);

    // must supply min of 74 clock cycles with CS high.
    SDCARD_SPI_DISABLE();
    for (i = 0; i < 10; i++) SDCARD_RD_BYTE();

    SPRINTF("2\n");
    // mdelay(1);

    BUSYWAIT_UNTIL_NOINTS(sdcardCommand(CMD_GO_IDLE_STATE, 0, 0x95) == R1_IDLE_STATE, INIT_TIMEOUT_TICKS / 2, ok);
    if (!ok) {
        BUSYWAIT_UNTIL_NOINTS(sdcardCommand(CMD_GO_IDLE_STATE, 0, 0x95) == R1_IDLE_STATE, INIT_TIMEOUT_TICKS / 2, ok);
        if (!ok) {
            goto fail;
        }
    }

    uint8_t response = 0x1;
    while (response == 0x01) {
        SDCARD_SPI_DISABLE();
        SDCARD_WR_BYTE(0xff);
        response = sdcardCommand(CMD_SEND_OP_COND, 0x00, 0xff);
    }
    SDCARD_SPI_DISABLE();
    SDCARD_WR_BYTE(0xff);

#if PLATFORM_SM3
    SPRINTF("3\n");

    // check SD version
    if ((sdcardCommand(CMD_SEND_IF_COND, 0x1AA, 0x87) & R1_ILLEGAL_COMMAND)) {
        cardType = SD_CARD_TYPE_SD1;
    } else {
        // only need last byte of r7 response
        for (i = 0; i < 4; i++) {
            status = SDCARD_RD_BYTE();
            // SPRINTF("status = %d\n", status);
        }
        if (status != 0xAA) {
            goto fail;
        }
        cardType = SD_CARD_TYPE_SD2;
    }
    SPRINTF("4: cardType=%d\n", cardType);
    // initialize card and send host supports SDHC if SD2
    arg = (cardType == SD_CARD_TYPE_SD2 ? 0x40000000 : 0);

    BUSYWAIT_UNTIL_NOINTS(sdcardAppCommand(ACMD_SD_SEND_OP_COMD, arg) == R1_READY_STATE,
            INIT_TIMEOUT_TICKS / 2, ok);
    if (!ok) {
        BUSYWAIT_UNTIL_NOINTS(sdcardAppCommand(ACMD_SD_SEND_OP_COMD, arg) == R1_READY_STATE,
                INIT_TIMEOUT_TICKS / 2, ok);
        if (!ok) {
            goto fail;
        }
    }

    SPRINTF("5\n");

    // if SD2 read OCR register to check for SDHC card
    if (cardType == SD_CARD_TYPE_SD2) {
        if (sdcardCommand(CMD_READ_OCR, 0, 0xff)) {
            goto fail;
        }
        if ((SDCARD_RD_BYTE() & 0xC0) == 0xC0) cardType = SD_CARD_TYPE_SDHC;
        // discard rest of ocr - contains allowed voltage range
        for (i = 0; i < 3; i++) SDCARD_RD_BYTE();
    }
    SDCARD_SPI_DISABLE();
    SPRINTF("sdcard init OK\n");

#endif // PLATFORM_SM3

    // make cache valid
    mdelay(1);
    sdcardReadBlock(cacheAddress, cacheBuffer);
    SPRINTF("cache read OK\n");
    return (initOk = true);

 fail:
    SDCARD_SPI_DISABLE();
    SPRINTF("sdcard init failed\n");
    return (initOk = false);
}

static bool sdcardReadRegister(uint8_t cmd, void* buf)
{
    uint8_t* dst = buf;
    if (sdcardCommand(cmd, 0, 0xff)) {
        goto fail;
    }
    return sdcardReadData(dst, 16);

  fail:
    SDCARD_SPI_DISABLE();
    return false;
}

static inline bool sdcardReadCSD(csd_t* csd)
{
    return sdcardReadRegister(CMD_SEND_CSD, csd);
}

static bool sdcardEraseRange(uint32_t firstBlock, uint32_t lastBlock)
{
    csd_t csd;
    if (!sdcardReadCSD(&csd)) goto fail;
    // check for single block erase
    if (!csd.v1.erase_blk_en) {
        // erase size mask
        uint8_t m = (csd.v1.sector_size_high << 1) | csd.v1.sector_size_low;
        if ((firstBlock & m) != 0 || ((lastBlock + 1) & m) != 0) {
            // error card can't erase specified area
            goto fail;
        }
    }
    if (cardType != SD_CARD_TYPE_SDHC) {
        // use addresses
        firstBlock <<= 9;
        lastBlock <<= 9;
    }
    if (sdcardCommand(CMD_ERASE_WR_BLK_START, firstBlock, 0xff)
            || sdcardCommand(CMD_ERASE_WR_BLK_END, lastBlock, 0xff)
            || sdcardCommand(CMD_ERASE, 0, 0xff)) {
        goto fail;
    }
    if (!waitCardNotBusy(ERASE_TIMEOUT)) {
        goto fail;
    }

    SDCARD_SPI_DISABLE();
    return true;

 fail:
    SDCARD_SPI_DISABLE();
    return false;
}

void sdcardBulkErase(void)
{
    if (!initOk) return;
    sdcardEraseRange(0, SDCARD_SECTOR_COUNT - 1);
}

void sdcardEraseSector(uint32_t address)
{
    if (!initOk) return;
    uint32_t block = address >> 9;
    sdcardEraseRange(block, block);
}

bool sdcardReadBlock(uint32_t address, void* buffer)
{
    //PRINTF("sdcardReadBlock at %lu\n", address);

    //  if SDHC card: use block number instead of address
    if (cardType == SD_CARD_TYPE_SDHC) address >>= 9;

    if (sdcardCommand(CMD_READ_SINGLE_BLOCK, address, 0xff) != 0) {
        goto fail;
    }

    return sdcardReadData(buffer, SDCARD_SECTOR_SIZE);

  fail:
    SDCARD_SPI_DISABLE();
    return false;
}

static bool sdcardReadData(void* buffer, uint16_t len)
{
    uint8_t status;
    bool ok;

    // wait for start block token
    BUSYWAIT_UNTIL((status = SDCARD_RD_BYTE()) != 0xFF, READ_TIMEOUT, ok);
    if (!ok) {
        goto fail;
    }

    if (status != DATA_START_BLOCK) {
        goto fail;
    }

    // transfer data
    SDCARD_RD_MANY(buffer, len);

    // discard CRC
    SDCARD_RD_BYTE();
    SDCARD_RD_BYTE();

    SDCARD_SPI_DISABLE();
    // SPRINTF("sdcardReadData ok\n");
    return true;

  fail:
    SDCARD_SPI_DISABLE();
    return false;
}

static bool sdcardWriteData(uint8_t token, const uint8_t* data)
{
    SDCARD_WR_BYTE(token);
    SDCARD_WR_MANY(data, 512);

    // dummy crc
    SDCARD_WR_BYTE(0xff);
    SDCARD_WR_BYTE(0xff);

    uint8_t status = SDCARD_RD_BYTE();
    if ((status & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
        goto fail;
    }
    return true;

  fail:
    SDCARD_SPI_DISABLE();
    return false;
}

// Write len bytes (len == 512) to card at address
bool sdcardWriteBlock(uint32_t address, const void *buf)
{
    // PRINTF("sdcardWriteBlock at %lu\n", address);

    //  if SDHC card: use block number instead of address
    if (cardType == SD_CARD_TYPE_SDHC) address >>= 9;

    if (sdcardCommand(CMD_WRITE_BLOCK, address, 0xff)) {
        goto fail;
    }
    if (!sdcardWriteData(DATA_START_BLOCK, buf)) goto fail;

    // wait for flash programming to complete
    if (!waitCardNotBusy(WRITE_TIMEOUT)) {
        goto fail;
    }
    // response is r2 so get and check two bytes for nonzero
    if (sdcardCommand(CMD_SEND_STATUS, 0, 0xff) || SDCARD_RD_BYTE()) {
        goto fail;
    }
    SDCARD_SPI_DISABLE();
    return true;

 fail:
    SDCARD_SPI_DISABLE();
    return false;
}
 
// -----------------------------------------------------------------

void sdcardFlush(void)
{
    sdcardWriteBlock(cacheAddress, cacheBuffer);
    cacheChanged = false;
}

static inline void takeFromCache(void* buffer, uint16_t len, uint16_t offset)
{
    //ASSERT(offset < SDCARD_SECTOR_SIZE);
    //ASSERT(cacheAddress % SDCARD_SECTOR_SIZE == 0);
    //PRINTF("sd card take from cache %u bytes at %lu\n", len, cacheAddress + offset);

    memcpy(buffer, cacheBuffer + offset, len);
}

// Read len bytes (len <= 512) at address
static void sdcardReadInBlock(uint32_t address, void* buffer, uint16_t len)
{
    // PRINTF("sdcardReadInBlock %u bytes at %lu\n", len, address);

    if (IN_SECTOR(address, cacheAddress)) {
        takeFromCache(buffer, len, address & (SDCARD_SECTOR_SIZE - 1));
        return;
    }
    // flush cache here too
    if (cacheChanged) {
        sdcardWriteBlock(cacheAddress, cacheBuffer);
        cacheChanged = false;
    }

    cacheAddress = address & 0xfffffe00;
    sdcardReadBlock(cacheAddress, cacheBuffer);
    takeFromCache(buffer, len, address & (SDCARD_SECTOR_SIZE - 1));

    // debugHexdump(buffer, len);
}

// Read len bytes (len <= 512) at address
void sdcardRead(uint32_t address, void* buffer, uint16_t len)
{
    if (!initOk) return;
    // PRINTF("sdcardRead %u bytes at %lu\n", len, address);

    uint8_t *buf = (uint8_t *)buffer;
    uint16_t pageOffset = (uint16_t) (address & (SDCARD_SECTOR_SIZE - 1));
    if (pageOffset + len > SDCARD_SECTOR_SIZE) {
        uint16_t safeLen = SDCARD_SECTOR_SIZE - pageOffset;
        sdcardReadInBlock(address, buf, safeLen);
        // write remaining bytes in the beginning of the next page
        address += safeLen;
        len -= safeLen;
        buf += safeLen;
    }
    sdcardReadInBlock(address, buf, len);
}


static inline void putInCache(const void* buffer, uint16_t len, uint16_t offset)
{
    //ASSERT(offset < SDCARD_SECTOR_SIZE);
    //ASSERT(cacheAddress % SDCARD_SECTOR_SIZE == 0);
    //PRINTF("sd card put in cache %u bytes at %lu\n", len, cacheAddress + offset);

    memcpy(cacheBuffer + offset, buffer, len);
    cacheChanged = true;
}

static void sdcardWriteInBlock(uint32_t address, const void* buffer, uint16_t len)
{
    // PRINTF("sdcardWriteInBlock %u bytes at %lu\n", len, address);
    // debugHexdump(buffer, len);

    if (IN_SECTOR(address, cacheAddress)) {
        putInCache(buffer, len, address & (SDCARD_SECTOR_SIZE - 1));
        return;
    }
    if (cacheChanged) {
        sdcardWriteBlock(cacheAddress, cacheBuffer);
        cacheChanged = false;
    }
    cacheAddress = address & 0xfffffe00;
    sdcardReadBlock(cacheAddress, cacheBuffer);
    putInCache(buffer, len, address & (SDCARD_SECTOR_SIZE - 1));
}

// Write len bytes (len <= 512) at address
void sdcardWrite(uint32_t address, const void *buffer, uint16_t len)
{
    if (!initOk) return;

    // PRINTF("sdcardWrite %u bytes at %lu\n", len, address);

    const uint8_t *buf = (const uint8_t *)buffer;
    uint16_t pageOffset = (uint16_t) (address & (SDCARD_SECTOR_SIZE - 1));
    if (pageOffset + len > SDCARD_SECTOR_SIZE) {
        uint16_t safeLen = SDCARD_SECTOR_SIZE - pageOffset;
        sdcardWriteInBlock(address, buf, safeLen);
        // write remaining bytes in the beginning of the next page
        address += safeLen;
        len -= safeLen;
        buf += safeLen;
    }
    sdcardWriteInBlock(address, buf, len);
}
