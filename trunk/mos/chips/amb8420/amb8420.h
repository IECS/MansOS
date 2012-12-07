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

#ifndef MANSOS_AMB8420_H
#define MANSOS_AMB8420_H

#include <kernel/defines.h>
#include <hil/busywait.h>

#define AMB8420_MAX_PACKET_LEN        128

#define AMB8420_TX_POWER_MIN          0  // -38.75 dBm
#define AMB8420_TX_POWER_MAX          31 // 0 dBm

#define AMB8420_TRANSPARENT_MODE      0x00
#define AMB8420_COMMAND_MODE          0x10

#define AMB8420_CMD_DATA_REQ          0x00
#define AMB8420_CMD_DATAEX_REQ        0x01
#define AMB8420_CMD_DATAEX_IND        0x81
#define AMB8420_CMD_SET_MODE_REQ      0x04
#define AMB8420_CMD_RESET_REQ         0x05
#define AMB8420_CMD_SET_CHANNEL_REQ   0x06
#define AMB8420_CMD_SET_DESTNETID_REQ 0x07
#define AMB8420_CMD_SET_DESTADDR_REQ  0x08
#define AMB8420_CMD_SET_REQ           0x09
#define AMB8420_CMD_GET_REQ           0x0A
#define AMB8420_CMD_SERIALNO_REQ      0x0B
#define AMB8420_CMD_RSSI_REQ          0x0D
#define AMB8420_CMD_ERRORFLAGS_REQ    0x0E

#define AMB8420_REPLY_FLAG            0x40

#define AMB8420_START_DELIMITER       0x02

// nonvolatile variable positions in memory
#define MAC_NUM_RETRYS_POS            20
#define MAC_ADDR_MODE_POS             21
#define MAC_DST_NETID_POS             24
#define MAC_DST_ADDR_POS              25
#define MAC_SRC_NETID_POS             28
#define MAC_SRC_ADDR_POS              29
#define MAC_ACK_TIMEOUT_POS           32

#define PHY_PA_POWER_POS              41
#define PHY_DEFAULT_CHANNEL_POS       42
#define PHY_CCA_LEVEL_POS             43

// addressing modes
typedef enum {
    AMB8420_ADDR_MODE_NONE    = 0, // no address
    AMB8420_ADDR_MODE_ADDR    = 1, // 1 byte address
    AMB8420_ADDR_MODE_ADDRNET = 2, // 1 byte addr, 1 byte net
} AMB8420AddrMode_t;

#ifndef PLATFORM_ARDUINO
#define RTS_WAIT_TIMEOUT_TICKS        TIMER_100_MS
//#define RTS_WAIT_TIMEOUT_TICKS        TIMER_SECOND
#define AMB8420_WAIT_FOR_RTS_READY(ok) \
    BUSYWAIT_UNTIL(pinRead(AMB8420_RTS_PORT, AMB8420_RTS_PIN) == 0, RTS_WAIT_TIMEOUT_TICKS, ok)
#else
#define AMB8420_WAIT_FOR_RTS_READY(ok) \
    while ((ok = pinRead(AMB8420_RTS_PORT, AMB8420_RTS_PIN) == 0));
#endif

// ----------------------------------------
// mode switching

typedef struct AMB8420ModeContext_s {
    uint8_t sleepPin : 1, trxDisablePin : 1;
} AMB8420ModeContext_t;

#define AMB8420_SAVE_MODE(ctx) do {                                     \
        ctx.sleepPin = pinRead(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);  \
        ctx.trxDisablePin = pinRead(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN); \
    } while (0)
#define AMB8420_RESTORE_MODE(ctx) do {                                  \
        pinWrite(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN, ctx.sleepPin);  \
        pinWrite(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN, ctx.trxDisablePin); \
    } while (0)

#define AMB8420_ENTER_ACTIVE_MODE(ctx) do{ \
        AMB8420_SAVE_MODE(ctx);                                         \
        pinClear(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);    \
        pinClear(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);                \
    } while(0)
#define AMB8420_ENTER_STANDBY_MODE(ctx) do{ \
        AMB8420_SAVE_MODE(ctx);                                         \
        pinSet(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);      \
        pinClear(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);                \
    } while(0)
#define AMB8420_ENTER_WOR_MODE(ctx) do{ \
        AMB8420_SAVE_MODE(ctx);                                         \
        pinClear(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);    \
        pinSet(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);                  \
    } while(0)
#define AMB8420_ENTER_SLEEP_MODE(ctx) do{ \
        AMB8420_SAVE_MODE(ctx);                                         \
        pinSet(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);      \
        pinSet(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);                  \
    } while(0)

#define AMB8420_SWITCH_TO_TRANSPARENT_MODE(currentMode) \
    if ((currentMode) == AMB8420_COMMAND_MODE) amb8420ChangeMode()
#define AMB8420_SWITCH_TO_COMMAND_MODE(currentMode) \
    if ((currentMode) == AMB8420_TRANSPARENT_MODE) amb8420ChangeMode()

void amb8420Init(void) WEAK_SYMBOL;

void amb8420On(void);
void amb8420Off(void);

int amb8420Read(void *buf, uint16_t bufsize);
int amb8420Send(const void *header, uint16_t headerLen,
                 const void *data, uint16_t dataLen);

void amb8420Discard(void);

// set receive callback
typedef void (*AMB8420RxHandle)(void);
AMB8420RxHandle amb8420SetReceiver(AMB8420RxHandle);

// radio channel control
void amb8420SetChannel(int channel);

// transmit power control (0..31)
void amb8420SetTxPower(uint8_t power);

// get RSSI (Received Signal Strength Indication) of the last received packet
int8_t amb8420GetLastRSSI(void);

// get LQI (Link Quality Indication) of the last received packet. Return value is in [0..127]
uint8_t amb8420GetLastLQI(void);

// measure the current RSSI on air
int amb8420GetRSSI(void);

// returns true if CCA fails to detect radio interference
bool amb8420IsChannelClear(void);

void amb8420InitSerial(void);

void amb8420Reset(void);

int amb8420EnterAddressingMode(AMB8420AddrMode_t, uint8_t srcAddress);

bool amb8420SetDstAddress(uint8_t dstAddress);

// must be inline. Saves energy: used iff the radio chip is present, but not used.
static inline void amb8420EnsureOff(void)
{
    pinAsOutput(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
    pinAsOutput(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
    pinSet(AMB8420_TRX_DISABLE_PORT, AMB8420_TRX_DISABLE_PIN);
    pinSet(AMB8420_SLEEP_PORT, AMB8420_SLEEP_PIN);
}

#endif
