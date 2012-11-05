#include "stdmansos.h"
#include <kernel/threads/mutex.h>

Mutex_t testMutex;
volatile uint8_t controlFlag;

static void secondThreadFunction(void);

void appMain(void)
{
    // create second thread
    threadCreate(1, secondThreadFunction);

    for (;;) {
        redLedToggle();
        while (controlFlag);
        mutexLock(&testMutex);
        PRINTF("in thread #0\n");
        mdelay(1000);
        controlFlag = 1;
        mutexUnlock(&testMutex);
    }
}

void secondThreadFunction(void)
{
    for (;;) {
        blueLedToggle();
        while (!controlFlag);
        mutexLock(&testMutex);
        PRINTF("in thread #1\n");
        controlFlag = 0;
        mutexUnlock(&testMutex);
    }
}
