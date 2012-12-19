/**
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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

#include <stdmansos.h>
#include <string.h>
#include <lib/codec/crc.h>
#include <net/mac.h>
#include <lib/activemsg.h>
#include <random.h>
#include <sensors.h>
#include <sdstream.h>
#include "protocol.h"

#if !PLATFORM_TELOSB
#include <ads8638/ads8638.h>
#else
void ads8638Read(uint16_t *v)
{
   *v = 1;
}
void ads8638SelectChannel(uint16_t c, uint16_t r)
{
}
#define led0On  redLedOn
#define led1On  greenLedOn
#define led2On  blueLedOn
#define led0Set redLedSet
#define led1Set greenLedSet
#define led2Set blueLedSet
#define led3Set(x)

#define ADS8638_CHANNEL_0  0
#define ADS8638_RANGE_2_5V 0

#endif


#if DEBUG
#define DPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define DPRINTF(...) do {} while (0)
#endif

// -----------------------------------
// variables controllable via protocol

static uint32_t adcPeriod = 1000;
static uint16_t adcChannel;
static bool printToSerial = true;
static bool writeToSdCard = false;

static bool commandMode;

// -----------------------------------

typedef struct SerialPacket_s {
    uint8_t command;
    uint8_t argLen;
    uint8_t argLenRead;
    uint8_t arguments[100];
    uint8_t crc;
} SerialPacket_t;

volatile SerialPacket_t sp;

// -------------------------------------------

static inline bool checkArgLen(uint8_t minLen)
{
    if (sp.argLen < minLen) {
        DPRINTF("checkArgLen: too short!\n");
        return false;
    }
    return true;
}

static void processLedCommand(void)
{
//    led1On();
//    led2On();

    uint8_t ledNr = sp.arguments[0];
    uint8_t onOff = sp.arguments[1];
//    onOff = 1;
    switch (ledNr) {
    case 0:
        led0Set(onOff);
        break;
    case 1:
        led1Set(onOff);
        break;
    case 2:
        led2Set(onOff);
        break;
    case 3:
        led3Set(onOff);
        break;
    }
}

static void processAdcCommand(void)
{
    uint8_t channel = sp.arguments[0];
    if (channel < 8) {
        adcChannel = channel;
    } else {
        DPRINTF("wrong adc channel!\n");
    }

    uint32_t period;
    memcpy(&period, &sp.arguments[1], 4);
    if (period > 0 && period < 1000000000ul) {
        adcPeriod = period;
    } else {
        DPRINTF("wrong adc period!\n");
    }

//    adcPeriod = 1000; // XXX
}

static void processDacCommand(void)
{
    // TODO
}

static void processSerialCommand(void)
{
    printToSerial = sp.arguments[0];
}

static void processSdCardCommand(void)
{
    writeToSdCard = sp.arguments[0];
}

static void processCommand(void)
{
//    led3On();
    DPRINTF("got command %d\n", sp.command);
    switch (sp.command) {
    case CMD_LED_CONTROL:
//        led0On();
        if (!checkArgLen(2)) return;
        processLedCommand();
        break;
    case CMD_ADC_CONTROL:
        if (!checkArgLen(5)) return;
        processAdcCommand();
        break;
    case CMD_DAC_CONTROL:
        if (!checkArgLen(3)) return;
        processDacCommand();
        break;
    case CMD_SERIAL_CONTROL:
        if (!checkArgLen(1)) return;
        processSerialCommand();
        break;
    case CMD_SD_CARD_CONTROL:
        if (!checkArgLen(1)) return;
        processSdCardCommand();
        break;
    }
}

static bool crcOK(void) {
    uint8_t i;
    uint8_t crc = START_CHARACTER;
    crc ^= sp.command;
    crc ^= sp.argLen;
    for (i = 0; i < sp.argLen; ++i) {
        crc ^= sp.arguments[i];
    }
    crc ^= sp.crc;
//    return crc == 0;
    return true;
}

static void serialReceive(uint8_t x) {
//    led0On();
    DPRINTF("got %x\n", (uint16_t) x);
    static enum {
        READ_START_CHARACTER,
        READ_COMMAND,
        READ_ARG_LEN,
        READ_ARGS,
        READ_CRC,
    } state = READ_START_CHARACTER;

    switch (state) {
    case READ_START_CHARACTER:
        if (x == START_CHARACTER) {
            commandMode = true;
            state = READ_COMMAND;
        }
        break;
    case READ_COMMAND:
//        led1On();
        sp.command = x;
        state = READ_ARG_LEN;
        break;
    case READ_ARG_LEN:
        sp.argLen = x;
        sp.argLenRead = 0;
        if (sp.argLenRead) {
            state = READ_ARGS;
        } else {
            state = READ_CRC;
        }
        break;
    case READ_ARGS:
        sp.arguments[sp.argLenRead++] = x;
        if (sp.argLenRead == sp.argLen) {
            state = READ_CRC;
        }
        break;
    case READ_CRC:
//        led2On();
        sp.crc = x;
        if (crcOK()) {
            processCommand();
        } else {
            DPRINTF("bad crc\n");
        }
        state = READ_START_CHARACTER;
        commandMode = false;
        break;
    }
}

// -------------------------------------------

static Alarm_t adcAlarm;

static void adcCallback(void *x)
{
    uint16_t val;
    ads8638Read(&val);

    // PRINTF("channel=%u period=%lu printToSerial=%u writeToSdCard=%u\n",
    //         adcChannel, adcPeriod, (uint16_t)printToSerial, (uint16_t)writeToSdCard);

    if (printToSerial && !commandMode) {
        PRINTF("%u\n", val);
    }
    if (writeToSdCard) {
//        sdStreamWriteRecord(&val, sizeof(val), true);
    }
    alarmSchedule(&adcAlarm, adcPeriod); 
}

void appMain(void)
{
    // led0Set(1);
    // led1Set(0);

    adcChannel = ADS8638_CHANNEL_0;
    alarmInit(&adcAlarm, adcCallback, NULL);
    alarmSchedule(&adcAlarm, adcPeriod);
    ads8638SelectChannel(adcChannel, ADS8638_RANGE_2_5V);
    serialSetReceiveHandle(PRINTF_SERIAL_ID, serialReceive);
    serialEnableRX(PRINTF_SERIAL_ID);
}
