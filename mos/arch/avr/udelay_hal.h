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

// ATMega delay functions partially copied from Arduino
// http://arduino.cc

/*
 Partial implementation of the Wiring API for the ATmega8.
 Part of Arduino - http://www.arduino.cc/
 
 Copyright (c) 2005-2006 David A. Mellis
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General
 Public License along with this library; if not, write to the
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA  02111-1307  USA
 */


#ifndef UDELAY_HAL_H
#define UDELAY_HAL_H

#include <kernel/stdtypes.h>
#include <kernel/defines.h>


/* Delay for the given number of microseconds.  Assumes a 8 or 16 MHz clock. */
// Warning: Max delay time is 16384us = 16.384ms!
static inline void udelay(uint16_t us)
{
//    Handle_t h;

    // calling avrlib's delay_us() function with low values (e.g. 1 or
    // 2 microseconds) gives delays longer than desired.
    //delay_us(us);

#if CPU_MHZ >= 16
    // for the 16 MHz clock on most Arduino boards

    // for a one-microsecond delay, simply return.  the overhead
    // of the function call yields a delay of approximately 1 1/8 us.
    if (--us == 0)
        return;

    // the following loop takes a quarter of a microsecond (4 cycles)
    // per iteration, so execute it four times for each microsecond of
    // delay requested.
    us <<= 2;

    // account for the time taken in the preceeding commands.
    us -= 2;
#else
    // for the 8 MHz internal clock on the ATmega168

    // for a one- or two-microsecond delay, simply return.  the overhead of
    // the function calls takes more than two microseconds.  can't just
    // subtract two, since us is unsigned; we'd overflow.
    if (--us == 0)
        return;
    if (--us == 0)
        return;

    // the following loop takes half of a microsecond (4 cycles)
    // per iteration, so execute it twice for each microsecond of
    // delay requested.
    us <<= 1;

    // partially compensate for the time taken by the preceeding commands.
    // we can't subtract any more than this or we'd overflow w/ small delays.
    us--;
#endif

//    ATOMIC_START(h);
    // busy wait
    ASM_VOLATILE (
        "1: sbiw %0,1" "\n\t" // 2 cycles
        "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
    );
//    ATOMIC_END(h);
}


// we lose approximately 3us on mdelay header & footer + 1ms on each udelay call
static inline void mdelay(uint16_t ms) {
    // max udelay = 16768us
    while (ms > 16) {
        ms -= 16;
        udelay(15999); // 15999us = 16ms - 1us (which is lost on call)
    }
    udelay(ms * 1000 - 3); // regain the lost 3us
}

#endif
