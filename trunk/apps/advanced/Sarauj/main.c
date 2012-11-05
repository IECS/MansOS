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

//-------------------------------------------
// Calendar which displays Latvian namedays
//-------------------------------------------

#include <stdmansos.h>
#include "i2c.h"
#include "vd_names.h"

// we will use all the names as one string separated by spaces
struct month_t {
    uint8_t daycount;
    const char *names;
};

enum {
    MONTH_COUNT = 12
};

// copy of namedays for a particular day, stored in RAM
char dayCopy[MAX_NAMES_LEN];

const struct month_t NAMES[MONTH_COUNT] = {
    { 31, (char *) MONTH_1 },
    { 29, (char *) MONTH_2 },
    { 31, (char *) MONTH_3 },
    { 30, (char *) MONTH_4 },
    { 31, (char *) MONTH_5 },
    { 30, (char *) MONTH_6 },
    { 31, (char *) MONTH_7 },
    { 31, (char *) MONTH_8 },
    { 30, (char *) MONTH_9 },
    { 31, (char *) MONTH_10 },
    { 30, (char *) MONTH_11 },
    { 31, (char *) MONTH_12 }
};

// copy namedays for a particular day from program flash into ram
// increment source pointer to a position after \0 byte
void copyDayNames(const char **pgmSource, char *ramCopy) {
    const char *end = *pgmSource;
    char c;
    uint8_t i = 0;
    do {
        c = pgm_read_byte(end);
        ramCopy[i++] = c;
        ++end;
//        PRINTF("c, end: %x, %x\n", c, end);
    } while (c != 0);
//    PRINTF("EOW\n");

    (*pgmSource) = end;
}


///////////////////////////////////////////
// I2C stuff
///////////////////////////////////////////
#define slaveAddress  81        // The datasheet says A2 to write and A3 to read
// But that's 8 bit addressing, we use 7 bit.
// So shift 1 bit to the right and we get 81 decimal

#define statconReg 0x00        // Status control register is at address 0
#define secsReg 0x02           // Seconds register is at address 2
#define minsReg 0x03           // Minutes register is at address 3
#define hourReg 0x04           // Hours register is at address 4

#define yearReg 0x05
#define monthReg 0x06
#define dayReg 0x07

#define alarmcontReg 0x08      // Alarm control register is at address 8

int control = 0;      // The control register is defined like:
// bit 0 = timer flag
// bit 1 = alarm flag
// bit 2 = alarm enable
// bit 3 = mask flag
// bit 4 + 5  = function mode
// bit 6 = hold last count flag
// bit 7 = stop counting flag

int alarm = 0;        // The alarm control register is defined like:
// bit 0 + 1 + 2 = timer function
// bit 3 = timer interupt enable
// bit 4 + 5 = clock alarm function
// bit 6 = timer alarm enable
// bit 7 = alarm interupt enable

int seconds = 0x55;              // Variable that will hold the seconds but initially it holds the start second (50)
int minutes = 0x59;              // Variable that will hold the minutes but initially it holds the start minute (59)
int hours = 0x23;                // Variable that will hold the hours but initially it holds the start hour (23)

int year = 0x08;
int month = 0x08;
int day = 0x04;

#define CALENDAR_RESET_PORT 3
#define CALENDAR_RESET_PIN  2

#define resetPin 56             // reset pin for the PCF8593
#define intPin 57               // interrupt pin from PCF8593 to interupt 11 of Arduino

uint8_t i2cSetReg(uint8_t addr, uint8_t regAddr, uint8_t regVal) {
    uint8_t buf[2];
    buf[0] = regAddr;
    buf[1] = regVal;
    return i2cWrite(addr, buf, 2);
}

uint8_t i2cGetReg(uint8_t addr, uint8_t regAddr) {
    uint8_t buf = regAddr;
    buf = i2cWrite(addr, &buf, 1);
    if (buf != 0) return buf;

    return i2cRead(addr, &buf, 1) == 1 ? buf : 0;
}

void initI2CStuff() {
    // 2-wire
    i2cOn();
    pinAsData(CALENDAR_RESET_PORT, CALENDAR_RESET_PIN);
    pinAsOutput(CALENDAR_RESET_PORT, CALENDAR_RESET_PIN);
    pinClear(CALENDAR_RESET_PORT, CALENDAR_RESET_PIN);
    mdelay(10);
    pinSet(CALENDAR_RESET_PORT, CALENDAR_RESET_PIN);


    uint8_t r;
    r = i2cSetReg(slaveAddress,statconReg,control);
    r = i2cSetReg(slaveAddress,hourReg,hours);
    r = i2cSetReg(slaveAddress,minsReg,minutes);
    r = i2cSetReg(slaveAddress,secsReg,seconds);
    r = i2cSetReg(slaveAddress,yearReg,year);
    r = i2cSetReg(slaveAddress,monthReg,month);
    r = i2cSetReg(slaveAddress,dayReg,day);
}


void readSeconds () {
    // Read byte from seconds register and
    uint8_t tmp = i2cGetReg(slaveAddress, secsReg);  // put it in the month variable
    // (tmp >> 4) * 10 = (tmp >> 4) * (1 << 3 + 1 << 1) = (tmp >> 1) + (tmp >> 3)
    seconds = (tmp & 0xf) + ((tmp & 0xf0) >> 1) + ((tmp & 0xf0) >> 3);
}

void readMinutes () {
    // Read byte from minutes register and
    uint8_t tmp = i2cGetReg(slaveAddress, minsReg);  // put it in the month variable
    minutes = (tmp & 0xf) + ((tmp & 0xf0) >> 1) + ((tmp & 0xf0) >> 3);
}

void readHours () {
    // Read byte from hours register and
    uint8_t tmp = i2cGetReg(slaveAddress, hourReg);  // put it in the hours variable
    hours = (tmp & 0xf) + ((tmp & 0x30) >> 1) + ((tmp & 0x30) >> 3);
}

void readDay () {
    // Read byte from day register and
    uint8_t tmp = i2cGetReg(slaveAddress, dayReg);  // put it in the day variable
    day = (tmp & 0xf) + ((tmp & 0xf0) >> 1) + ((tmp & 0xf0) >> 3);
}

void readMonth () {
    // Read byte from month register and
    uint8_t tmp = i2cGetReg(slaveAddress, monthReg);  // put it in the month variable
    month = (tmp & 0xf) + ((tmp & 0x10) >> 1) + ((tmp & 0x10) >> 3);
}

void readYear () {
    // Read byte from year register and
    uint8_t tmp = i2cGetReg(slaveAddress, yearReg);  // put it in the year variable
    year = (tmp & 0xc0) >> 6;
}

void getTime() {
  readHours();
  readMinutes();
  readSeconds();
}

void getDate() {
  readYear();
  readMonth();
  readDay();
}

void printTime() {
    PRINTF("%i:%i:%i\n", hours, minutes, seconds);
}

void printDate() {
    PRINTF("%i-%i-%i ", year, month, day);
}


void appMain() {
    PRINTF("starting\n");

    initI2CStuff();

    while (1) {
        getTime();
        getDate();
        printDate();
        printTime();
        msleep(1000);
    }



    uint8_t i;
    // go through all months
    for (i = 0; i < MONTH_COUNT; ++i) {
        PRINTF("%i. menesis:\n", (i + 1));
        const char *nameStart = NAMES[i].names;
        uint8_t j;
        // go through all days
        for (j = 0; j < NAMES[i].daycount; ++j) {
            PRINTF("%i: ", (j + 1));
            copyDayNames(&nameStart, dayCopy);
            PRINTF("%s\n", dayCopy);
        }
    }
}
