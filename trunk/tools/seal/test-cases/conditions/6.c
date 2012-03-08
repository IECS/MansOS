// blink blue led
// when System.time < 5s: use BlueLed, period 100ms;

// -------------------------------------------

#include "stdmansos.h"

Alarm_t blueLedAlarm;

Alarm_t condition1Alarm;
bool condition1Variable;

#define BLUE_LED_CONDITION1_TIME  100

#define CONDITION1_TIME  5000

void blueLedCallback(void *param)
{
    toggleBlueLed();
    if (condition1Variable) {
        alarmSchedule(&blueLedAlarm, BLUE_LED_CONDITION1_PERIOD);
    } else {
        blueLedOff();
    }
}

void condition1Callback(void *param)
{
    condition1Variable = false;
}

void appMain(void)
{
    // condition1Variable = getJiffies() < CONDITION1_TIME;
    condition1Variable = true;

    alarmInit(&blueLedAlarm, blueLedCallback, NULL);
    if (condition1Variable) {
        alarmSchedule(&blueLedAlarm, BLUE_LED_CONDITION1_PERIOD);
    }

    alarmInit(&condition1Alarm, condition1Callback, NULL);
    alarmSchedule(&condition1Alarm, CONDITION1_TIME);
}
