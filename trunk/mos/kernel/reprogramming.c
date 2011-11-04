#include <kernel/reprogramming.h>
#include <smp/smp.h>
#include <hil/extflash.h>
#include <hil/intflash.h>
#include <hil/watchdog.h>
#include <hil/radio.h>
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
    }

    // in any case, reboot the mote
    watchdogReboot();
}

//
// Start routine, used in case bootloader is not present.
//
void start(void) __attribute__ ((section (".start")));
void start(void)
{
    // simply jump to start of .text section
    ((ApplicationStartExec)SYSTEM_CODE_START)();
}
