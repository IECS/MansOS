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

/*
 * Portions copyright (c) 2007, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>

#include "platform.h"
#include "cc2420.h"
#include "cc2420_const.h"
#include "cc2420_spi.h"
#include <radio.h>
#include <errors.h>
#include <lib/codec/crc.h>
#include <kernel/threads/threads.h>
#include <lib/energy.h>
#include <hil/atomic.h>
#if USE_PROTOTHREADS
#include <kernel/protothreads/process.h>
#include <kernel/protothreads/radio-process.h>
#endif
#include <serial.h>


// for 4MHz CPU speed? (326us required)
#define LOOP_20_SYMBOLS 700

#if DEBUG
#define RADIO_DEBUG 1
#endif

#if RADIO_DEBUG
#include <lib/dprint.h>
#define RPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define RPRINTF(...) do {} while (0)
#endif

static void cc2420SetPanAddr(unsigned pan,
                             unsigned addr,
                             const uint8_t *ieee_addr) UNUSED;

static void cc2420DisableSpi(void);
static void cc2420Start(void);

/*---------------------------------------------------------------------------*/

#define RTIMER_SECOND 32768 // timer A feeds from from ACLK
#define RTIMER_NOW() ALARM_TIMER_READ()
#define RTIMER_CLOCK_LT(a,b)     ((signed short)((a)-(b)) < 0)

#undef BUSYWAIT_UNTIL
#define BUSYWAIT_UNTIL(cond, max_time)                                     \
    do {                                                                   \
        uint32_t t0;                                                       \
        t0 = RTIMER_NOW();                                                 \
        while (!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))); \
    } while (0)

static CC2420RxHandle receiver_callback;

static int8_t cc2420_last_rssi;
static uint8_t cc2420_last_lqi;

static uint8_t receive_on;
/* Radio stuff in network byte order. */
static uint16_t pan_id;

static uint8_t channel = RADIO_CHANNEL;
static uint8_t txPower = RADIO_TX_POWER;

static bool initialized;

/*---------------------------------------------------------------------------*/

static void getrxdata(void *buf, int len)
{
    CC2420_READ_FIFO_NO_WAIT(buf, len);
}

static void getrxbyte(uint8_t *byte)
{
    CC2420_READ_FIFO_BYTE(*byte);
}

static void flushrx(void)
{
    uint8_t dummy;

    CC2420_READ_FIFO_BYTE(dummy);
    CC2420_STROBE(CC2420_SFLUSHRX);
    CC2420_STROBE(CC2420_SFLUSHRX);
}

static void strobe(enum cc2420_register regname)
{
    CC2420_STROBE(regname);
}

uint8_t cc2420GetStatus(void)
{
    uint8_t status;
    CC2420_UPD_STATUS(status);
    return status;
}

static void on(void)
{
    RPRINTF("turn CC2420 radio rx on\n");

    // start the CC2420
    cc2420InitSpi();
    cc2420Start();

    CC2420_ENABLE_FIFOP_INT();
    strobe(CC2420_SRXON);

    BUSYWAIT_UNTIL(cc2420GetStatus() & (BV(CC2420_XOSC16M_STABLE)), RTIMER_SECOND / 100);

    receive_on = 1;
    energyConsumerOn(ENERGY_CONSUMER_RADIO_RX);
}

static void off(bool disableSpi)
{
    RPRINTF("turn CC2420 radio rx off\n");
    receive_on = 0;

    // Wait for transmission to end before turning radio off.
    BUSYWAIT_UNTIL(!(cc2420GetStatus() & BV(CC2420_TX_ACTIVE)), RTIMER_SECOND / 10);

    energyConsumerOff(ENERGY_CONSUMER_RADIO_RX);

    strobe(CC2420_SRFOFF);
    CC2420_DISABLE_FIFOP_INT();

    if (!CC2420_FIFOP_IS_1) {
        flushrx();
    }

    if (disableSpi) {
        CC2420_SET_VREG_INACTIVE();
        cc2420DisableSpi();
    }
}

static inline unsigned getreg(enum cc2420_register regname)
{
    unsigned reg;
    CC2420_GETREG(regname, reg);
    return reg;
}

static inline void setreg(enum cc2420_register regname, unsigned value)
{
    CC2420_SETREG(regname, value);
}

void cc2420InitSpi(void)
{
    pinAsOutput(CC2420_CSN_PORT, CC2420_CSN_PIN);
    // Unselect radio
    CC2420_SPI_DISABLE();

    serial[CC2420_SPI_ID].function = SERIAL_FUNCTION_RADIO;
    spiBusInit(CC2420_SPI_ID, SPI_MODE_MASTER);
}

static void cc2420DisableSpi(void)
{
    spiBusDisable(CC2420_SPI_ID);
}

static inline void cc2420StartVreg(void)
{
    CC2420_SET_VREG_ACTIVE();
    ALARM_TIMER_WAIT_TICKS(20);
    CC2420_SET_RESET_ACTIVE();
    ALARM_TIMER_WAIT_TICKS(20);
    CC2420_SET_RESET_INACTIVE();
    ALARM_TIMER_WAIT_TICKS(20); // ?
}

static inline void cc2420StopVreg(void)
{
    CC2420_SET_RESET_ACTIVE();
    CC2420_SET_VREG_INACTIVE();
    CC2420_SET_RESET_INACTIVE();
}

void cc2420Init(void)
{
    Handle_t h;

    if (initialized) {
        RPRINTF("calling cc2420Init multiple times!\n");
        // allow calling initialization multiple times, for example
        // to arbitrate between multiple devices on the same SPI bus
        // return;
    }
    initialized = true;

    ATOMIC_START(h);
    {
        // Initalize ports and SPI
        cc2420InitSpi();

        pinAsOutput(CC2420_RESET_N_PORT, CC2420_RESET_N_PIN);
        pinAsOutput(CC2420_VREG_EN_PORT, CC2420_VREG_EN_PIN);

        pinAsInput(CC2420_FIFO_P_PORT, CC2420_FIFO_P_PIN);
        pinAsInput(CC2420_FIFO_PORT, CC2420_FIFO_PIN);
        pinAsInput(CC2420_CCA_PORT, CC2420_CCA_PIN);
        pinAsInput(CC2420_SFD_PORT, CC2420_SFD_PIN);

        CC2420_DISABLE_FIFOP_INT();
        CC2420_FIFOP_INT_INIT();
    }
    ATOMIC_END(h);
}

static void cc2420Start(void)
{
    uint16_t reg;

    cc2420StartVreg();

    // Turn on the crystal oscillator
    strobe(CC2420_SXOSCON);

    // Turn on/off automatic packet acknowledgment and address decoding
    reg = getreg(CC2420_MDMCTRL0);
#if CC2420_CONF_AUTOACK
    reg |= MDMCTRL0_AUTOACK | MDMCTRL0_ADDR_DECODE;
#else
    reg &= ~(MDMCTRL0_AUTOACK | MDMCTRL0_ADDR_DECODE);
#endif // CC2420_CONF_AUTOACK
#if CC2420_CONF_AUTOCRC
    /* Turn on automatic CRC calculation. */
    reg |= MDMCTRL0_AUTOCRC;
#else
    reg &= ~MDMCTRL0_AUTOCRC;
#endif
    setreg(CC2420_MDMCTRL0, reg);

    // Set transmission turnaround time to the lower setting (8 symbols
    //   = 0.128 ms) instead of the default (12 symbols = 0.192 ms).
    /* reg = getreg(CC2420_TXCTRL);
    reg &= ~(1 << 13);
    setreg(CC2420_TXCTRL, reg); */

    /* Change default values as recommended in the data sheet, */
    /* correlation threshold = 20, RX bandpass filter = 3uA. */
    setreg(CC2420_MDMCTRL1, CORR_THRESHOLD_DEFAULT);
    reg = getreg(CC2420_RXCTRL1);
    reg |= RXCTRL1_RXBPF_LOCUR;
    setreg(CC2420_RXCTRL1, reg);

    /* Set the FIFOP threshold to maximum. */
    setreg(CC2420_IOCFG0, IOCFG0_FIFOP_THR(CC2420_HW_PACKET_LIMIT));

    /* Turn off "Security enable" (page 32). */
    reg = getreg(CC2420_SECCTRL0);
    reg &= ~SECCTRL0_RXFIFO_PROTECTION;
    setreg(CC2420_SECCTRL0, reg);

    // cc2420SetPanAddr(0xffff, 0x0000, NULL); // ?
    cc2420SetChannel(channel);
    cc2420SetTxPower(txPower);

    flushrx();
}

int_t cc2420Send(const void *header, uint16_t headerLen,
                 const void *data, uint16_t dataLen)
{
    int i;
    uint8_t totalLen;
#if CC2420_CONF_CHECKSUM
    uint16_t checksum;
#endif // CC2420_CONF_CHECKSUM
    int_t result;

    Handle_t h;
    ATOMIC_START(h);

    if (!receive_on)  {
        cc2420InitSpi();
        cc2420Start();
    }

    /* Wait for any previous transmission to finish. */
    /*  while(cc2420GetStatus() & BV(CC2420_TX_ACTIVE));*/

    /* Write packet to TX FIFO. */
    strobe(CC2420_SFLUSHTX);

#if CC2420_CONF_CHECKSUM
    checksum = crc16(data, dataLen);
#endif
    totalLen = headerLen + dataLen;
    if (totalLen > CC2420_MAX_PACKET_LEN) {
        RPRINTF("cc2420: cannot tx, packet too long (%u bytes)!\n", totalLen);
        result = -EMSGSIZE;
        goto end;
    }
    totalLen += AUX_LEN;
    RPRINTF("cc2420: sending %u bytes\n", totalLen);
    CC2420_WRITE_FIFO(&totalLen, 1);
    CC2420_WRITE_FIFO2(header, headerLen, data, dataLen);
#if CC2420_CONF_CHECKSUM
    CC2420_WRITE_FIFO(&checksum, CHECKSUM_LEN);
#endif

    /* The TX FIFO can only hold one packet. Make sure to not overrun
     * FIFO by waiting for transmission to start here and synchronizing
     * with the CC2420_TX_ACTIVE check in cc2420_send.
     *
     * Note that we may have to wait up to 320 us (20 symbols) before
     * transmission starts.
     */

#if WITH_SEND_CCA
    strobe(CC2420_SRXON);
    BUSYWAIT_UNTIL(cc2420GetStatus() & BV(CC2420_RSSI_VALID), RTIMER_SECOND / 10);
    strobe(CC2420_STXONCCA);
#else /* WITH_SEND_CCA */
    strobe(CC2420_STXON);
#endif /* WITH_SEND_CCA */

    for (i = LOOP_20_SYMBOLS; i > 0; i--) {
        if (CC2420_SFD_IS_1) {
            if (!(cc2420GetStatus() & BV(CC2420_TX_ACTIVE))) {
                /* SFD went high but we are not transmitting. This means that
                   we just started receiving a packet, so we drop the transmission. */
                result = -EBUSY;
                goto end;
            }

            energyConsumerOnNoints(ENERGY_CONSUMER_RADIO_TX);
            // TODO: when transmitting, listening does NOT spend energy!

            /* We wait until transmission has ended so that we get an
               accurate measurement of the transmission time.*/
            BUSYWAIT_UNTIL(!(cc2420GetStatus() & BV(CC2420_TX_ACTIVE)), RTIMER_SECOND / 10);

            energyConsumerOffNoints(ENERGY_CONSUMER_RADIO_TX);
            if (!receive_on) {
                /* We need to explicitly turn off the radio,
                 * since STXON[CCA] -> TX_ACTIVE -> RX_ACTIVE */
                off(false);
            }

            result = 0; // success
            goto end;
        }
    }

    /* If we are using WITH_SEND_CCA, we get here if the packet wasn't
       transmitted because of other channel activity. */
    RPRINTF("cc2420Send(): transmission never started\n");
    result = -EBUSY;
  end:
    // RPRINTF("cc2420: tx done, result=%d\n", result);

    if (!receive_on)  {
        //mdelay(100);
        cc2420DisableSpi();
        cc2420StopVreg();
    }
    ATOMIC_END(h);
    return result;
}

int_t cc2420Read(void *buf, uint16_t bufsize)
{
    uint8_t footer[2];
    uint8_t len;
#if CC2420_CONF_CHECKSUM
    uint16_t checksum;
#endif
    int_t result = -EIO;

    Handle_t h;
    ATOMIC_START(h);

    if (!CC2420_FIFOP_IS_1) {
        goto end;
    }
 
    getrxbyte(&len);

    if (len > CC2420_MAX_PACKET_LEN + AUX_LEN) {
        /* Oops, we must be out of sync. */
        flushrx();
        RPRINTF("len > max, len=%u\n", len);
        goto end;
    }

    if (len <= AUX_LEN) {
        flushrx();
        RPRINTF("len <= aux len, len=%u\n", len);
        goto end;
    }

    len -= AUX_LEN;

    if (len > bufsize) {
        flushrx();
        RPRINTF("len > bufsize (%u vs. %u)\n", len, bufsize);
        result = -EMSGSIZE;
        goto end;
    }

    getrxdata(buf, len);
#if CC2420_CONF_CHECKSUM
    getrxdata(&checksum, CHECKSUM_LEN);
#endif // CC2420_CONF_CHECKSUM
    getrxdata(footer, FOOTER_LEN);

#if CC2420_CONF_CHECKSUM
    if (checksum != crc16(buf, len)) {
        RPRINTF("checksum failed 0x%04x != 0x%04x\n",
                checksum, crc16(buf, len));
    }

    if ((footer[1] & FOOTER1_CRC_OK) &&
            checksum == crc16(buf, len)) {
#else
    if ((footer[1] & FOOTER1_CRC_OK)) {
#endif // CC2420_CONF_CHECKSUM
        cc2420_last_rssi = footer[0] + CC2420_RSSI_OFFSET;
        cc2420_last_lqi = footer[1] & FOOTER1_LQI;
        result = len;
    } else {
        RPRINTF("footer = 0x%x 0x%x\n", footer[0], footer[1]);
        result = -EBADMSG;
    }

    if (CC2420_FIFOP_IS_1) {
        if (!CC2420_FIFO_IS_1) {
            // Clean up in case of FIFO overflow!  This happens for every
            // full length frame and is signaled by FIFOP = 1 and FIFO = 0.
            flushrx();
        } else {
            // Another packet has been received and needs attention
            //
            // XXX TODO!
            //
            RPRINTF("cc2420Read(): another packet pending!\n");
            flushrx();
        }
    }

   end:
    // RPRINTF("cc2420Read: result=%d\n", result);
    ATOMIC_END(h);
    return result;
}

void cc2420Discard(void)
{
    Handle_t h;
    ATOMIC_START(h);
    flushrx();
    ATOMIC_END(h);
}

void cc2420Off(void)
{
    if (receive_on) {
        off(true);
    }
}

void cc2420On(void)
{
    if (!receive_on) {
        on();
    }
}

void cc2420SetChannel(int c)
{
    uint16_t f;

    /*
     * Subtract the base channel (11), multiply by 5, which is the
     * channel spacing. 357 is 2405-2048 and 0x4000 is LOCK_THR = 1.
     */
    channel = c;

    f = 5 * (c - 11) + 357 + 0x4000;
    /*
     * Writing RAM requires crystal oscillator to be stable.
     */
    BUSYWAIT_UNTIL((cc2420GetStatus() & (BV(CC2420_XOSC16M_STABLE))), RTIMER_SECOND / 10);

    /* Wait for any transmission to end. */
    BUSYWAIT_UNTIL(!(cc2420GetStatus() & BV(CC2420_TX_ACTIVE)), RTIMER_SECOND / 10);

    setreg(CC2420_FSCTRL, f);

    /* If we are in receive mode, we issue an SRXON command to ensure
       that the VCO is calibrated. */
    if (receive_on) {
        strobe(CC2420_SRXON);
    }
}

static void cc2420SetPanAddr(unsigned pan,
                             unsigned addr,
                             const uint8_t *ieee_addr)
{
    /*
     * Writing RAM requires crystal oscillator to be stable.
     */
    BUSYWAIT_UNTIL(cc2420GetStatus() & (BV(CC2420_XOSC16M_STABLE)), RTIMER_SECOND / 10);

    pan_id = pan;
    CC2420_WRITE_RAM_LE(&pan, CC2420RAM_PANID, 2);
    CC2420_WRITE_RAM_LE(&addr, CC2420RAM_SHORTADDR, 2);
    if (ieee_addr != NULL) {
        CC2420_WRITE_RAM_LE(ieee_addr, CC2420RAM_IEEEADDR, 8);
    }
}

/*
* Interrupt leaves frame intact in FIFO.
*/
ISR(PORT1, cc2420Interrupt)
{
    // enter active CPU mode
    energyConsumerOnIRQ(ENERGY_CONSUMER_MCU);

    CC2420_CLEAR_FIFOP_INT();

    // PRINT("cc2420Interrupt\n");

    // USARTSendByte(1, 'I');

#if USE_THREADS
    processFlags.bits.radioProcess = true;
    // wake up the kernel thread!
    EXIT_SLEEP_MODE();
#elif USE_PROTOTHREADS
    // poll protothread
    //RPRINTF("%lu, calling radio poll\n", getJiffies());
    process_poll(&radio_process);
#else
    // plain handler
    if (receiver_callback) {
        receiver_callback();
    } else {
        flushrx();
    }
#endif

    energyConsumerOffIRQ(ENERGY_CONSUMER_MCU);
}

CC2420RxHandle cc2420SetReceiver(CC2420RxHandle recv)
{
    CC2420RxHandle old = receiver_callback;
    receiver_callback = recv;
    return old;
}


void cc2420SetTxPower(uint8_t power)
{
    uint16_t reg;
    txPower = power;
    reg = getreg(CC2420_TXCTRL);
    reg = (reg & 0xffe0) | (power & 0x1f);
    setreg(CC2420_TXCTRL, reg);
}

int8_t cc2420GetLastRSSI(void)
{
    return cc2420_last_rssi;
}

uint8_t cc2420GetLastLQI(void)
{
    return cc2420_last_lqi;
}

int cc2420GetRSSI(void)
{
    // radio must be turned on before calling this!
    if (!receive_on) return 0;

    BUSYWAIT_UNTIL(cc2420GetStatus() & BV(CC2420_RSSI_VALID), RTIMER_SECOND / 100);

    return ((int)((signed char)getreg(CC2420_RSSI)) + CC2420_RSSI_OFFSET);
}

bool cc2420IsChannelClear(void)
{
    // radio must be turned on before calling this!
    if (!receive_on) return true;

    BUSYWAIT_UNTIL(cc2420GetStatus() & BV(CC2420_RSSI_VALID), RTIMER_SECOND / 100);

    return CC2420_CCA_IS_1;
}

//
// Send continous wave (unmodulated signal carrier)
//
void cc2420ContinousWave(bool on)
{
    /*
     CC2420 datasheet:
     "An unmodulated carrier may be transmitted by setting MDMCTRL1.TX_MODE to 2 or 3,
     writing 0x1800 to the DACTST register and issue a STXON command strobe. The transmitter
     is then enabled while the transmitter I/O DACs are overridden to static values. An
     unmodulated carrier will then be available on the RF output pins."
    */
    uint16_t reg = getreg(CC2420_MDMCTRL1);
    if (on) {
        reg |= TX_TEST_MODE_CYCLIC;
    } else {
        reg &= ~TX_TEST_MODE_MASK;
    }
    setreg(CC2420_MDMCTRL1, reg);
    if (on) {
        setreg(CC2420_DACTST, DACTST_CONTINUOUS_WAVE);
        strobe(CC2420_STXON);
    } else {
        setreg(CC2420_DACTST, 0);
        strobe(CC2420_STXON);
    }
}

//
// Configure CCA with specific values
//
void configureCCA(int8_t threshold, uint8_t mode)
{
    uint16_t reg;

    reg = getreg(CC2420_MDMCTRL0);
    reg &= ~CCA_MODE_MASK;
    reg |= (mode & CCA_MODE_MASK);
    setreg(CC2420_MDMCTRL0, reg);

    reg = (uint16_t)threshold << 8;
    setreg(CC2420_RSSI, reg);

    // if (receive_on) {
    //     BUSYWAIT_UNTIL(cc2420GetStatus() & BV(CC2420_RSSI_VALID), RTIMER_SECOND / 100);
    //     uint8_t on = CC2420_CCA_IS_1;
    //     PRINTF("CCA = %u\n", on);
    // }
}
