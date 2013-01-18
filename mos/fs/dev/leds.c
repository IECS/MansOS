/*
 * Copyright (c) 2008-2011 the MansOS team. All rights reserved.
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
 * fs/dev/leds.c -- LEDs device file
 */

#include <stdbool.h>
#include <stddef.h>

#include <leds.h>
#include <fs/common.h>
#include <fs/types.h>
#include <kernel/defines.h>
#include <kernel/stdtypes.h>

#include "dev.h"
#include "leds.h"

/* File operations */
static bool    ledsOpen(void);
static ssize_t ledsRead(void *buf, size_t count);
static ssize_t ledsWrite(const void *buf, size_t count);
static bool    ledsClose(void);

const struct fsDevOperations fsDevLEDOps = {
    .open   = ledsOpen,
    .read   = ledsRead,
    .write  = ledsWrite,
    .close  = ledsClose
};

/* Implementation */

static bool ledsOpen(void)
{
    /* LEDs should have been initialized by now */
    return true;
}

#define LED_COUNT 4

/*
 * Read function. Returns: "0101\n".
 */
static ssize_t ledsRead(void *buf, size_t count)
{
    uint_t leds = ledsGet();
    size_t i;
    char *b = buf;

    count = MIN(LED_COUNT + 1, count);

    for (i = 0; i < count; i++)
    {
        if (i < LED_COUNT)
            b[i] = leds & (1U << (LED_COUNT - 1 - i)) ? '1' : '0';
        else
            b[i] = '\n';
    }

    return count;
}

/*
 * Write function. Expects a string of form "####", where each # is either of:
 *   '0'  Turn LED off
 *   '1'  Turn LED on
 *   'x'  Toggle LED
 *   '.'  Do not change LED state
 */
static ssize_t ledsWrite(const void *buf, size_t count)
{
    uint_t leds = ledsGet();
    size_t i;
    const char *b = buf;

    count = MIN(LED_COUNT + 1, count);

    for (i = 0; i < count; i++)
    {
        /* Consume also a trailing newline if present */
        if (i == LED_COUNT)
        {
            if (b[i] == '\n')
                continue;
            else
            {
                fsSetError(FS_ERR_IO);
                return -1;
            }
        }

        switch (b[i])
        {
            case '0':
                leds &= ~(1U << (LED_COUNT - 1 - i));
                break;
            case '1':
                leds |= 1U << (LED_COUNT - 1 - i);
                break;
            case 'x':
            case 'X':
                leds ^= 1U << (LED_COUNT - 1 - i);
            case '.':
                break;
            default:
                fsSetError(FS_ERR_IO);
                return -1;
        }
    }

    ledsSet(leds);
    return count;
}

static bool ledsClose(void)
{
    return true;
}
