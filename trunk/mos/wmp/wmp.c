/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#include "wmp.h"
#include "stdmansos.h"
#include <lib/byteorder.h>
#include <lib/algo.h>
#include <lib/assert.h>
#include <fatfs/fatfs.h>
#include <stdio.h>
#include <sdstream.h>
#include <eeprom.h>
#include <timing.h>

//#define WMP_DEBUG DEBUG

// XXX
#define WMP_DEBUG 1

#if WMP_DEBUG
#define DPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define DPRINTF(...) do {} while (0)
#endif


static void wmpSendReply(void);
static uint8_t wmpCrc(void);

// -------------------------------------------------------

struct SerialPacket_s {
    uint8_t command;    // WMP command
    uint8_t argLen;     // promised argument length
    uint8_t argLenRead; // actual argument length read so far
    uint8_t arguments[99];
    uint8_t zero;
    uint8_t crc;        // XOR of packet fields
} PACKED;

typedef struct SerialPacket_s SerialPacket_t;
SerialPacket_t sp;

static bool commandMode;

static bool wmpSerialOutputEnabled = true;
static bool wmpSdCardOutputEnabled;
static bool wmpFileOutputEnabled;
static char wmpOutputFileName[13];
static FILE *outputFile;

// the generic function type for reading sensors
typedef int32_t (*WmpSensorReadFunction)(void);

// Description of a WMP-enabled sensor
typedef struct {
    WmpSensorType_t code;        // numerical code (NOT SEAL-compatible!)
    bool isRead;                 // whether is read in the last period
    const char *name;            // ASCII name of the sensor (SEAL-compatible)
    uint32_t period;             // set to 0 to disable reading
    int32_t lastReadValue;       // last reading value
    Alarm_t alarm;               // read alarm
    WmpSensorReadFunction func;  // reading function
} WmpSensor_t;

static uint8_t numReadSensors;
static uint8_t numTotalActiveSensors;
static bool csvFileFirstLinePending;

// sensor-reading functions
int32_t wmpReadLight(void) {
    return lightRead();
}

#if USE_HUMIDITY
int32_t wmpReadHumidity(void) {
    return humidityRead();
}
#endif

#if USE_ADC
int32_t wmpReadADC0(void) {
    return adcRead(0);
}

int32_t wmpReadADC1(void) {
    return adcRead(1);
}

int32_t wmpReadADC2(void) {
    return adcRead(2);
}

int32_t wmpReadADC3(void) {
    return adcRead(3);
}

int32_t wmpReadADC4(void) {
    return adcRead(4);
}

int32_t wmpReadADC5(void) {
    return adcRead(5);
}

int32_t wmpReadADC6(void) {
    return adcRead(6);
}

int32_t wmpReadADC7(void) {
    return adcRead(7);
}

int32_t wmpReadBattery(void) {
    return adcRead(ADC_INTERNAL_VOLTAGE);
}
#endif

static WmpSensor_t availableSensors[] = {
    {
        .code = WMP_SENSOR_LIGHT,
        .name = "Light",
        .func = wmpReadLight,
    },
#if USE_HUMIDITY
    {
        .code = WMP_SENSOR_HUMIDITY,
        .name = "Humidity",
        .func = wmpReadHumidity,
    },
#endif
#if USE_ADC
    // support up to 8 ADC channels
    {
        .code = WMP_SENSOR_ADC0,
        .name = "ADC0",
        .func = wmpReadADC0,
    },
    {
        .code = WMP_SENSOR_ADC1,
        .name = "ADC0",
        .func = wmpReadADC1,
    },
    {
        .code = WMP_SENSOR_ADC2,
        .name = "ADC2",
        .func = wmpReadADC2,
    },
    {
        .code = WMP_SENSOR_ADC3,
        .name = "ADC3",
        .func = wmpReadADC3,
    },
    {
        .code = WMP_SENSOR_ADC4,
        .name = "ADC4",
        .func = wmpReadADC4,
    },
    {
        .code = WMP_SENSOR_ADC5,
        .name = "ADC5",
        .func = wmpReadADC5,
    },
    {
        .code = WMP_SENSOR_ADC6,
        .name = "ADC6",
        .func = wmpReadADC6,
    },
    {
        .code = WMP_SENSOR_ADC7,
        .name = "ADC7",
        .func = wmpReadADC7,
    },
    {
        .code = WMP_SENSOR_BATTERY,
        .name = "Battery",
        .func = wmpReadBattery,
    }
#endif
};

// -------------------------------------------------------

static void writeCsvFileRecord(void)
{
    uint8_t i;
    char str[12];

    if (csvFileFirstLinePending) {
        // write first (header) line with sensor names
        fputs("\ntimestamp,", outputFile);
        for (i = 0; i < ARRAYLEN(availableSensors); ++i) {
            if (availableSensors[i].period != 0) {
                fputs(availableSensors[i].name, outputFile);
                fputc(',', outputFile);
            }
        }
        fputc('\n', outputFile);
        csvFileFirstLinePending = false;
    }

    // write timestamp first
    sprintf(str, "%ld", getSyncTimeSec());
    fputs(str, outputFile);
    fputc(',', outputFile);
    // write all active sensors
    for (i = 0; i < ARRAYLEN(availableSensors); ++i) {
        if (availableSensors[i].isRead) {
            sprintf(str, "%ld", availableSensors[i].lastReadValue);
            fputs(str, outputFile);
            fputc(',', outputFile);
            availableSensors[i].isRead = false;
        }
    }
    // end with a newline
    fputc('\n', outputFile);
    numReadSensors = 0;
}

void wmpReadSensor(void *sensor_)
{
    WmpSensor_t *sensor = (WmpSensor_t *) sensor_;
    static char buffer[32];

    sensor->lastReadValue = sensor->func();
    snprintf(buffer, sizeof(buffer), "%s=%ld\n", sensor->name, sensor->lastReadValue);

    PRINTF("File enabled=%s, output=%p\n", wmpFileOutputEnabled ? "yes" : "no", outputFile);

    // handle serial output
    if (wmpSerialOutputEnabled) {
        serialSendString(PRINTF_SERIAL_ID, buffer);
    }
    // handle output to SD card
    if (wmpSdCardOutputEnabled) {
#if USE_SDCARD_STREAM
        sdStreamWriteRecord(buffer, strlen(buffer), false);
#endif
    }
    // handle output to file 
    else if (wmpFileOutputEnabled && outputFile) {
        if (!sensor->isRead) {
            sensor->isRead = true;
            numReadSensors++;
        } else {
            PRINTF("sensor already read!\n");
        }
        PRINTF("numReadSensors=%u, total=%u\n", numReadSensors, numTotalActiveSensors);
        if (numReadSensors >= numTotalActiveSensors) {
            writeCsvFileRecord();
        }
    }
    alarmSchedule(&sensor->alarm, sensor->period);
}

// -------------------------------------------------------

static uint8_t wmpCrc(void) {
    uint8_t i;
    uint8_t crc = WMP_START_CHARACTER;
    crc ^= sp.command;
    crc ^= sp.argLen;
    for (i = 0; i < sp.argLen; ++i) {
        crc ^= sp.arguments[i];
    }
    // crc ^= sp.crc;
    return crc;
}

static void wmpSendReply(void)
{
    sp.command |= WMP_CMD_REPLY_FLAG;
    sp.crc = wmpCrc();

    serialSendByte(PRINTF_SERIAL_ID, WMP_START_CHARACTER);
    serialSendByte(PRINTF_SERIAL_ID, sp.command);
    serialSendByte(PRINTF_SERIAL_ID, sp.argLen);
    serialSendData(PRINTF_SERIAL_ID, sp.arguments, sp.argLen);
    serialSendByte(PRINTF_SERIAL_ID, sp.crc);
}

// -------------------------------------------------------

static inline bool checkArgLen(uint8_t minLen)
{
    if (sp.argLen < minLen) {
        DPRINTF("checkArgLen: too short!\n");
        return false;
    }
    if (sp.argLen > sizeof(sp.arguments)) {
        DPRINTF("checkArgLen: too long!\n");
        return false;
    }
    return true;
}

static void processLedSet(void)
{
    uint8_t ledNr = sp.arguments[0];
    uint8_t onOff = sp.arguments[1];
    uint8_t returnCode = WMP_ERROR;
    if (ledNr > 3) {
        goto error;
    }
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

    eepromWrite(WMP_EEPROM_LED_BASE + ledNr, &onOff, 1);

    returnCode = WMP_SUCCESS;
  error:
    sp.arguments[0] = returnCode;
    sp.argLen = 1;
    wmpSendReply();
}

static void processLedGet(void)
{
    uint8_t ledNr = sp.arguments[0];
    uint8_t result;
    switch (ledNr) {
    case 0:
        result = led0Get();
        break;
    case 1:
        result = led1Get();
        break;
    case 2:
        result = led2Get();
        break;
    case 3:
        result = led3Get();
        break;
    default:
        result = 0;
    }

    sp.arguments[1] = result ? 1 : 0;
    sp.argLen = 2;
    wmpSendReply();
}

static WmpSensor_t *findSensor(uint8_t code)
{
    uint8_t i;
    for (i = 0; i < ARRAYLEN(availableSensors); ++i) {
        if (availableSensors[i].code == code) {
            return &availableSensors[i];
        }
    }
    return NULL;
}

static void processSensorSet(void)
{
    WmpSensor_t *sensor = findSensor(sp.arguments[0]);
    uint8_t returnCode = WMP_ERROR;
    if (!sensor) goto error;

    uint32_t oldPeriod = sensor->period;
    sensor->period = le32read(sp.arguments + 1);

    if ((oldPeriod == 0) != (sensor->period == 0)) {
        if (oldPeriod == 0) numTotalActiveSensors++;
        else {
            ASSERT(numTotalActiveSensors);
            numTotalActiveSensors--;
            sensor->isRead = false;
        }
        csvFileFirstLinePending = true;
    }

    if (sensor->period) {
        alarmSchedule(&sensor->alarm, sensor->period);
    } else {
        alarmRemove(&sensor->alarm);
    }

    eepromWrite(WMP_EEPROM_SENSOR_BASE + sensor->code * 4, &sensor->period, 4);

    returnCode = WMP_SUCCESS;
  error:
    sp.arguments[0] = returnCode;
    sp.argLen = 1;
    wmpSendReply();
}

static void processSensorGet(void)
{
    WmpSensor_t *sensor = findSensor(sp.arguments[0]);
    if (!sensor) return;

    sp.argLen = 5;
    le32write(sp.arguments + 1, sensor->period);
    wmpSendReply();
}

static void processOutputSet(void)
{
    uint8_t outputNr = sp.arguments[0];
    uint8_t value =  sp.arguments[1];
    uint8_t returnCode = WMP_ERROR;
    if (outputNr > WMP_OUTPUT_FILE) {
        goto error;
    }
    switch (outputNr) {
    case WMP_OUTPUT_SERIAL:
        wmpSerialOutputEnabled = value;
        break;
    case WMP_OUTPUT_SDCARD:
        wmpSdCardOutputEnabled = value;
        break;
    case WMP_OUTPUT_FILE:
        wmpFileOutputEnabled = value;
        break;
    }

    uint16_t addr = WMP_EEPROM_OUTPUT_BASE + outputNr;
    eepromWrite(addr, &value, 1);

    returnCode = WMP_SUCCESS;
  error:
    sp.arguments[0] = returnCode;
    sp.argLen = 1;
    wmpSendReply();
}

static void processOutputGet(void)
{
    uint8_t outputNr = sp.arguments[0];
    uint8_t result;
    switch (outputNr) {
    case WMP_OUTPUT_SERIAL:
        result = wmpSerialOutputEnabled;
        break;
    case WMP_OUTPUT_SDCARD:
        result = wmpSdCardOutputEnabled;
        break;
    case WMP_OUTPUT_FILE:
        result = wmpFileOutputEnabled;
        break;
    default:
        result = 0;
    }
    
    sp.arguments[1] = result;
    sp.argLen = 2;
    wmpSendReply();
}

static void processAddressSet(void)
{
#if USE_ADDRESSING
    localAddress = le16read(sp.arguments);
    sp.arguments[0] = WMP_SUCCESS;
    eepromWrite(WMP_EEPROM_NETADDR_BASE, &localAddress, 2);
#else
    sp.arguments[0] = WMP_ERROR;
#endif
    sp.argLen = 1;
    wmpSendReply();
}

static void processAddressGet(void)
{
#if USE_ADDRESSING
    le16write(sp.arguments, localAddress);
    sp.argLen = 2;
#else
    sp.arguments[0] = WMP_ERROR;
    sp.argLen = 1;
#endif
    wmpSendReply();
}

static void processFilenameSet(void)
{
    uint8_t returnCode = WMP_ERROR;
    uint8_t len = min(sizeof(wmpOutputFileName) - 1, sp.argLen);
    memcpy(wmpOutputFileName, sp.arguments, len);
    wmpOutputFileName[len] = '\0';

    if (outputFile) {
        fclose(outputFile);
        outputFile = NULL;
    }

    if (wmpOutputFileName[0]) {
        outputFile = fopen(wmpOutputFileName, "a");
        if (!outputFile) goto error;
    }

    eepromWrite(WMP_EEPROM_FILE_BASE, wmpOutputFileName, 12);

    returnCode = WMP_SUCCESS;
  error:
    sp.arguments[0] = returnCode;
    sp.argLen = 1;
    wmpSendReply();
}

static void processFilenameGet(void)
{
    strcpy((char *)sp.arguments, wmpOutputFileName);
    sp.argLen = strlen(wmpOutputFileName);
    wmpSendReply();
}

// TODO: do this properly, e.g. with a callback
static void processFilelistGet(void)
{
    sp.arguments[0] = '\0';
#if USE_FATFS && USE_SDCARD
    sp.argLen = fatFsGetFiles((char *) sp.arguments, sizeof(sp.arguments));
#else
    sp.argLen = 0;
#endif
    wmpSendReply();
}

// TODO: do this properly, e.g. with a callback
static void processFileGet(void)
{
    sp.arguments[sp.argLen] = '\0';
    FILE *f = fopen((char *) sp.arguments, "r");
    sp.argLen = 1;
    sp.arguments[0] = WMP_ERROR;
    if (f) {
        size_t len = fread(sp.arguments, 1, sizeof(sp.arguments), f);
        if (len >= 0) {
            sp.argLen = len;
        }
        fclose(f);
    }
    wmpSendReply();
}

static void wmpProcessCommand(void)
{
    DPRINTF("got command %u\n", (uint16_t) sp.command);
    DPRINTF("    argLen=%u\n", (uint16_t) sp.argLen);
    DPRINTF("    *arg=%d\n", (uint16_t) sp.arguments[0]);
    DPRINTF("    crc=%u\n", (uint16_t) sp.crc);

    switch (sp.command) {
    case WMP_CMD_SET_LED:
        if (!checkArgLen(2)) return;
        processLedSet();
        break;
    case WMP_CMD_GET_LED:
        if (!checkArgLen(1)) return;
        processLedGet();
        break;
    case WMP_CMD_SET_SENSOR:
        if (!checkArgLen(5)) return;
        processSensorSet();
        break;
    case WMP_CMD_GET_SENSOR:
        if (!checkArgLen(1)) return;
        processSensorGet();
        break;
    case WMP_CMD_SET_OUTPUT:
        if (!checkArgLen(1)) return;
        processOutputSet();
        break;
    case WMP_CMD_GET_OUTPUT:
        if (!checkArgLen(1)) return;
        processOutputGet();
        break;
    case WMP_CMD_SET_ADDR:
        if (!checkArgLen(2)) return;
        processAddressSet();
        break;
    case WMP_CMD_GET_ADDR:
        processAddressGet();
        break;
    case WMP_CMD_SET_FILENAME:
        if (!checkArgLen(1)) return;
        processFilenameSet();
        break;
    case WMP_CMD_GET_FILENAME:
        processFilenameGet();
        break;
    case WMP_CMD_GET_FILELIST:
        processFilelistGet();
        break;
    case WMP_CMD_GET_FILE:
        if (!checkArgLen(1)) return;
        processFileGet();
        break;
    case WMP_CMD_SET_DAC:
        if (!checkArgLen(4)) return;
        // TODO
        break;
    case WMP_CMD_GET_DAC:
        if (!checkArgLen(1)) return;
        // TODO
        break;
    }
}

static void wmpSerialReceive(uint8_t x) {
//    DPRINTF("got %x\n", (uint16_t) x);
    enum State {
        READ_START_CHARACTER,
        READ_COMMAND,
        READ_ARG_LEN,
        READ_ARGS,
        READ_CRC,
    };
    static enum State state = READ_START_CHARACTER;
    switch (state) {
    case READ_START_CHARACTER:
        if (x == WMP_START_CHARACTER) {
            commandMode = true;
            state = READ_COMMAND;
        }
        break;
    case READ_COMMAND:
        sp.command = x;
        state = READ_ARG_LEN;
        break;
    case READ_ARG_LEN:
        sp.argLen = x;
        if (sp.argLen > sizeof(sp.arguments)) {
            PRINTF("Command too long: %u byte arguments!\n", (uint16_t) sp.argLen);
            state = READ_START_CHARACTER;
            break;
        }
        sp.argLenRead = 0;
        if (sp.argLen) {
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
        sp.crc = x;
        if (wmpCrc() == sp.crc) {
            wmpProcessCommand();
        } else {
            DPRINTF("bad crc %u\n", (uint16_t) sp.crc);
        }
        state = READ_START_CHARACTER;
        commandMode = false;
        break;
    }
}

void wmpInit(void)
{
    serialSetReceiveHandle(PRINTF_SERIAL_ID, wmpSerialReceive);

    // read config values from flash
    uint32_t key;
    eepromRead(0, &key, 4);
    bool eepromOK = (key == WMP_EEPROM_MAGIC_KEY);

    uint16_t i;
    for (i = 0; i < ARRAYLEN(availableSensors); ++i) {
        WmpSensor_t *sensor = &availableSensors[i];
        alarmInit(&sensor->alarm, wmpReadSensor, sensor);

        uint16_t addr = WMP_EEPROM_SENSOR_BASE + sensor->code * 4;
        if (eepromOK) {
            eepromRead(addr, &sensor->period, 4);
            if (sensor->period) {
                alarmSchedule(&sensor->alarm, sensor->period);
                numTotalActiveSensors++;
            }
        } else {
            eepromWrite(addr, &sensor->period, 4);
        }
    }

    if (eepromOK) {
        eepromRead(WMP_EEPROM_OUTPUT_BASE + WMP_OUTPUT_SERIAL, &wmpSerialOutputEnabled, 1);
        eepromRead(WMP_EEPROM_OUTPUT_BASE + WMP_OUTPUT_SDCARD, &wmpSdCardOutputEnabled, 1);
        eepromRead(WMP_EEPROM_OUTPUT_BASE + WMP_OUTPUT_FILE, &wmpFileOutputEnabled, 1);
        if (wmpFileOutputEnabled) {
            wmpSdCardOutputEnabled = false;
        }
        eepromRead(WMP_EEPROM_FILE_BASE, &wmpOutputFileName, 12);
        if (wmpOutputFileName[0]) {
            outputFile = fopen(wmpOutputFileName, "a");
        }
    } else {
        eepromWrite(WMP_EEPROM_OUTPUT_BASE + WMP_OUTPUT_SERIAL, &wmpSerialOutputEnabled, 1);
        eepromWrite(WMP_EEPROM_OUTPUT_BASE + WMP_OUTPUT_SDCARD, &wmpSdCardOutputEnabled, 1);
        eepromWrite(WMP_EEPROM_OUTPUT_BASE + WMP_OUTPUT_FILE, &wmpFileOutputEnabled, 1);
        eepromWrite(WMP_EEPROM_FILE_BASE, "", 1);
    }

    if (numTotalActiveSensors) {
        csvFileFirstLinePending = true;
    }

    bool on;
    if (eepromOK) {
        eepromRead(WMP_EEPROM_LED_BASE + 0, &on, 1);
        led0Set(on);
        eepromRead(WMP_EEPROM_LED_BASE + 1, &on, 1);
        led1Set(on);
        eepromRead(WMP_EEPROM_LED_BASE + 2, &on, 1);
        led2Set(on);
        eepromRead(WMP_EEPROM_LED_BASE + 3, &on, 1);
        led3Set(on);
    } else {
        on = false;
        eepromWrite(WMP_EEPROM_LED_BASE + 0, &on, 1);
        eepromWrite(WMP_EEPROM_LED_BASE + 1, &on, 1);
        eepromWrite(WMP_EEPROM_LED_BASE + 2, &on, 1);
        eepromWrite(WMP_EEPROM_LED_BASE + 3, &on, 1);
    }

#if USE_ADDRESSING
    if (eepromOK) {
        eepromRead(WMP_EEPROM_NETADDR_BASE, &localAddress, 2);
    } else {
        eepromWrite(WMP_EEPROM_NETADDR_BASE, &localAddress, 2);
    }
#endif

    key = WMP_EEPROM_MAGIC_KEY;
    eepromWrite(0, &key, 4);
}
