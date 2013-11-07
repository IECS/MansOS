/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
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
 * SDI-12 software driver
 *
 * To use this driver, define SDI12_DATA_PORT and SDI12_DATA_PIN before
 * including this header.
 *
 * BIG WARNING: The quality of this driver should be improved
 *
 * TODO: Add timeouts to infinite loops
 * TODO: Add max retries to sdiCommand
 */

#if !defined(SDI12_RX_PORT) || !defined(SDI12_RX_PIN)
#  define SDI12_RX_PORT  SDI12_DATA_PORT
#  define SDI12_RX_PIN   SDI12_DATA_PIN
#endif
#if !defined(SDI12_TX_PORT) || !defined(SDI12_TX_PIN)
#  define SDI12_TX_PORT  SDI12_DATA_PORT
#  define SDI12_TX_PIN   SDI12_DATA_PIN
#endif

#include <ctype.h>

#include <delay.h>
#include <sleep.h>
#include <digital.h>
#include <leds.h>

#define SDI_WAKEUP_DELAY        15  // (ms); should be at least 12
#define SDI_SLEEP_DELAY         105 // (ms); should be at least 100
#define SDI_TIMING_DELAY        833 // (us); 1200 baud = 833.(3) us per bit
#define SDI_TIMING_DELAY_HALF   417 // (us)
#define SDI_PRECOMMAND_DELAY    8333 // (us)

#define SDI_MAX_RESPONSE_LENGTH 48 // TODO: Check this

#define SDI_ERROR               0xFF // Invalid character


//////////// FIXME ///////////////////
#ifdef PLATFORM_MIIMOTE
#define mydelay(us) udelay(100L*(us)/108)
#endif
//////////// FIXME ///////////////////


static inline void sdiDataOut(void)
{
    pinAsOutput(SDI12_TX_PORT, SDI12_TX_PIN);

#ifdef PLATFORM_MIIMOTE
    pinClear(4, 0);
#endif
}

static inline void sdiDataIn(void)
{
    pinAsInput(SDI12_RX_PORT, SDI12_RX_PIN);

#ifdef PLATFORM_MIIMOTE
    pinSet(4, 0);
#endif
}

static inline void sdiDataWrite(uint8_t val)
{
#ifndef SDI12_NO_INVERT_VALUES
    pinWrite(SDI12_TX_PORT, SDI12_TX_PIN, !val);
#else
    pinWrite(SDI12_TX_PORT, SDI12_TX_PIN, val);
#endif
}

static inline uint8_t sdiDataRead(void)
{
#ifndef SDI12_NO_INVERT_VALUES
    return !pinRead(SDI12_RX_PORT, SDI12_RX_PIN);
#else
    return pinRead(SDI12_RX_PORT, SDI12_RX_PIN);
#endif
}

static void sdiWakeup(void)
{
    // Send a break -- wake up sensors on the line
    // A command should be transmitted afterwards

    sdiDataOut();
    sdiDataWrite(0);
    msleep(SDI_WAKEUP_DELAY);
}

static void sdiSleep(void)
{
    // Put sensors in the sleep mode

    sdiDataOut();
    sdiDataWrite(1);
    msleep(SDI_SLEEP_DELAY);
}

static void sdiWriteByte(uint8_t val)
{
    // Start bit
    sdiDataWrite(0);
    mydelay(SDI_TIMING_DELAY);

    // 7 data bits
    uint8_t parity = 0;
    uint8_t i;
    for (i = 0; i < 7; i++)
    {
        parity ^= val;
        sdiDataWrite(val & 1);
        mydelay(SDI_TIMING_DELAY);
        val >>= 1;
    }

    // Even parity bit
    sdiDataWrite(parity & 1);
    mydelay(SDI_TIMING_DELAY);

    // Stop bit
    sdiDataWrite(1);
    mydelay(SDI_TIMING_DELAY);
}

bool z = false;
static uint8_t sdiReadByte(void)
{
    // Wait for start bit
    while (sdiDataRead() == 1);

    // We will try to hit the middle of each bit
    mydelay(SDI_TIMING_DELAY_HALF);

    // Start bit
    if (sdiDataRead() != 0)
    {
        return SDI_ERROR;
    }
    mydelay(SDI_TIMING_DELAY);

    // 7 data bits
    uint8_t res = 0, parity = 0;
    uint8_t i;
    for (i = 0; i < 7; i++)
    {
        res = (sdiDataRead() << 6) | (res >> 1);
        parity ^= res;
        mydelay(SDI_TIMING_DELAY);
    }

    // Even parity bit
    if (sdiDataRead() != ((parity >> 6) & 1))
    {
        return SDI_ERROR;
    }
    mydelay(SDI_TIMING_DELAY);

    // Stop bit
    if (sdiDataRead() != 1)
    {
        return SDI_ERROR;
    }

    return res;
}

static char sdiBuf[SDI_MAX_RESPONSE_LENGTH];

static const char *sdiCommand(const char *cmd)
{
    const char *res;

    sdiWakeup();

    sdiDataOut();
    sdiDataWrite(1);
    mydelay(SDI_PRECOMMAND_DELAY);

    do {
        sdiWriteByte(*cmd);
    } while (*cmd++ != '!'); // '!' terminates the command

    sdiDataIn();

    uint8_t *out = (uint8_t *)sdiBuf;
    do {
        if ((*out++ = sdiReadByte()) == SDI_ERROR)
        {
            res = NULL;
            goto end;
        }
    } while (out < sdiBuf + 2 || !(out[-2] == '\r' && out[-1] == '\n'));
    out[-2] = '\0';
    res = sdiBuf;

end:
    sdiSleep();

    return res;
}

// Write up to 9 parsed measurements to the supplied array
static void sdiMeasure(int16_t values[9])
{
    const char *s;

    while (!(s = sdiCommand("?M!")));

    int seconds = (s[1] - '0')*100 + (s[2] - '0')*10 + (s[3] - '0');
    int numValues = s[4] - '0';

    sleep(seconds);

    int i = 0;
    char dcmd[] = "?D0!";
    while (i < numValues)
    {
        while (!(s = sdiCommand(dcmd)));

        s++;
        bool frac;
        while (*s)
        {
            values[i] = 0;
            char sign = *s++;

            frac = false;
            while (isdigit(*s) || *s == '.')
            {
                if (isdigit(*s) && !frac)
                {
                    values[i] = values[i] * 10 + (*s - '0');
                }
                else if (*s == '.')
                {
                    frac = true;
                }
                s++;
            }
            if (sign == '-')
                values[i] = -values[i];
            i++;
        }

        dcmd[2]++;
    }
}

static void sdiInit(void)
{
#ifdef PLATFORM_MIIMOTE
    pinAsData(4, 0);
    pinAsOutput(4, 0);
#endif

    sdiSleep();
}
