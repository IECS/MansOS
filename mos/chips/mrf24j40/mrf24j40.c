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

#include "mrf24j40.h"
#include "mrf24j40_const.h"
#include "mrf24j40_pins.h"
#include <spi.h>
#include <delay.h>
#include <radio.h>
#include <errors.h>
#include <serial.h>
#include <kernel/threads/threads.h>
#include "platform.h"

static MRF24J40RxHandle rxCallback;
static uint8_t lastLqi;
static int8_t lastRssi;
static bool mrf24j40IsOn;

#if DEBUG
#define RADIO_DEBUG 1
#endif

#if RADIO_DEBUG
#include "dprint.h"
#define RPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define RPRINTF(...) do {} while (0)
#endif

#define CCA_ED_THRESHOLD 0x60  // -69 dBm


static void mrf24j40SetPanAddr(unsigned pan,
                               unsigned addr,
                               const uint8_t *ieee_addr);

static void on(void);
static void off(void);

// --------------------------------------------------

static void SetShortRAMAddr(uint8_t address, uint8_t data)
{
    MRF24_SPI_ENABLE();
    MRF24_WR_BYTE(address);
    MRF24_WR_BYTE(data);
    MRF24_SPI_DISABLE();
}

static uint8_t GetShortRAMAddr(uint8_t address)
{
    uint8_t data = 0;
    MRF24_SPI_ENABLE();
    MRF24_WR_BYTE(address);
    data = MRF24_RD_BYTE();
    MRF24_SPI_DISABLE();
    return data;
}

static void SetLongRAMAddr(uint16_t address, uint8_t data)
{
    MRF24_SPI_ENABLE();
    MRF24_WR_BYTE(((address >> 3) & 0x7f) | 0x80);
    MRF24_WR_BYTE(((address << 5) & 0xe0) | 0x10);
    MRF24_WR_BYTE(data);
    MRF24_SPI_DISABLE();
}

static uint8_t GetLongRAMAddr(uint16_t address)
{
    uint8_t data = 0;
    MRF24_SPI_ENABLE();
    MRF24_WR_BYTE(((address >> 3) & 0x7f) | 0x80);
    MRF24_WR_BYTE(((address << 5) & 0xe0));
    data = MRF24_RD_BYTE();
    MRF24_SPI_DISABLE();
    return data;
}

static void resetRx(void)
{
    // write reset bit
    SetShortRAMAddr(WRITE_RFCTL, 0x04);
    // write normal status
    SetShortRAMAddr(WRITE_RFCTL, 0x00);
}

static void flushRx(void)
{
    SetShortRAMAddr(WRITE_RXFLUSH, 0x01);
}

void mrf24j40Init(void)
{
    RPRINTF("mrf24j40Init\n");

    spiBusInit(MRF24_SPI_ID, SPI_MODE_MASTER);

    pinAsOutput(MRF24_CS_PORT, MRF24_CS_PIN);
    pinAsOutput(MRF24_RESET_PORT, MRF24_RESET_PIN);
    pinAsOutput(MRF24_WAKE_PORT, MRF24_WAKE_PIN);
    pinAsInput(MRF24_INT_PORT, MRF24_INT_PIN);

    pinSet(MRF24_CS_PORT, MRF24_CS_PIN);
    pinSet(MRF24_RESET_PORT, MRF24_RESET_PIN);
    pinSet(MRF24_WAKE_PORT, MRF24_WAKE_PIN);

    // place the device in hardware reset
    pinClear(MRF24_RESET_PORT, MRF24_RESET_PIN);
    mdelay(2);
    // remove the device from hardware reset
    pinSet(MRF24_RESET_PORT, MRF24_RESET_PIN);
    mdelay(2);
    // reset the RF module
    flushRx();
    resetRx();

#if 0
    // receive all packets (promisc mode), but not with invalid FCS
    SetShortRAMAddr(WRITE_RXMCR, 0x01);
#else
    // receive all packets, including those with invalid FCS
    SetShortRAMAddr(WRITE_RXMCR, 0x03);
#endif

    // PACON2 = 0x98, Initialize FFOEN=1 and TXONTS = 0x6
    SetShortRAMAddr(WRITE_PACON2, 0x98);
    // Initialize RFSTBL = 0x9
    SetShortRAMAddr(WRITE_TXSTBL, 0x95);

    // --
    // wait until the chip is in receive mode
    uint8_t check;
    do {
        check = GetLongRAMAddr(RFSTATE);
    } while ((check & 0xA0) != 0xA0);
    SetLongRAMAddr(RFCON0, 0x03); // or three?
    // --

    // Initialize VCOOPT=1
    SetLongRAMAddr(RFCON1, 0x01);
    // Enable PLL
    SetLongRAMAddr(RFCON2, 0x80);
    // Initialize TXFIL=1, 20MRECVR=1
    SetLongRAMAddr(RFCON6, 0x90);
    // Initialize SLPCLKSEL = 0x2 (100KHz internal oscillator)
    SetLongRAMAddr(RFCON7, 0x80);
    // Initialize RFVCO =1
    SetLongRAMAddr(RFCON8, 0x10);
    // Initialize CLKOUTEN=1 and SLPCLKDIV = 0x01
    SetLongRAMAddr(SLPCON1, 0x21);
    // Set CCA mode to ED
    SetShortRAMAddr(WRITE_BBREG2, 0x80);
    // Set CCA-ED Threshold
    SetShortRAMAddr(WRITE_CCAEDTH, CCA_ED_THRESHOLD);
    // Set appended RSSI value to RX FIFO
    SetShortRAMAddr(WRITE_BBREG6, 0x40);

    mrf24j40SetPanAddr(0xffff, 0x0000, NULL);
    mrf24j40SetChannel(RADIO_CHANNEL);
    mrf24j40SetTxPower(RADIO_TX_POWER);

    off();
}

static void mrf24j40SetPanAddr(unsigned pan,
                               unsigned addr,
                               const uint8_t *ieee_addr)
{
    // Program the short MAC Address, 0xffff
    SetShortRAMAddr(WRITE_SADRL, 0xFF);
    SetShortRAMAddr(WRITE_SADRH, 0xFF);
    SetShortRAMAddr(WRITE_PANIDL, 0xFF);
    SetShortRAMAddr(WRITE_PANIDH, 0xFF);
    // Program long MAC Address
    if (ieee_addr != NULL) {
        uint8_t i;
        for (i = 0; i < 8; i++) {
            SetShortRAMAddr(WRITE_EADR0 + i * 2, ieee_addr[i]);
        }
    }
    // reset the RF module with new settings
    resetRx();
}

static void on(void) {
    RPRINTF("MRF24 on\n");

    Handle_t h;
    ATOMIC_START(h);

    pinSet(MRF24_WAKE_PORT, MRF24_WAKE_PIN);

    mdelay(1); // XXX?
    resetRx();

    ATOMIC_END(h);
}

static void off(void) {
    RPRINTF("MRF24 off\n");

    Handle_t h;
    ATOMIC_START(h);

    // clear the WAKE pin in order to allow the device to go to sleep
    pinClear(MRF24_WAKE_PORT, MRF24_WAKE_PIN);

    // make a power management reset to ensure device goes to sleep
    SetShortRAMAddr(WRITE_SOFTRST, 0x04);

    // enable immediate wakeup mode
    SetShortRAMAddr(WRITE_WAKECON, 0x80);
    // enable WAKE pin and set polarity high
    SetShortRAMAddr(WRITE_RXFLUSH, 0x60);
    // put MRF24J40 to sleep mode (0x35)
    SetShortRAMAddr(WRITE_SLPACK, 0x80);

    ATOMIC_END(h);
}

void mrf24j40On(void)
{
    if (!mrf24j40IsOn) {
        on();
        mrf24j40IsOn = true;
    }
}

void mrf24j40Off(void)
{
    if (mrf24j40IsOn) {
        off();
        mrf24j40IsOn = false;
    }
}

int_t mrf24j40Send(const void *header, uint16_t headerLen,
                   const void *data, uint16_t dataLen)
{
    int_t result;
    uint8_t status;

    if (serial[MRF24_SPI_ID].busy) {
        RPRINTF("cannot send via radio, flash is being used!\n");
        return -EBUSY;
    }

    Handle_t h;
    ATOMIC_START(h);

#define PENDACK (1 << 4)
    do {
        status = GetShortRAMAddr(READ_TXNCON);
    } while (status & PENDACK);

    bool turnOff;
    if (!mrf24j40IsOn) {
        on();
        turnOff = true;
    } else {
        turnOff = false;
    }

    uint16_t totalLen = headerLen + dataLen;
    if (totalLen > MRF24J40_MAX_PACKET_LEN) {
        RPRINTF("mrf24j40: cannot tx, packet too long (%u bytes)!\n", totalLen);
        result = -EMSGSIZE;
        goto end;
    }
    RPRINTF("mrf24j40: sending %u bytes\n", totalLen);

    SetLongRAMAddr(0x000, totalLen);
    SetLongRAMAddr(0x001, totalLen);

    // hardware release
    GetLongRAMAddr(0x300);

    uint8_t i = 2;
    // write user's packet header
    const uint8_t *p = (const uint8_t *) header;
    const uint8_t *end = p + headerLen;
    while (p < end) {
        SetLongRAMAddr(i++, *p++);
    }
    // write user's packet data
    p = (const uint8_t *) data;
    end = p + dataLen;
    while (p < end) {
        SetLongRAMAddr(i++, *p++);
    }
    // hardware release
    GetLongRAMAddr(0x300);

#if 0
    mdelay(2);

    Handle_t handle;
    ATOMIC_START(handle);
    // transmit packet without ACK requested
    SetShortRAMAddr(WRITE_TXNCON, 0x1);
    // wait for Tx interrupt to happen
    do {
        status = GetShortRAMAddr(READ_INTSTAT);
    } while (!(status & MRF24J40_TX_IF));
    ATOMIC_END(handle);
#else
    // transmit packet without ACK requested
    SetShortRAMAddr(WRITE_TXNCON, 0x1);
#endif

    if (turnOff) {
        off();
    }

    result = 0;
  end:
    ATOMIC_END(h);
    return result;
}

ISR(PORT1, mrf24Interrupt)
{
    if (pinReadIntFlag(MRF24_INT_PORT, MRF24_INT_PIN)) {
        pinClearIntFlag(MRF24_INT_PORT, MRF24_INT_PIN);
        RPRINTF("****************** got radio interrupt!\n");

        mrf24j40PollForPacket();
#if USE_THREADS
        if (processFlags.bits.radioProcess) {
            // wake up the kernel thread
            EXIT_SLEEP_MODE();
        }
#endif
    } else {
        RPRINTF("got some other port1 interrupt!\n");
    }
}

int_t mrf24j40Read(void *buf_, uint16_t bufsize)
{
    uint8_t *buf = (uint8_t *) buf_;
    uint8_t i, len;
    int_t result = -EIO;

    // If the part is enabled for receiving packets right now
    // (not pending an ACK)
    // indicate that we have a packet in the buffer pending to
    // be read into the buffer from the FIFO
    SetShortRAMAddr(WRITE_BBREG1, 0x04);

    len = GetLongRAMAddr(0x300);
    RPRINTF("mrf24j40Read: len=%d\n", len);
    if (len < AUX_LEN) {
        RPRINTF("mrf24j40Read: packet too small (%d)!\n", len);
        goto flush;
    }
    if (len > bufsize + AUX_LEN) {
        RPRINTF("mrf24j40Read: buffer too small (%d vs %d)!\n", len, bufsize);
        goto flush;
    }
#if XBEE_PRO_SUPPORT
    // XXX: this code is needed to ignore Waspmote beacons!
    if (len < 30) {
        RPRINTF("len < min len, len=%u\n", len);
        goto flush;
    }
#endif

    // read these value first because reading the whole packets enables another packet to be received!
    lastLqi = GetLongRAMAddr(0x300 + len + 1);
    lastRssi = GetLongRAMAddr(0x300 + len + 2);

    // read the packet (excluding FCS)
    len -= AUX_LEN;
    for (i = 0; i < len; i++) {
        buf[i] = GetLongRAMAddr(0x300 + i + 1);
    }
    result = len;

  flush:
    // Note: according to errata, in promisc mode rx must 
    // be flushed after each rx interrupt!
    flushRx();
    // enable radio to receive next packet
    SetShortRAMAddr(WRITE_BBREG1, 0x00);

    return result;
}

void mrf24j40Discard(void)
{
//    SetShortRAMAddr(WRITE_BBREG1, 0x04);
    // flush the RX fifo
    flushRx();
//    SetShortRAMAddr(WRITE_BBREG1, 0x00);
}

MRF24J40RxHandle mrf24j40SetReceiver(MRF24J40RxHandle newRxCallback)
{
    MRF24J40RxHandle oldRxCallback = rxCallback;
    rxCallback = newRxCallback;
    return oldRxCallback;
}

void mrf24j40SetChannel(int channel)
{
    if (channel >= 11 && channel <= 26) {
        channel -= 11;
        SetLongRAMAddr(RFCON0,(channel << 4) | 0x03);

        // reset the RF module with new settings
        resetRx();
    }
}

void mrf24j40SetTxPower(uint8_t power)
{
    // writing 0 in register selects max power, writing 31 min power
    power = power > MRF24J40_TX_POWER_MAX ? 0 : MRF24J40_TX_POWER_MAX - power;

    SetLongRAMAddr(RFCON3, power << 3);

    // reset the RF module with new settings
    resetRx();
}

int8_t mrf24j40GetLastRSSI(void)
{
    return lastRssi;
}

uint8_t mrf24j40GetLastLQI(void)
{
    return lastLqi;
}

int mrf24j40GetRSSI(void)
{
    int rssi;
    uint8_t rssiReady;

    // set RSSI request
    SetShortRAMAddr(WRITE_BBREG6, 0x80);

    // check RSSI
    do {
        rssiReady = GetShortRAMAddr(READ_BBREG6);
    } while ((rssiReady & 0x01) != 0x01 || (rssiReady & 0x80));

    rssi = (int)GetLongRAMAddr(RSSI);

    // enable RSSI attached to received packet again after
    // the energy scan is finished
    SetShortRAMAddr(WRITE_BBREG6, 0x40);

    return rssi;
}

bool mrf24j40IsChannelClear(void)
{
    // TODO: this is not working very well
    uint8_t rssi = (uint8_t) mrf24j40GetRSSI();
    return !rssi || rssi > CCA_ED_THRESHOLD;
}

bool mrf24j40PollForPacket(void)
{
    // don't touch radio SPI when usart is busy (TODO: check that this does not hang sending!)
    if (serial[MRF24_SPI_ID].busy) {
        return false;
    }

    MRF24J40_IFREG intstat;
    intstat.value = GetShortRAMAddr(READ_INTSTAT);
    if (!intstat.value) {
        return false;
    }

    bool process = false;

    if (intstat.bits.RF_TXIF) {
        RPRINTF("got radio TX int!\n");
    }
    if (intstat.bits.RF_RXIF) {
        RPRINTF("got radio RX int!\n");

#if USE_THREADS
        if (mrf24j40IsOn) {
            process = true;
        } else {
            mrf24j40Discard();
        }
#else
        if (mrf24j40IsOn && rxCallback) {
            rxCallback();
        } else {
            mrf24j40Discard();
        }
#endif
    }
    if (intstat.bits.RF_SECIF) {
        RPRINTF("MRF24J40 RF_SECIF, intstat=0x%02X\n", intstat.value);
        SetShortRAMAddr(WRITE_SECCON0, 0x80); // ignore the packet
    }
    if (intstat.bits.RF_WAKEIF) {
        // this is normal when the device is awaken from sleep
    }

    if (intstat.value & ~(MRF24J40_RX_IF | MRF24J40_TX_IF
                    | MRF24J40_SEC_IF | MRF24J40_WAKE_IF)) {
        // some error? reset the stack
        RPRINTF("************** MRF24J40 RESET RADIO STACK, intstat=0x%02X\n",
            intstat.value);
        flushRx();
        resetRx();
    }

#if USE_THREADS
    if (process) processFlags.bits.radioProcess = true;
#endif
    return process;
}
