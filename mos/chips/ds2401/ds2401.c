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

#include "ds2401.h"
#include <delay.h>
#include <kernel/defines.h>
#include <lib/codec/crc.h>
#include <string.h>
#include "platform.h"

#if SNUM_CHIP == SNUM_DS2401

static uint8_t ds2401[SERIAL_NUMBER_SIZE];
static bool ds2401Ok;

// application is responsible for giving enough bytes
bool ds2401Get(uint8_t *result) {
    memcpy(result, ds2401, SERIAL_NUMBER_SIZE);
    return ds2401Ok;
}

bool ds2401SnumMatches(const uint8_t *snum) {
    return ds2401Ok && memcmp(ds2401, snum, SERIAL_NUMBER_SIZE) == 0;
}

/* 1-wire is at p4.7 */
#define PIN BV(7)

#define PIN_INIT() {                                            \
        P4DIR &= ~PIN;      /* p4.7 in, resistor pull high */   \
        P4OUT &= ~PIN;      /* p4.7 == 0 but still input */     \
    }

/* Set 1-Wire low or high. */
#define OUTP_0() (P4DIR |=  PIN) /* output and p4.7 == 0 from above */
#define OUTP_1() (P4DIR &= ~PIN) /* p4.7 in, external resistor pull high */

/* Read one bit. */
#define INP() (P4IN & PIN)

/*
 * Recommended delay times in us.
 */
#define udelay_tA() udelay_6()
#define tA 10         /*      max 15 */
#define tB 100
#define tC 100           /* max 120 */
#define tD 10
#define tE 30            /* max 15 */
#define tF 60
#define tG 0
#define tH 1500
#define tI 50
#define tJ 360
#define tK NOP3()

static int owreset(void)
{
    uint8_t result;
    OUTP_0();
    udelay(tH);
    OUTP_1();           /* Releases the bus */
    udelay(tI);
    result = INP();
    udelay(tJ);
    return result;
}

static void owwriteb(unsigned byte)
{
    int i = 7;
    do {
        if (byte & 0x01) {
            OUTP_0();
            udelay(tA);
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

static unsigned owreadb(void)
{
    unsigned result = 0;
    int i = 7;
    do {
        OUTP_0();
        tK;
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

int ds2401Init() {
    unsigned family, crc, acc;
    uint_t i;
    uint8_t buffer[SERIAL_NUMBER_SIZE];

    if (ds2401Ok) return 1;

    PIN_INIT();
    udelay(50000);
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

    ds2401Ok = true;
    memcpy(ds2401, buffer, SERIAL_NUMBER_SIZE);

    //
    // Note! This chip is used by SADmote. Since we don't have OUI registered,
    // we are using all zeros instead and just hope it will not collide with
    // any other known equipment.
    //
    ds2401[0] = 0x00;
    ds2401[1] = 0x00;
    // ds2401[2] is read from the chip

    return 1;
}

#endif // SNUM_CHIP == SNUM_DS2401
