// config:
// USE_LEDS=y

#include "stdmansos.h"

#define ARRAYLEN(a) (sizeof(a)/sizeof(*a))

const uint16_t patternSOS[] = {
    300,300,300,300,300,300,
    900,300,900,300,900,300,
    300,300,300,300,300,300,
    3000
};

Alarm_t redLedAlarm;
uint16_t patternSOSCounter;

void redLedAlarmCallback(void *param)
{
    toggleRedLed();
    alarmSchedule(&redLedAlarm, patternSOS[patternSOSCounter]);
    patternSOSCounter = (patternSOSCounter + 1) % ARRAYLEN(patternSOS);
}

void appMain(void)
{
    alarmInit(&redLedAlarm, redLedAlarmCallback, NULL);
    alarmSchedule(&redLedAlarm, 0);
}
