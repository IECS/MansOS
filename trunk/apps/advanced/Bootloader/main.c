/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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

#include <hil/intflash.h>
#include <hil/extflash.h>
#include <hil/usart.h>
#include <hil/watchdog.h>
#include <lib/codec/crc.h>
#include <kernel/reprogramming.h>
#include <msp430/msp430_int.h>
#include <msp430/msp430_timers.h>
#include "stdmansos.h"

// delay for some time to allow the user to reboot mote (in case he wants to load the "golden image")
#define ENABLE_BOOT_DELAY 0

#define CODE_SIZE         (MSP430_FLASH_SIZE - MAX_BOOTLOADER_SIZE) // 48kb

// TODO: move this MSP430 specific section to somewhere...

// ---------------------------------------------------------------------------
uint16_t readVoltage(void)
{
    uint_t i;
    // initalize ADC 
    // TODO XXX: this code from TinyOS, rewrite it
    ADC12CTL0 = (0x0010 | (2 << 8)) | 0x0020;
    ADC12CTL1 = 0x0200;
    ADC12MCTL0 = (0x80 | (1 << 4)) | 11;
    for (i = 0; i < 0x3600; i++);

    ADC12CTL0 |= 0x0002;
    ADC12CTL0 |= 0x0001;
    while ((ADC12IFG & 0x0001) == 0);

    ADC12CTL0 &= ~0x0002;
    ADC12CTL0 = 0;

    // read the ADC
    return ADC12MEM0;
}

// FIXME: this is wrong, but not all motes can read voltage correctly!
#define THRESHOLD_VOLTAGE  0xd00
//#define THRESHOLD_VOLTAGE  0xE66  // 2.7 volts (@ 1.5 V reference voltage?)

// ---------------------------------------------------------------------------

#define RED      (1 << LEDS_RED_PIN)
#define GREEN    (1 << LEDS_GREEN_PIN)
#define BLUE     (1 << LEDS_BLUE_PIN)
#define ALL_LEDS (RED | GREEN | BLUE)

#define toggleLeds(mask) pinToggleMask(LEDS_RED_PORT, mask)
#define clearLeds(mask) pinSetMask(LEDS_RED_PORT, mask)

#define LEDS_BOOTLOADER_START GREEN
#define LEDS_BOOTLOADER_END   BLUE
#define LEDS_LOW_BATTERY      (RED | GREEN)
#define LEDS_CRC_ERROR        (RED | BLUE)

// almost same as TinyOS leds.flash()
static void flashLeds(uint8_t mask)
{
    uint_t i;
    clearLeds(ALL_LEDS);
    for (i = 0; i < 6; ++i) {
        toggleLeds(mask);
        mdelay(150);
    }
}

int main(void)
{
    // Make sure all interrupts are disabled.
    // This must be the first step, because in some cases interupt vectors are totally wrong
    // (e.g. when we get here after a soft reboot from another application)
    msp430ClearAllInterruptsNosave();

    msp430WatchdogStop();

    initLeds();

    flashLeds(LEDS_BOOTLOADER_START);

    BootParams_t bootParams;
    intFlashRead(BOOT_PARAMS_ADDRESS, &bootParams, sizeof(bootParams));

    ++bootParams.bootRetryCount;
    if (bootParams.bootRetryCount > MAX_RETRY_COUNT) {
        bootParams.bootRetryCount = 0;
        bootParams.extFlashAddress = GOLDEN_IMAGE_ADDRESS;
        bootParams.doReprogramming = 1;
    }

    // make sure internal flash address is sane
    if (bootParams.intFlashAddress == 0xffff || bootParams.intFlashAddress < BOOTLOADER_END) {
        bootParams.intFlashAddress = SYSTEM_CODE_START;
    }

    // read voltage, and quit if not enough for writing in flash
    if (readVoltage() < THRESHOLD_VOLTAGE) {
        flashLeds(LEDS_LOW_BATTERY);
        goto exec;
    }

    // write the updated info back in flash
    intFlashErase(BOOT_PARAMS_ADDRESS, sizeof(bootParams));
    intFlashWrite(BOOT_PARAMS_ADDRESS, &bootParams, sizeof(bootParams));

    if (bootParams.doReprogramming) {
        redLedOn();

        // will be using external flash
        extFlashInit();
        extFlashWake();

        uint32_t extAddress = bootParams.extFlashAddress;

        // read number of blocks
        uint16_t imageBlockCount;
        extFlashRead(extAddress, &imageBlockCount, sizeof(uint16_t));
        extAddress += 2;

        while (imageBlockCount) {
            // read a block from external flash
            ExternalFlashBlock_t block;
            extFlashRead(extAddress, &block, sizeof(block));

            if (block.crc != crc16((uint8_t *)&block, sizeof(block) - 2)) {
                // the best we can do is to reboot now;
                // after a few tries golden image will be loaded
                flashLeds(LEDS_CRC_ERROR);
                // no need to disable all of the interrupts (they already are),
                // just write in watchdog timer wihout password, it will generate reset.
                watchdogRebootSimple();
            }

            bool firstBlockInChunk = block.address & 0x1;
            block.address &= ~0x1;
            if (firstBlockInChunk) {
                // prepare internal flash to be written
                intFlashErase(block.address, INT_FLASH_SEGMENT_SIZE);
            }

            // program internal flash
            COMPILE_TIME_ASSERT(sizeof(block.data) == INT_FLASH_BLOCK_SIZE, ifs);
            intFlashWriteBlock(block.address, block.data, INT_FLASH_BLOCK_SIZE);

            --imageBlockCount;
            extAddress += sizeof(ExternalFlashBlock_t);
        }

        extFlashSleep();
        redLedOff();
    }

#if ENABLE_BOOT_DELAY
    // delay for a second or so to allow the user to interrupt booting (by pressing the reset button)
    flashLeds(LEDS_BOOTLOADER_END);
#endif

    // execute the program
  exec:
    ((ApplicationStartExec)bootParams.intFlashAddress)();
}
