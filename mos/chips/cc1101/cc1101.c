/*
 * Copyright (c) 2011, Institute of Electronics and Computer Science
 * All rights reserved.
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
 *
 * cc1101.c -- CC1101 radio driver
 *
 * TODO: Replace busy-wait loops with something better
 * TODO: This module monopolizes a port interrupt vector
 */

#include <digital.h>
#include <errors.h>
#include <spi.h>
#include <cc1101_pins.h>
#include <kernel/threads/threads.h>

#include "cc1101.h"


//
// SPI access shortcuts
//

static inline uint8_t readByte(void)
{
    return spiReadByte(CC1101_SPI_ID);
}

static inline void writeByte(uint8_t data)
{
    spiWriteByte(CC1101_SPI_ID, data);
}

static inline uint8_t exchByte(uint8_t data)
{
    return spiExchByte(CC1101_SPI_ID, data);
}

static inline void chipSelect(void)
{
    pinClear(CC1101_CSN_PORT, CC1101_CSN_PIN);
}

static inline void chipRelease(void)
{
    pinSet(CC1101_CSN_PORT, CC1101_CSN_PIN);
}


//
// SPI header byte
//

#define HEADER_RW    (1 << 7)
#define HEADER_BURST (1 << 6)


//
// Radio registers and strobes
//

enum reg {
    // Configuration registers
    REG_IOCFG2     = 0x00,
    REG_IOCFG1     = 0x01,
    REG_IOCFG0     = 0x02,
    REG_FIFOTHR    = 0x03,
    REG_SYNC1      = 0x04,
    REG_SYNC0      = 0x05,
    REG_PKTLEN     = 0x06,
    REG_PKTCTRL1   = 0x07,
    REG_PKTCTRL0   = 0x08,
    REG_ADDR       = 0x09,
    REG_CHANNR     = 0x0A,
    REG_FSCTRL1    = 0x0B,
    REG_FSCTRL0    = 0x0C,
    REG_FREQ2      = 0x0D,
    REG_FREQ1      = 0x0E,
    REG_FREQ0      = 0x0F,
    REG_MDMCFG4    = 0x10,
    REG_MDMCFG3    = 0x11,
    REG_MDMCFG2    = 0x12,
    REG_MDMCFG1    = 0x13,
    REG_MDMCFG0    = 0x14,
    REG_DEVIATN    = 0x15,
    REG_MCSM2      = 0x16,
    REG_MCSM1      = 0x17,
    REG_MCSM0      = 0x18,
    REG_FOCCFG     = 0x19,
    REG_BSCFG      = 0x1A,
    REG_AGCCTRL2   = 0x1B,
    REG_AGCCTRL1   = 0x1C,
    REG_AGCCTRL0   = 0x1D,
    REG_WOREVT1    = 0x1E,
    REG_WOREVT0    = 0x1F,
    REG_WORCTRL    = 0x20,
    REG_FREND1     = 0x21,
    REG_FREND0     = 0x22,
    REG_FSCAL3     = 0x23,
    REG_FSCAL2     = 0x24,
    REG_FSCAL1     = 0x25,
    REG_FSCAL0     = 0x26,
    REG_RCCTRL1    = 0x27,
    REG_RCCTRL0    = 0x28,
    REG_FSTEST     = 0x29,
    REG_PTEST      = 0x2A,
    REG_AGCTEST    = 0x2B,
    REG_TEST2      = 0x2C,
    REG_TEST1      = 0x2D,
    REG_TEST0      = 0x2E,

    // Command strobes
    STROBE_SRES    = 0x30,
    STROBE_SFSTXON = 0x31,
    STROBE_SXOFF   = 0x32,
    STROBE_SCAL    = 0x33,
    STROBE_SRX     = 0x34,
    STROBE_STX     = 0x35,
    STROBE_SIDLE   = 0x36,
    STROBE_SWOR    = 0x38,
    STROBE_SPWD    = 0x39,
    STROBE_SFRX    = 0x3A,
    STROBE_SFTX    = 0x3B,
    STROBE_SWORRST = 0x3C,
    STROBE_SNOP    = 0x3D,

    // Status registers. The burst bit is used to tell them apart from strobes
    // with the same number.
#define STREG(n) ((n) | HEADER_BURST)
    REG_PARTNUM    = STREG(0x30),
    REG_VERSION    = STREG(0x31),
    REG_FREQEST    = STREG(0x32),
    REG_LQI        = STREG(0x33),
    REG_RSSI       = STREG(0x34),
    REG_MARCSTAT   = STREG(0x35),
    REG_WORTIME1   = STREG(0x36),
    REG_WORTIME2   = STREG(0x37),
    REG_PKTSTATUS  = STREG(0x38),
    REG_VCO_VC_DAC = STREG(0x39),
    REG_TXBYTES    = STREG(0x3A),
    REG_RXBYTES    = STREG(0x3B),
    REG_RCCTRL1_STATUS = STREG(0x3C),
    REG_RCCTRL0_STATUS = STREG(0x3D),
#undef STREG

    // Multibyte registers
    REG_PATABLE    = 0x3E,
    REG_FIFO       = 0x3F, // RX/TX FIFO
};

typedef enum reg reg_t;

#define LQI_CRC_OK (1 << 7) // 7th bit of the LQI register


//
// Radio status byte
//

#define STATUS_READY (1 << 7)

typedef enum state {
    STATE_IDLE         = 0,
    STATE_RX           = 1,
    STATE_TX           = 2,
    STATE_FSTXON       = 3,
    STATE_CALIBRATE    = 4,
    STATE_SETTLING     = 5,
    STATE_RX_OVERFLOW  = 6,
    STATE_TX_UNDERFLOW = 7
} state_t;

static inline state_t getState(uint8_t status)
{
    return (status >> 4) & 0x7;
}

static inline uint8_t getFIFOAvail(uint8_t status)
{
    return status & 0xf;
}


//
// Radio register access
//

static inline void strobe(reg_t num)
{
    writeByte(num);
}

static inline uint8_t getreg(reg_t reg)
{
    strobe(reg | HEADER_RW);
    return readByte();
}

static inline void setreg(reg_t reg, uint8_t data)
{
    strobe(reg);
    writeByte(data);
}

static void burstRead(reg_t start, void *buf, uint8_t len)
{
    strobe(start | HEADER_BURST | HEADER_RW);
    spiRead(CC1101_SPI_ID, buf, len);

    chipRelease(); // End burst access
    chipSelect();
}

static void burstWrite(reg_t start, const void *data, uint8_t len)
{
    strobe(start | HEADER_BURST);
    spiWrite(CC1101_SPI_ID, data, len);

    chipRelease(); // End burst access
    chipSelect();
}


//
// Status handling
//

static inline uint8_t status(void)
{
    return exchByte(STROBE_SNOP);
}

static inline void waitForReady(void)
{
    //
    // Sometimes we need to wait until the SO pin on the radio chip goes low,
    // which indicates that the radio crystal is up (p. 29). But the SO pin is
    // used by the SPI module. This code temporarily steals the pin, then
    // reverts it to its original function. Improvements welcome.
    //
    pinAsInput(CC1101_SO_PORT, CC1101_SO_PIN);
    while (pinRead(CC1101_SO_PORT, CC1101_SO_PIN) == 1);
    pinAsFunction(CC1101_SO_PORT, CC1101_SO_PIN);
}

static inline void waitForListen(void)
{
    //
    // Wait until the chip is not in the TX state, which means that no outgoing
    // packets are pending
    //

    while (getState(status()) == STATE_TX);
}


//
// Radio initialization
//

static const uint8_t rfConfig[] = {
    // Generated by SmartRF Studio 7 v1.10.3
    // Data rate: 10 kBaud, Optimized for current consumption
    [REG_FIFOTHR]  = 0x47, // RX FIFO and TX FIFO Thresholds
    [REG_FSCTRL1]  = 0x08, // Frequency Synthesizer Control
    [REG_FSCTRL0]  = 0x00, // Frequency Synthesizer Control
    [REG_FREQ2]    = 0x21, // Frequency Control Word, High Byte
    [REG_FREQ1]    = 0x62, // Frequency Control Word, Middle Byte
    [REG_FREQ0]    = 0x76, // Frequency Control Word, Low Byte
    [REG_MDMCFG4]  = 0xC8, // Modem Configuration
    [REG_MDMCFG3]  = 0x93, // Modem Configuration
    [REG_MDMCFG2]  = 0x93, // Modem Configuration
    [REG_MDMCFG1]  = 0x22, // Modem Configuration
    [REG_MDMCFG0]  = 0xF8, // Modem Configuration
    [REG_DEVIATN]  = 0x34, // Modem Deviation Setting
    [REG_MCSM2]    = 0x07, // Main Radio Control State Machine Configuration
    [REG_MCSM0]    = 0x18, // Main Radio Control State Machine Configuration
    [REG_FOCCFG]   = 0x16, // Frequency Offset Compensation Configuration
    [REG_BSCFG]    = 0x6C, // Bit Synchronization Configuration
    [REG_AGCCTRL2] = 0x43, // AGC Control
    [REG_AGCCTRL1] = 0x40, // AGC Control
    [REG_AGCCTRL0] = 0x91, // AGC Control
    [REG_WOREVT1]  = 0x87, // High Byte Event0 Timeout
    [REG_WOREVT0]  = 0x6B, // Low Byte Event0 Timeout
    [REG_WORCTRL]  = 0xFB, // Wake On Radio Control
    [REG_FREND1]   = 0x56, // Front End RX Configuration
    [REG_FREND0]   = 0x10, // Front End TX Configuration
    [REG_FSCAL3]   = 0xE9, // Frequency Synthesizer Calibration
    [REG_FSCAL2]   = 0x2A, // Frequency Synthesizer Calibration
    [REG_FSCAL1]   = 0x00, // Frequency Synthesizer Calibration
    [REG_FSCAL0]   = 0x1F, // Frequency Synthesizer Calibration
    [REG_RCCTRL1]  = 0x41, // RC Oscillator Configuration
    [REG_RCCTRL0]  = 0x00, // RC Oscillator Configuration

    // Custom setup
    [REG_CHANNR]   = CC1101_DEFAULT_CHANNEL,
    [REG_PKTCTRL1] = 0x04, // Append status bytes
    [REG_PKTCTRL0] = 0x45, // CRC enable, variable packet length, data whitening
    [REG_ADDR]     = 0x00, // Node address
    [REG_PKTLEN]   = CC1101_MAX_PACKET_LEN, // Maximum packet length
    [REG_SYNC1]    = MSB(CC1101_SYNC_WORD), // Sync word
    [REG_SYNC0]    = LSB(CC1101_SYNC_WORD),
    [REG_MCSM1]    = 0x33, // Switch to RX after packet transmission, stay in
                           // IDLE after receiving a packet, CCA enable

    // Interrupt setup
    // Generate interrupt at end of packet, if packet is discarded or if FIFO
    // overflows/underflows (0x06), set the two other GDO pins to high
    // impedance (0x2E)
#define IOCFG_VAL(x) (CC1101_GDO_INTR == (x) ? 0x06 : 0x2E)
    [REG_IOCFG2]   = IOCFG_VAL(2),
    [REG_IOCFG1]   = IOCFG_VAL(1),
    [REG_IOCFG0]   = IOCFG_VAL(0),
};

void cc1101InitSpi(void)
{
    spiBusInit(CC1101_SPI_ID, SPI_MODE_MASTER);
    pinAsOutput(CC1101_CSN_PORT, CC1101_CSN_PIN);
    chipRelease();
}

void cc1101Init(void)
{
    Handle_t h;

    ATOMIC_START(h);

    cc1101InitSpi();

    chipSelect();

#if 0
    // Rely on automatic module reset
    waitForReady();
#else
    // Manual module reset
    strobe(STROBE_SRES);
    waitForReady();
#endif

    // Set up registers
    burstWrite(REG_IOCFG2, rfConfig, sizeof(rfConfig));
    setreg(REG_PATABLE, CC1101_TXPOWER);

    // Enable radio interrupts
    pinClearIntFlag(CC1101_INTR_PORT, CC1101_INTR_PIN);
    pinIntFalling(CC1101_INTR_PORT, CC1101_INTR_PIN);
    pinEnableInt(CC1101_INTR_PORT, CC1101_INTR_PIN);

    // Wait for the user to enable RX
    strobe(STROBE_SPWD);

    chipRelease();

    ATOMIC_END(h);
}


//
// On/off functions
//

void cc1101On(void)
{
    Handle_t h;

    ATOMIC_START(h);
    chipSelect();       // Wakes the radio up
    waitForReady();
    strobe(STROBE_SRX); // Go into receive mode
    chipRelease();
    ATOMIC_END(h);
}

void cc1101Off(void)
{
    Handle_t h;

    ATOMIC_START(h);
    chipSelect();
    waitForListen();
    strobe(STROBE_SIDLE); // Might drop a partially received packet
    strobe(STROBE_SPWD);  // Will also flush FIFOs
    chipRelease();
    ATOMIC_END(h);
}


//
// Configuration functions
//

static cc1101Callback_t callback;

void cc1101SetChannel(int channel)
{
    chipSelect();
    strobe(STROBE_SIDLE);
    setreg(REG_CHANNR, channel); // Can only be updated in IDLE state
    strobe(STROBE_SRX);
    chipRelease();
}

void cc1101SetTxPower(uint8_t power)
{
    chipSelect();
    setreg(REG_PATABLE, power);
    strobe(STROBE_SNOP); // Reset PATABLE pointer
    chipRelease();
}

void cc1101SetAddress(uint8_t addr)
{
    chipSelect();
    setreg(REG_ADDR, addr);
    chipRelease();
}

void cc1101SetRecvCallback(cc1101Callback_t func)
{
    callback = func;
}


//
// Signal analysis functions
//

static int8_t  lastRSSI;
static uint8_t lastLQI;

int8_t cc1101GetRSSI(void)
{
    int8_t res;

    chipSelect();
    res = getreg(REG_RSSI);
    chipRelease();

    return res;
}

int8_t cc1101GetLastRSSI(void)
{
    return lastRSSI;
}

uint8_t cc1101GetLastLQI(void)
{
    return lastLQI;
}


//
// Send/receive functions
//

int8_t cc1101Send(const uint8_t *header, uint8_t hlen,
                  const uint8_t *data,   uint8_t dlen)
{
    uint8_t len = hlen + dlen; // FIXME: might overflow
    Handle_t h;

    if (len > CC1101_MAX_PACKET_LEN)
    {
        return -EMSGSIZE;
    }

    ATOMIC_START(h);

    chipSelect();
    waitForListen(); // Ensure any previous outbound packet has been sent

    if (getreg(REG_TXBYTES) != 0)
    {
        // TX FIFO is not empty and the radio is not in TX, which means that
        // the last send operation did not start due to CCA failure. Currently,
        // no re-send is attempted. Hence, we clear the TX FIFO to avoid
        // possible overflow. This is done inside an "if" as to not disrupt
        // the packet reception process, if possible.
        strobe(STROBE_SIDLE);
        strobe(STROBE_SFTX);
        // Get back to RX to enable TX-if-CCA
        strobe(STROBE_SRX);
    }

    setreg(REG_FIFO, len);
    if (header)
    {
        burstWrite(REG_FIFO, header, hlen);
    }
    if (data)
    {
        burstWrite(REG_FIFO, data, dlen);
    }

    strobe(STROBE_STX);

    chipRelease();

    ATOMIC_END(h);

    return 0;
}

static inline void flushrx(void)
{
    strobe(STROBE_SIDLE);
    strobe(STROBE_SFRX);
    strobe(STROBE_SRX);
}

int8_t cc1101Read(uint8_t *buf, uint8_t buflen)
{
    uint8_t res, len;
    uint8_t aux[2];
    Handle_t h;

    ATOMIC_START(h);

    chipSelect();

    len = getreg(REG_FIFO);
    if (len > buflen)
    {
        res = -EMSGSIZE;
        flushrx(); // Will also resume RX
        goto end;
    }

    burstRead(REG_FIFO, buf, len);
    burstRead(REG_FIFO, aux, sizeof(aux));
    lastRSSI = aux[0];
    //
    // The CRC was already checked in the interrupt handler; now we just remove
    // the CRC bit
    //
    lastLQI  = aux[1] & ~LQI_CRC_OK;
    res = len;

    // Resume RX
    strobe(STROBE_SRX);

end:
    chipRelease();
    ATOMIC_END(h);
    return res;
}

void cc1101Discard(void)
{
    Handle_t h;

    ATOMIC_START(h);
    chipSelect();
    flushrx();
    chipRelease();
    ATOMIC_END(h);
}

bool cc1101IsChannelClear(void)
{
    return true; // XXX TODO
}


//
// Interrupt handling
//
XISR(CC1101_INTR_PORT, cc1011Interrupt)
{
    if (pinReadIntFlag(CC1101_INTR_PORT, CC1101_INTR_PIN))
    {
        bool    quit = false;
        state_t state;

        pinClearIntFlag(CC1101_INTR_PORT, CC1101_INTR_PIN);

        chipSelect();
        state = getState(status());
        if (state == STATE_TX_UNDERFLOW) // TX FIFO underflow, shouldn't happen
        {
            strobe(STROBE_SFTX);
            strobe(STROBE_SRX); // Return to RX
            quit = true;
        }
        else if (state == STATE_RX_OVERFLOW ||    // If RX FIFO overflowed
                 !(getreg(REG_LQI) & LQI_CRC_OK)) // Or if CRC not OK
        {
            flushrx();
            quit = true;
        }
        chipRelease();
        if (quit)
        {
            return;
        }

#if USE_THREADS
        processFlags.bits.radioProcess = true;
        // wake up the kernel thread
        EXIT_SLEEP_MODE();
#else
        if (!callback)
        {
            cc1101Discard();
        }
        else
        {
            callback();
        }
#endif
    }
}
