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

//---------------------------------------------------------------
// Analog sensor debug tool
//---------------------------------------------------------------

#include "stdmansos.h"

// Samples one ADC channel as fast as possible
// Outputs sampled values to UART, each on a separate line
// Outputs also performance statistics once every 10000 samples

enum {
    ADC_CHAN = 5, // the channel to sample
    STATS_AFTER = 10000 // print throughput statistics after every X samples
};

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    adcSetChannel(ADC_CHAN);
    uint32_t i;
    for (;;) {
        uint32_t time1 = getRealTime();
        for (i = 0; i < STATS_AFTER; ++i) {
            uint16_t val = adcReadFast();
            PRINTF("%i\n", val);
        }
        uint32_t time2 = getRealTime();
        uint32_t samplesHz = STATS_AFTER * 1000ul / (uint32_t) (time2 - time1);
        PRINTF("Sampling rate: %i Hz\n", samplesHz);
    }
}
