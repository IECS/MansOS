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

#ifndef MSP430_BEEPER_HAL_H
#define MSP430_BEEPER_HAL_H

#include <hil/udelay.h>
#include <hil/gpio.h>

#define beeperInit()  PIN_AS_OUTPUT(1, 2)
#define beeperToggle()  PIN_TOGGLE(1, 2)

enum {
    BEEPER_DEFAULT_FREQUENCY = 1000, // Hz
    BEEPER_DEFAULT_PERIOD = 1000000ul / BEEPER_DEFAULT_FREQUENCY, // microseconds
};

// note that the timing is not exactly accurate
static inline void beeperBeep(uint16_t ms) {
    uint32_t usec = ms * 800ul;
    while (usec >= BEEPER_DEFAULT_PERIOD) {
        beeperToggle();
        udelay(BEEPER_DEFAULT_PERIOD - 50);
        usec -= BEEPER_DEFAULT_PERIOD;
    }
}

static inline void beeperBeepEx(uint16_t ms, uint16_t frequency) {
    uint32_t usec = ms * 800ul;
    uint16_t period = (uint16_t) (1000000ul / frequency); // XXX: division
    uint16_t adjustedPeriod = period > 50 ? period - 50 : 0;
    while (usec >= period) {
        beeperToggle();
        udelay(adjustedPeriod);
        usec -= period;
    }
}

#endif
