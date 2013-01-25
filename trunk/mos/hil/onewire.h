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
 * Generic 1-Wire driver
 *
 * Define ONEWIRE_PORT and ONEWIRE_PIN before including this file.
 */

#include <delay.h>
#include <digital.h>
#include <lib/codec/crc.h>

/* Common 1-Wire commands */
enum {
    ONEWIRE_SEARCH_ROM = 0xF0,
    ONEWIRE_READ_ROM   = 0x33
};

#define ONEWIRE_ROM_SIZE 8

/*
 * Set 1-Wire low or high.
 *
 * IMPORTANT: The GPIO pin must have a pullup resistor attached, either
 * internal or external.
 */
#define OUTP_0() ( \
    pinAsOutput(ONEWIRE_PORT, ONEWIRE_PIN), /* Enable master drive */     \
    pinClear(ONEWIRE_PORT, ONEWIRE_PIN))    /* Output 0 */
#define OUTP_1() ( \
    pinAsInput(ONEWIRE_PORT, ONEWIRE_PIN),  /* Disable master drive */    \
    pinSet(ONEWIRE_PORT, ONEWIRE_PIN))      /* Turn on internal MSP430 */ \
                                            /* pullup resistor, if any */

/* Read one bit. */
#define INP() pinRead(ONEWIRE_PORT, ONEWIRE_PIN)

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
        if (i == 0) {
            return;
        }
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
        if (i == 0) {
            return result;
        }
        i--;
        result >>= 1;
    } while (1);
}

static bool owReadROM(uint8_t buf[ONEWIRE_ROM_SIZE])
{
    int_t    i;
    unsigned acc;

    if (owreset()) return false; // fail

    Handle_t h;
    ATOMIC_START(h);
    owwriteb(ONEWIRE_READ_ROM);
    /* We receive 8 bytes in the reverse order, LSbyte first. */
    for (i = ONEWIRE_ROM_SIZE - 1; i >= 0; i--) {
        buf[i] = owreadb();
    }
    ATOMIC_END(h);

    acc = 0;
    for (i = ONEWIRE_ROM_SIZE - 1; i >= 1; i--) {
        acc = crc8Add(acc, buf[i]);
    }
    return acc == buf[0];
}
