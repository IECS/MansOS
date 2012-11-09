/**
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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

#include "energy.h"
#include <lib/dprint.h>

volatile EnergyStats_t energyStats[TOTAL_ENERGY_CONSUMERS];

const char *energyConsumerNames[TOTAL_ENERGY_CONSUMERS] = {
    "MCU",
    "LPM",
    "LED_RED",
    "LED_GREEN",
    "LED_BLUE",
    "RADIO_TX",
    "RADIO_RX",
    "FLASH_WRITE",
    "FLASH_READ",
    "SENSORS",
    "SERIAL"
};

void energyStatsDump(void)
{
    int i;
    for (i = 0; i < TOTAL_ENERGY_CONSUMERS; ++i) {
        if (energyStats[i].on) {
            uint32_t now = (uint32_t) getJiffies();
            energyStats[i].totalTicks += now - energyStats[i].lastTicks;
            energyStats[i].lastTicks = now;
        }
        PRINTF("%s: %lu%s\n", energyConsumerNames[i], energyStats[i].totalTicks,
                energyStats[i].on ? " (on)" : "");
    }
}


void energyConsumerOn(EnergyConsumer_t type) {
    if (!energyStats[type].on) {
        energyStats[type].on = true;
        energyStats[type].lastTicks = (uint32_t) getJiffies();
    }
}

void energyConsumerOff(EnergyConsumer_t type) {
    if (energyStats[type].on) {
        energyStats[type].totalTicks +=
                (uint32_t) getJiffies() - energyStats[type].lastTicks;
        energyStats[type].on = false;
    }
}
