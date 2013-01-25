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

//----------------------------------------------------------
// MansOS Kernel main source file
//----------------------------------------------------------
#include <stdmansos.h>
#include <eeprom.h>
#include <watchdog.h>
#include <random.h>
#include <smp/smp.h>
#include <wmp/wmp.h>
#include <assert.h>
#if USE_EXT_FLASH
#include <extflash.h>
#endif
#if USE_FLASH
#include "boot.h"
#endif
#include "alarms_internal.h"
#include <net/networking.h>
#include <net/seal_networking.h>
#include <beeper.h>
#if USE_THREADS
#include "threads/threads.h"
#elif USE_PROTOTHREADS
#include <kernel/protothreads/protosched.h>
#endif
#include <arch_mem.h>
#include <sdcard/sdcard.h>
#include <net/timesync.h>
#include <lib/energy.h>
#if USE_ADS8638
#include <ads8638/ads8638.h>
#endif
#if USE_DAC7718
#include <dac7718/dac7718.h>
#endif
#if USE_AD5258
#include <ad5258/ad5258.h>
#endif
#include <fs/init.h>
#include <fatfs/fatfs.h>

#if (defined DEBUG && !defined DPRINT_TO_RADIO)
#define INIT_PRINTF(...) PRINTF(__VA_ARGS__)
#else
#define INIT_PRINTF(...) // nothing
#endif

// global time counter
volatile ticks_t jiffies;

//----------------------------------------------------------
//      System initialization
//----------------------------------------------------------
static inline void initSystem(void)
{
    bool success;
    (void)success;

    // disable interrupts: disabled on msp430 by default, but other systems might need this
    DISABLE_INTS();

    // stop the watchdog: GCC disables it by default, but other compilers might not be so helpful
    watchdogStop();

    // TODO: init dynamic memory
    // platformMemInit();

    // basic, platform-specific initialization: timers, platform-specific drivers (?)
    initPlatform();

    // start energy accounting (as soon as timers are initialized)
    energyConsumerOn(ENERGY_CONSUMER_MCU);

#ifdef USE_PRINT
    // init printing to serial (makes sense only after clock has been calibrated)
    if (printInit != NULL) printInit();
#endif

    INIT_PRINTF("starting MansOS...\n");

#ifdef USE_LEDS
    INIT_PRINTF("init LED(s)...\n");
    ledsInit();
#endif
#ifdef USE_BEEPER
    beeperInit();
#endif
#ifdef RAMTEXT_START
    if ((MemoryAddress_t)&_end > RAMTEXT_START) {
        // Panic right aways on RAM overflow.
        // In case this happens, you might want to increase the address
        // specified by CONST_RAMTEXT_START in config file
        assertionFailed("Overflow between .data and .ramtext sections", __FILE__, __LINE__);
    }
#endif
#ifdef USE_ADC
    if (adcInit != NULL) {
        INIT_PRINTF("init ADC...\n");
        adcInit();
    }
#endif
#ifdef USE_RANDOM
    INIT_PRINTF("init RNG...\n");
    randomInit();
#endif
#if USE_ALARMS
    INIT_PRINTF("init alarms...\n");
    initAlarms();
#endif
#ifdef USE_RADIO
    INIT_PRINTF("init radio...\n");
    radioInit();
#endif
#ifdef USE_ADDRESSING
    INIT_PRINTF("init communication stack...\n");
    networkingInit();
#endif
#ifdef USE_EXT_FLASH
    INIT_PRINTF("init external flash...\n");
    extFlashInit();
#endif
#ifdef USE_SDCARD
    INIT_PRINTF("init SD card...\n");
    sdcardInit();
#endif
#ifdef USE_EEPROM
    INIT_PRINTF("init EEPROM...\n");
    eepromInit();
#endif
#ifdef USE_ISL29003
    INIT_PRINTF("init ISL light sensor...\n");
    success = islInit();
    if (!success) {
        INIT_PRINTF("ISL init failed!\n");
    }
#endif
#ifdef USE_ADS1115
    INIT_PRINTF("init ADS111x ADC converter chip...\n");
    adsInit();
#endif
#if USE_ADS8638
    INIT_PRINTF("init ADS8638 ADC converter chip...\n");
    ads8638Init();
#endif
#if USE_AD5258
    INIT_PRINTF("init AD5258 digital potentiometer...\n");
    ad5258Init();
#endif
#ifdef USE_HUMIDITY
    INIT_PRINTF("init humidity sensor...\n");
    humidityInit();
#endif
#ifdef USE_ACCEL
    INIT_PRINTF("init accelerometer...\n");
    accelInit();
#endif
#ifdef USE_TIMESYNC
    INIT_PRINTF("init base station time sync...\n");
    timesyncInit();
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
#ifdef USE_FATFS
    INIT_PRINTF("init FAT file system...\n");
    fatFsInit();
    INIT_PRINTF("init POSIX-like file routines...\n");
    posixStdioInit();
#endif
#ifdef USE_WMP
    INIT_PRINTF("init WMP...\n");
    wmpInit();
#endif
#ifdef USE_SEAL_COMM
    INIT_PRINTF("init SEAL communications...\n");
    sealCommInit();
#endif

    INIT_PRINTF("starting the application...\n");
}

#if USE_KERNEL_MAIN
//----------------------------------------------------------
//      Main entry point
//----------------------------------------------------------
int main(void)
{
    initSystem();

// ------------------------------------------
#ifdef USE_THREADS
    MESSAGE("Using threads");

    // never returns
    startThreads(appMain, systemMain);

    // if we're here, something went wrong
    ASSERT(!"systemMain returned?!");

// ------------------------------------------
#else

    ENABLE_INTS();

#ifdef USE_PROTOTHREADS
    startProtoSched();
#else
    MESSAGE("Not using threads");
    appMain();
#endif

    //
    // Do not allow to return from this function. The reason:
    // GCC 4.5+ disables interrupts after completion of main() function,
    // but MansOS applications may want to return from appMain()
    // and do all the "real work" in interrupt handlers.
    // Therefore, interrupts must be kept enabled.
    //
    for (;;) {}
#endif // USE_THREADS

    return 0;
}
#endif // USE_KERNEL_MAIN
