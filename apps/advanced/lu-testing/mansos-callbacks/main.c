#include "stdmansos.h"
#include <hil/alarm.h>
#include <string.h>

#define SENSOR_READ_INTERVAL 3000 // milliseconds

// MansOS global variable, used to record the current time on BaseStation
extern uint32_t rootClock;

// our timer
Alarm_t timer;

//-------------------------------------------
//      Timer callback
//-------------------------------------------
void onTimer(void *param)
{
    // read light sensor value
    uint16_t light = readLight();
    PRINTF("light = %u\n", light);

    // send the value read to radio
    radioSend(&light, sizeof(light));

    // blink LED
    toggleRedLed();
}

//-------------------------------------------
//      Radio packet receive callback
//-------------------------------------------
void onRadioInterrupt(void)
{
    static uint8_t buffer[RADIO_MAX_PACKET];
    int16_t len;

    toggleGreenLed();
    len = radioRecv(buffer, sizeof(buffer));
    if (len >= sizeof(rootClock)) {
        memcpy(&rootClock, buffer, sizeof(rootClock));
        PRINTF("received new root clock value=%lu\n", rootClock);
    }
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // initialize and schedule the alarm
    alarmInitAndRegister(&timer, onTimer, SENSOR_READ_INTERVAL, true, NULL);
    // set radio packet receive callback
    radioSetReceiveHandle(onRadioInterrupt);
    // turn radio listening on
    radioOn();
    // our job here is complete
    return;
}
