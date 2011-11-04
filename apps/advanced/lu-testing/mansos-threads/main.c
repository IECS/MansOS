#include "stdmansos.h"
#include <string.h>
#include <kernel/expthreads/radio.h>
#include <kernel/expthreads/timing.h>

#define SENSOR_READ_INTERVAL 3000 // milliseconds

// MansOS global variable, used to record the current time on BaseStation
extern uint32_t rootClock;

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // declare our radio packet buffer
    RADIO_PACKET_BUFFER(radioBuffer, RADIO_MAX_PACKET);
    // turn on radio listening
    radioOn();

    // read sensors right now, when the loop is entered first time
    uint32_t sensorReadTime = getJiffies();
    for (;;) {
        // --- radio code
        if (isRadioPacketReceived(radioBuffer)) {
            // a packet is received from the radio
            if (radioBuffer.receivedLength >= sizeof(rootClock)) {
                memcpy(&rootClock, radioBuffer.buffer, sizeof(rootClock));
                PRINTF("received new root clock value=%lu\n", rootClock);
            }
        }
        // reset our radio buffer in any case
        radioBufferReset(radioBuffer);

        // --- sensor code
        uint32_t timeNow = getJiffies();
        // if 'timeNow' is after 'sensorReadTime'...
        if (timeAfter32(timeNow, sensorReadTime)) {
            // read light sensor value
            uint16_t light = readLight();
            PRINTF("light = %u\n", light);

            // send the value read to radio
            radioSend(&light, sizeof(light));

            // blink LED
            toggleRedLed();

            // set next read time
            sensorReadTime += SENSOR_READ_INTERVAL;
        }

        // do nothing for a while
        mdelay(1000);
    }
}
