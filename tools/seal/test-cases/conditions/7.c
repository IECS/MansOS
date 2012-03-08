// -------------------------------------------
// blink blue led; faster at the start

// when System.time < 2s:
//      use BlueLed, period 100ms;
// elsewhen System.time < 6s:
//      use BlueLed, period 500ms;
// else:
//      use BlueLed, period 2000ms;
// end

// -------------------------------------------

#include "stdmansos.h"

Alarm_t blueLedAlarm;

Alarm_t condition1Alarm;
Alarm_t condition2Alarm;
bool condition1Variable;
bool condition2Variable;

#define BLUE_LED_CONDITION1_PERIOD   100
#define BLUE_LED_CONDITION2_PERIOD   500
#define BLUE_LED_OTHERWISE_PERIOD   1000

#define CONDITION1_TIME  2000
#define CONDITION2_TIME  6000

void blueLedCallback(void *param)
{
    toggleBlueLed();
    if (condition1Variable) {
        alarmSchedule(&blueLedAlarm, BLUE_LED_CONDITION1_PERIOD);
    } else if (condition2Variable) {
        alarmSchedule(&blueLedAlarm, BLUE_LED_CONDITION2_PERIOD);
    } else {
        alarmSchedule(&blueLedAlarm, BLUE_LED_OTHERWISE_PERIOD);
    }
}

void condition1Callback(void *param)
{
    condition1Variable = false;
}

void condition2Callback(void *param)
{
    condition2Variable = false;
}

void appMain(void)
{
    condition1Variable = true;
    condition2Variable = true;

    alarmInit(&blueLedAlarm, blueLedCallback, NULL);
    if (condition1Variable) {
        alarmSchedule(&blueLedAlarm, BLUE_LED_CONDITION1_PERIOD);
    } else if (condition2Variable) {
        alarmSchedule(&blueLedAlarm, BLUE_LED_CONDITION2_PERIOD);
    } else {
        alarmSchedule(&blueLedAlarm, BLUE_LED_OTHERWISE_PERIOD);
    }

    alarmInit(&condition1Alarm, condition1Callback, NULL);
    alarmSchedule(&condition1Alarm, CONDITION1_TIME);

    alarmInit(&condition2Alarm, condition2Callback, NULL);
    alarmSchedule(&condition2Alarm, CONDITION2_TIME);
}
