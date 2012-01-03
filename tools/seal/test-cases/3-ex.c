// blink red led periodically
// use RedLed, period 1000ms;
// blink green led; faster at the start
// use GreenLed, if System.time < 5s then period 100ms else period 1000ms;
// turn blue led on 2 seconds after the program has started, and off at 5 seconds
// use BlueLed, on_at 2000ms, off_at 5000ms;

// --------------------------------------------------

#include "stdmansos.h"

Alarm_t redLedAlarm;
Alarm_t greenLedAlarm;
Alarm_t blueLedAlarm;

#define RED_LED_PERIOD     1000
#define GREEN_LED_PERIOD1   100
#define GREEN_LED_PERIOD2  1000
#define BLUE_LED_ON_AT     2000

void redLedCallback(void *param)
{
    toggleRedLed();
    alarmSchedule(&redLedAlarm, RED_LED_PERIOD);
}

void greenLedCallback(void *param)
{
    toggleGreenLed();
    uint16_t greenLedPeriod;
    if (getRealTime() < 5000) {
        greenLedPeriod = GREEN_LED_PERIOD1;
    } else {
        greenLedPeriod = GREEN_LED_PERIOD2;
    }
    alarmSchedule(&greenLedAlarm, greenLedPeriod);
}

void blueLedOnCallback(void *param)
{
    blueLedOn();
}

void appMain(void)
{
    alarmInit(&redLedAlarm, redLedCallback, NULL);
    alarmSchedule(&redLedAlarm, RED_LED_PERIOD);

    alarmInit(&greenLedAlarm, greenLedCallback, NULL);
    uint16_t greenLedPeriod;
    if (getRealTime() < 5000) {
        greenLedPeriod = GREEN_LED_PERIOD1;
    } else {
        greenLedPeriod = GREEN_LED_PERIOD2;
    }
    alarmSchedule(&greenLedAlarm, greenLedPeriod);

    alarmInit(&blueLedAlarm, blueLedOnCallback, NULL);
    alarmSchedule(&blueLedAlarm, BLUE_LED_ON_AT);
}
