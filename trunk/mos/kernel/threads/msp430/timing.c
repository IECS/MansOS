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

#include <platform.h>
#include "../timing.h"
#include <lib/assert.h>

void msp430TimerBSet(uint16_t ms)
{
    // assume the 'ms' value passed here is correct (below maximum supported by HW)
    ASSERT(ms < 15984); // 15984 * 4.1 ~= 0xfffe

    uint16_t ocr;
    if (ms > 0) {
        // 32768 ticks per second
        // => 32.768 ticks per millisecond
        // scaled by 8 => 4.096 ticks per millisecond
        // the following is equivalent to st * 4.1
        ocr = ms * 4 + ms / 10;
    } else {
        ms = 1;
        ocr = 1;
    }

    // Note: this means that jiffies will contain incorrect value when someone
    //       wakes from sleep because of an interrupt!
    // XXX: also, a rounding error is introduced here
    jiffies += ms2jiffies(ms);

    TBCCR0 = TBR + ocr;
}
