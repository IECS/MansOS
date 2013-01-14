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

#ifndef MANSOS_ENERGY_H
#define MANSOS_ENERGY_H

#include <timing.h>

//
// List all potential energy consumers in the system
//
typedef enum {
    ENERGY_CONSUMER_MCU,
    ENERGY_CONSUMER_LPM,
    ENERGY_CONSUMER_LED_RED,
    ENERGY_CONSUMER_LED_GREEN,
    ENERGY_CONSUMER_LED_BLUE,
    ENERGY_CONSUMER_RADIO_TX,
    ENERGY_CONSUMER_RADIO_RX,
    ENERGY_CONSUMER_FLASH_WRITE,
    ENERGY_CONSUMER_FLASH_READ,
    ENERGY_CONSUMER_SENSORS,
    ENERGY_CONSUMER_SERIAL,

    TOTAL_ENERGY_CONSUMERS
} EnergyConsumer_t;

// TODO: ADC, I2C, SPI, watchdog, non-floating GPIO pins?

// Note: the current energy stats gathering cannot handle nested interrupts!

typedef struct EnergyStats_s {
    uint32_t totalTicks;
    uint32_t lastTicks;
    bool on;
} EnergyStats_t;

extern volatile EnergyStats_t energyStats[TOTAL_ENERGY_CONSUMERS];

#if USE_ENERGY_STATS

void energyConsumerOn(EnergyConsumer_t type);
void energyConsumerOff(EnergyConsumer_t type);

static inline void energyConsumerSet(EnergyConsumer_t type, uint8_t on) {
    if (on) energyConsumerOn(type);
    else energyConsumerOff(type);
}

//
// When interrupts are disabled, use TAR directly
//
#define energyConsumerOnNoints(type)                      \
    {                                                     \
        const uint16_t _start_time = ALARM_TIMER_READ();  \
        energyStats[type].on = true

#define energyConsumerOffNoints(type) {                                 \
        const uint_t _time_diff = ALARM_TIMER_READ() - _start_time;     \
        energyStats[type].totalTicks += TIMER_TICKS_TO_MS(_time_diff);  \
        energyStats[type].on = false;                                   \
    }                                                                   \
    }

//
// When measuring IN interrupt, use TAR *and* account it if not already on
//
#define energyConsumerOnIRQ(type)                       \
    bool _energy_consumer_off = !energyStats[type].on;  \
    energyConsumerOnNoints(type);

#define energyConsumerOffIRQ(type)                             \
    if (_energy_consumer_off) energyConsumerOffNoints(type);   \


#else

#define energyConsumerOn(type)         // nothing
#define energyConsumerOff(type)         // nothing
#define energyConsumerSet(type, value) // nothing

#define energyConsumerOnNoints(type)   // nothing
#define energyConsumerOffNoints(type)  // nothing

#define energyConsumerOnIRQ(type)      // nothing
#define energyConsumerOffIRQ(type)     // nothing

#endif

void energyStatsDump(void);

#endif
