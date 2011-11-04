#include "stdmansos.h"
#include <kernel/expthreads/alarms.h>
#include <kernel/expthreads/mutex.h>

Alarm_t alarm;
#define ALARM_INTERVAL 600

Mutex_t testMutex;

void alarmCallback(void *param)
{
    (void) param;
    // lock the mutex in kernel context
    uint32_t start, end;
    start = getJiffies();
    mutexLock(&testMutex);
    end = getJiffies();
    PRINTF("in alarm callback, mutex lock time=%lu\n", end - start);
    mdelay(1300);
    mutexUnlock(&testMutex);

    // reschedule the alarm
    alarmSchedule(&alarm, ALARM_INTERVAL);
}

void appMain(void)
{
    // initialize the alarm
    alarmInit(&alarm, alarmCallback, NULL);
    // schedule the alarm after specific interval
    alarmSchedule(&alarm, ALARM_INTERVAL);

    for (;;) {
        toggleLed();

        // lock the mutex in user context
        uint32_t start, end;
        start = getJiffies();
        mutexLock(&testMutex);
        end = getJiffies();
        PRINTF("in user main, mutex lock time=%lu\n", end - start);
        mdelay(1000);
        mutexUnlock(&testMutex);
    }
}
