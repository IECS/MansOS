#include "stdmansos.h"
#include <kernel/expthreads/alarms.h>
#include <kernel/expthreads/mutex.h>

Mutex_t testMutex;
volatile uint8_t controlFlag;

static void secondThreadFunction(void);

void appMain(void)
{
    // create second thread
    threadCreate(1, secondThreadFunction);

    for (;;) {
        toggleRedLed();
        while (controlFlag);
        mutexLock(&testMutex);
        PRINT("in thread #0\n");
        mdelay(1000);
        controlFlag = 1;
        mutexUnlock(&testMutex);
    }
}

void secondThreadFunction(void)
{
    for (;;) {
        toggleBlueLed();
        while (!controlFlag);
        mutexLock(&testMutex);
        PRINT("in thread #1\n");
        controlFlag = 0;
        mutexUnlock(&testMutex);
    }
}
