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

//----------------------------------------------------------
// MansOS Kernel main source file
//----------------------------------------------------------
#include "stdmansos.h"
#include <hil/i2c.h>
#include <hil/eeprom.h>
#include <hil/fs/init.h>
#include <hil/watchdog.h>
#include <smp/smp.h>
#include <lib/random.h>
#include <lib/assert.h>
#if USE_EXT_FLASH
#include <hil/extflash.h>
#endif
#if USE_FLASH
#include "boot.h"
#endif
#include "coop_scheduler.h"
#include "alarms_system.h"
#include <net/comm.h>
#if USE_EXP_THREADS
#include "threads/threads.h"
#endif

#if (defined DEBUG && !defined DPRINT_TO_RADIO)
#define INIT_PRINTF(...) PRINTF(__VA_ARGS__)
#else
#define INIT_PRINTF(...) // nothing
#endif

//----------------------------------------------------------
//      System initialization
//----------------------------------------------------------
static void initSystem(void)
{
    // disable interrupts (disabled on msp430 by default, but other systems might need this)
    DISABLE_INTS();

    // stop the watchdog (GCC disables it by default, but other compilers might need this line)
    watchdogStop();

    // basic, platform-specific initialization: timers, platform-specific drivers (?)
    initPlatform();

    // init printing to serial (makes sense only after clock has been calibrated)
    if (printInit != NULL) printInit();

    INIT_PRINTF("starting MansOS...\n");

#ifdef USE_LEDS
    INIT_PRINTF("init LED(s)...\n");
    ledsInit();
#endif
#ifdef USE_ADC
    if (initAdc != NULL) {
        INIT_PRINTF("init ADC...\n");
        initAdc();
    }
#endif
#ifdef USE_I2C
    INIT_PRINTF("init I2C...\n");
    i2cInit();
#endif
#ifdef USE_RANDOM
    INIT_PRINTF("init RNG...\n");
    randomInit();
#endif
#ifdef USE_RADIO
    INIT_PRINTF("init radio...\n");
    radioInit();
#endif
#if USE_ALARMS
    INIT_PRINTF("init alarms...\n");
    initAlarms();
#endif
#ifdef USE_ADDRESSING
    INIT_PRINTF("init communication stack...\n");
    initComm();
#endif
#ifdef USE_EXT_FLASH
    INIT_PRINTF("init external flash...\n");
    extFlashInit();
#endif
#ifdef USE_EEPROM
    INIT_PRINTF("init EEPROM...\n");
    eepromInit();
#endif
#ifdef USE_HUMIDITY
    INIT_PRINTF("init humidity sensor...\n");
    humidityInit();
#endif
#ifdef USE_SMP
    INIT_PRINTF("init SSMP...\n");
    smpInit();
#endif
#ifdef USE_REPROGRAMMING
    INIT_PRINTF("init reprogramming...\n");
    bootParamsInit();
#endif
#ifdef USE_FS
    INIT_PRINTF("init file system...\n");
    fsInit();
#endif

    INIT_PRINTF("starting the application...\n");
}

//----------------------------------------------------------
//      Main entry point
//----------------------------------------------------------
int main(void)
{
    initSystem();

// ------------------------------------------
#ifdef USE_EXP_THREADS
    PRAGMA(message "Using threads");

    // never returns
    startThreads(appMain, systemMain);

    // if we're here, something went wrong
    ASSERT(!"systemMain returned?!");

// ------------------------------------------
#else
    PRAGMA(message "Not using threads");

    ENABLE_INTS();

#ifdef USE_FIBERS
    coopSchedStart();
#else
    appMain();

    for (;;); // needed for GCC 4.5+
#endif // !USE_FIBERS

#endif // !USE_EXP_THREADS

    return 0;
}
