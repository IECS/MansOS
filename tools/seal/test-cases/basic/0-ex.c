// config:
// USE_LEDS=y

#include "stdmansos.h"

#define REDLED_PERIOD 1000

Alarm_t redLedAlarm;

void redLedAlarmCallback(void *param)
{
    toggleRedLed();
    alarmSchedule(&redLedAlarm, REDLED_PERIOD);
}

void appMain(void)
{
    alarmInit(&redLedAlarm, redLedAlarmCallback, NULL);
    alarmSchedule(&redLedAlarm, REDLED_PERIOD);
}
