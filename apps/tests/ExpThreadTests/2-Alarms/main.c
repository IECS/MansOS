#include "stdmansos.h"
#include <kernel/expthreads/alarms.h>

Alarm_t alarm1;
Alarm_t alarm2;

#define ALARM_1_INTERVAL 2000
#define ALARM_2_INTERVAL 3000

void alarmCallback(void *param)
{
    uint16_t id = (uint16_t) param;
    // blink green or blue light
    toggleNrLed(id);
    // print something
    PRINTF("hello from alarm %d, jiffies=%lu\n", id, getJiffies());
    // reschedule the alarm
    alarmSchedule(id == 1 ? &alarm1 : &alarm2, id == 1 ? ALARM_1_INTERVAL : ALARM_2_INTERVAL);
}

void appMain(void)
{
    // initialize the alarm
    alarmInit(&alarm1, alarmCallback, (void *) 1);
    // schedule the alarm after specific interval
    alarmSchedule(&alarm1, ALARM_1_INTERVAL);

    // initialize the alarm
    alarmInit(&alarm2, alarmCallback, (void *) 2);
    // schedule the alarm after specific interval
    alarmSchedule(&alarm2, ALARM_2_INTERVAL);

    for (;;) {
        PRINT("in app main...\n");
        toggleLed();
        msleep(1000);
    }
}
