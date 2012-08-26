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

#include <kernel/reprogramming.h>
#include <smp/smp.h>
#include <extflash.h>
#include <intflash.h>
#include <watchdog.h>
#include <radio.h>
#include <kernel/boot.h>
#include <lib/dprint.h>
#include <lib/codec/crc.h>

static uint16_t currentImageId;
static uint32_t currentExtFlashStartAddress;
static uint16_t currentImageBlockCount;

// TODO: also write checksums in external flash

// while working with flash, radio must be turned off
#define SELECT_FLASH        \
    {                       \
        radioOff();         \
        extFlashWake();

// turn radio back on
#define UNSELECT_FLASH      \
        extFlashSleep();    \
        radioOn();          \
    }

void processRSPacket(ReprogrammingStartPacket_t *p)
{
    PRINTF("process RS packet, address=0x%lx\n", p->extFlashAddress);

    currentImageId = p->imageId;
    currentExtFlashStartAddress = p->extFlashAddress;
    currentImageBlockCount = p->imageBlockCount;

    SELECT_FLASH;
    // prepare flash to be written (one sector, i.e. 64k max!)
    extFlashEraseSector(p->extFlashAddress);
    // write image size
    extFlashWrite(p->extFlashAddress, &p->imageBlockCount, sizeof(uint16_t));
    UNSELECT_FLASH;
}

bool processRCPacket(ReprogrammingContinuePacket_t *p)
{
    // PRINTF("process RC packet\n");

    if (p->imageId != currentImageId) {
        PRINTF("reprogramming: wrong image id (0x%x, expecting 0x%x)\n",
                p->imageId, currentImageId);
        return false;
    }
    const ExternalFlashBlock_t *fb = (ExternalFlashBlock_t *) &p->address;
    if (fb->crc != crc16((uint8_t *)fb, sizeof(*fb) - sizeof(uint16_t))) {
        PRINTF("reprogramming: wrong checksum\n");
        return false;
    }

    uint32_t address = currentExtFlashStartAddress + 2
            + p->blockId * sizeof(ExternalFlashBlock_t);
    // PRINTF("address=0x%x, id=%d\n", address, p->blockId);

    // write data and CRC
    SELECT_FLASH;
    extFlashWrite(address, fb, sizeof(*fb));
    UNSELECT_FLASH;
    return true;
}

void processRebootCommand(RebootCommandPacket_t *p)
{
    PRINTF("process reboot command\n");

    if (p->doReprogram) {
        PRINTF("ext addr=0x%lx int addr=0x%x\n", p->extFlashAddress, p->intFlashAddress);

        BootParams_t bootParams;
        intFlashRead(BOOT_PARAMS_ADDRESS, &bootParams, sizeof(bootParams));
        bootParams.extFlashAddress = p->extFlashAddress;
        bootParams.intFlashAddress = p->intFlashAddress;
        bootParams.doReprogramming = 1;
        bootParams.bootRetryCount = 0;
        intFlashErase(BOOT_PARAMS_ADDRESS, sizeof(bootParams));
        intFlashWrite(BOOT_PARAMS_ADDRESS, &bootParams, sizeof(bootParams));
        (void) bootParams; // make compiler happy
    }

    // in any case, reboot the mote
    watchdogReboot();
}

// global pseudovariables defined by linker (gcc specific code!)
extern const uint16_t __stack;
extern const uint16_t __data_size;
extern const void *__data_start, *__data_load_start;
extern const uint16_t __bss_size;
extern const void *__bss_start;

//
// Start routine, used in case bootloader is not present.
//
void start(void) __attribute__ ((section (".start"))) NAKED;
void start(void)
{
    // 
    // Call initialization routines.
    // By default these are put by the compiler, right before start of the text segment.
    // However, if you redefine _reset_vector__ in the program
    // (as is the case when reprogamming is used),
    // then they are not included, starting from msp430 GCC 4.5+
    //
    // The sequence is the same as in GCC sources,
    // file gcc/config/msp430/crt0.S
    //

    // initialize stack register using __stack value provided by linker
    SET_SP_IMMED(__stack);

    // stop the watchdog
    watchdogStop();

    // copy data segment to RAM
    register uint16_t i;
    i = __data_size;
    while (i) {
        i--;
        *(uint8_t *)((uint16_t)&__data_start + i) =
                *(uint8_t *)((uint16_t)&__data_load_start + i);
    }
    // msp430 assembler code:
    // asm("mov     #__data_size, r15\n"
    //     "tst     r15\n"
    //     "jz      .L__copy_data_end\n"
    //     ".L__copy_data_loop:\n"
    //     "decd    r15\n"
    //     "mov.w   __data_load_start(r15), __data_start(r15)\n"
    //     "jne     .L__copy_data_loop\n"
    //     ".L__copy_data_end:");

    // zero out BSS segment
    i = __bss_size;
    while (i) {
        i--;
        *(uint8_t *)((uint16_t)&__bss_start + i) = 0;
    }
    // msp430 assembler code:
    // asm("mov     #__bss_size, r15\n"
    //     "tst     r15\n"
    //     "jz      .L__clear_bss_end\n"
    //     ".L__clear_bss_loop:\n"
    //      "dec     r15\n"
    //     "clr.b   __bss_start(r15)\n"
    //      "jne     .L__clear_bss_loop\n"
    //      ".L__clear_bss_end:\n");

    // initalization done;
    // simply jump to start of .text section
    ((ApplicationStartExec)SYSTEM_CODE_START)();
}
