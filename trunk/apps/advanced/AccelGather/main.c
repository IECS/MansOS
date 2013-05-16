#include <stdmansos.h>
#include "user_button.h"

#define MAIN_LOOP_LENGTH 20 // ms

#define START_DELAY    100

//
// Sample 3D Accelerometer on Zolertia Z1 platform, print output to serial
//
typedef struct Packet {
    uint32_t timestamp;
    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;
} Packet_t;

bool atStart = true;
volatile bool isActiveMode;
uint32_t extFlashAddr;

static inline void printPacket(Packet_t *p) {
    // PRINTF("%lu %d %d %d\n", p->timestamp,
    //         p->acc_x, p->acc_y, p->acc_z);

    extFlashWrite(extFlashAddr, (uint8_t *) p, sizeof(*p));
    extFlashAddr += sizeof(*p);
}

void cleanFlash(void) {
    ledsSet(0xffff);
    extFlashBulkErase();
    extFlashAddr = 0;
    // start with zeros
    extFlashWrite(extFlashAddr, (uint8_t *) &extFlashAddr, 4);
    extFlashAddr = 4;
    ledsSet(0x0);
}

void onUserButton(void) {
    static bool x;
    x = !x;
    if (x) return;

    PRINTF("onUserButton!\n");
    isActiveMode = !isActiveMode;

    if (isActiveMode) {
        if (atStart) {
            atStart = false;
        } else {
            // start from clean flash again...
            cleanFlash();
        }
    }
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINTF("\n\nAccelerometer data gathering app\n");

    extFlashWake();

    // start with clean and new...
    cleanFlash();

    accelOn();

    userButtonEnable(onUserButton);

    mdelay(START_DELAY);

    uint16_t lastSecond;
    uint16_t samplesPerSecond;

    for (;;) {
        while (!isActiveMode);

        redLedOn();

        lastSecond = getTimeSec();
        samplesPerSecond = 0;

        uint16_t i;
        for (i = 0; isActiveMode; i++) {
            if (i % 10 == 0) redLedToggle();

            uint32_t now = getTimeMs();
            uint32_t endTime = now + MAIN_LOOP_LENGTH;

            Packet_t p;
            p.timestamp = now;

            p.acc_x = accelReadX();
            p.acc_y = accelReadY();
            p.acc_z = accelReadZ();

            printPacket(&p);

            samplesPerSecond++;

            uint16_t currentSecond = getTimeSec();
            if (currentSecond != lastSecond) {
                PRINTF("samples per second = %u\n", samplesPerSecond);
                lastSecond = currentSecond;
                samplesPerSecond = 0;
            } 

            while (timeAfter32(endTime, getTimeMs()));
        }

        redLedOff();
    }
}
