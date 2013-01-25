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

#include "threads.h"
#include <kernel/alarms_internal.h>
#include <kernel/threads/radio.h>
#include <net/radio_packet_buffer.h>
#include <net/mac.h>

ProcessFlags_t processFlags;

//
// Kernel main loop
//
void systemMain(void)
{
    for (;;) {
        // Process all outstanding system tasks.
        // Put radio processing first to get smaller packet processing delays!
#if USE_RADIO
        if (processFlags.bits.radioProcess) {
            processFlags.bits.radioProcess = false;
            radioProcess();
        }
#endif
#if USE_NET
        if (!isRadioPacketEmpty()) {
            macProtocol.poll();
        }
#endif
        if (processFlags.bits.alarmsProcess) {
            processFlags.bits.alarmsProcess = false;
            alarmsProcess();
        }

        if (!hasAnyAlarms()) {
            jiffiesToSleep = MAX_KERNEL_SLEEP_TIME;
        }
        else {
            uint32_t t = getNextAlarmTime() - (uint32_t)getJiffies();
            if ((int32_t) t < 0) {
                jiffiesToSleep = 0;
            } else {
                jiffiesToSleep = (uint16_t) t;
                if (jiffiesToSleep > MAX_KERNEL_SLEEP_TIME) {
                    jiffiesToSleep = MAX_KERNEL_SLEEP_TIME;
                }
            }
        }
        
        // find the next thread to run
        schedule();
    }
}
