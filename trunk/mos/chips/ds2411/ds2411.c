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

#include "ds2411.h"
#include <hil/udelay.h>
#include <hil/gpio.h>
#include <lib/codec/crc.h>
#include <string.h>
#include "platform.h"

#if SNUM_CHIP == SNUM_DS2411

static uint8_t ds2411[SERIAL_NUMBER_SIZE];
static bool ds2411Ok;

// application is responsible for giving enough bytes
bool ds2411Get(uint8_t *result) {
    memcpy(result, ds2411, SERIAL_NUMBER_SIZE);
    return ds2411Ok;
}

bool ds2411SnumMatches(const uint8_t *snum) {
    return ds2411Ok && memcmp(ds2411, snum, SERIAL_NUMBER_SIZE) == 0;
}

/*
 * Set 1-Wire low or high.
 *
 * IMPORTANT: The GPIO pin must have a pullup resistor attached, either
 * internal or external.
 */
#define OUTP_0() ( \
    pinAsOutput(DS2411_PORT, DS2411_PIN), /* Enable master drive */ \
    pinClear(DS2411_PORT, DS2411_PIN))    /* Output 0 */
#define OUTP_1() ( \
    pinAsInput(DS2411_PORT, DS2411_PIN),  /* Disable master drive */ \
    pinSet(DS2411_PORT, DS2411_PIN))      /* Turn on pullup resistor, if any */

/* Read one bit. */
#define INP() pinRead(DS2411_PORT, DS2411_PIN)

/*
 * Recommended delay times in us.
 */
#define udelay_tA() udelay_6()
/*      tA 6               max 15 */
#define tB 64
#define tC 60           /* max 120 */
#define tD 10
#define tE 9            /* max 12 */
#define tF 55
#define tG 0
#define tH 480
#define tI 70
#define tJ 410

static int
owreset(void)
{
    int result;
    OUTP_0();
    udelay(tH);
    OUTP_1();           /* Releases the bus */
    udelay(tI);
    result = INP();
    udelay(tJ);
    return result;
}

static void
owwriteb(unsigned byte)
{
    int i = 7;
    do {
        if (byte & 0x01) {
            OUTP_0();
            udelay_tA();
            OUTP_1();           /* Releases the bus */
            udelay(tB);
        } else {
            OUTP_0();
            udelay(tC);
            OUTP_1();           /* Releases the bus */
            udelay(tD);
        }
        if (i == 0)
            return;
        i--;
        byte >>= 1;
    } while (1);
}

static unsigned
owreadb(void)
{
    unsigned result = 0;
    int i = 7;
    do {
        OUTP_0();
        udelay_tA();
        OUTP_1();           /* Releases the bus */
        udelay(tE);
        if (INP())
            result |= 0x80;     /* LSbit first */
        udelay(tF);
        if (i == 0)
            return result;
        i--;
        result >>= 1;
    } while (1);
}

int ds2411Init() {
    unsigned family, crc, acc;
    uint_t i;
    uint8_t buffer[SERIAL_NUMBER_SIZE];

    if (owreset()) return 0; // fail

    Handle_t h;
    ATOMIC_START(h);
    owwriteb(0x33);     /* Read ROM command. */
    family = owreadb();
    /* We receive 6 bytes in the reverse order, LSbyte first. */
    for (i = 7; i >= 2; i--) {
        buffer[i] = owreadb();
    }
    crc = owreadb();
    ATOMIC_END(h);

    if (family != 0x01) return 0; // fail

    acc = crc8Add(0x0, family);
    for (i = 7; i >= 2; i--) {
        acc = crc8Add(acc, buffer[i]);
    }
    if (acc != crc) return 0; // fail

    ds2411Ok = true;
    memcpy(ds2411, buffer, SERIAL_NUMBER_SIZE);
#ifdef PLATFORM_TELOSB
    // 00:12:75    Moteiv    # Moteiv Corporation
    ds2411[0] = 0x00;
    ds2411[1] = 0x12;
    ds2411[2] = 0x75;
#endif

    return 1;
}

#endif // SNUM_CHIP == SNUM_DS2411
