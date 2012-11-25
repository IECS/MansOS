#include <stdmansos.h>
#include "user_button.h"

#define MAIN_LOOP_LENGTH 320 // ticks; a little less than 10 ms

#define START_DELAY    1000

//
// Sample 3D Accelerometer on Zolertia Z1 platform, print output to serial
//
typedef struct Packet_s {
    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;
} Packet_t;

typedef enum { MODE_STANDING, MODE_DRIVING } VehicleMode_e;

VehicleMode_e currentMode = MODE_STANDING;
uint8_t modeStateChange;

void buttonStateChanged(void) {
    modeStateChange++;
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    userButtonEnable(buttonStateChanged);

    PRINTF("Starting...\n");

    accelOn();

    mdelay(START_DELAY);

    redLedOn();

    for (;;) {
        uint16_t now = ALARM_TIMER_VALUE();
        uint16_t endTime = now + MAIN_LOOP_LENGTH;

        Packet_t packet;
        packet.acc_x = accelReadX();
        packet.acc_y = accelReadY();
        packet.acc_z = accelReadZ();

        if (modeStateChange >= 2) {
            Packet_t packet1;
            modeStateChange = 0;
            if (currentMode == MODE_DRIVING) {
                currentMode = MODE_STANDING;
                redLedOn();
                packet1.acc_x = packet1.acc_y = packet1.acc_z = 0xff;
            } else {
                currentMode = MODE_DRIVING;
                redLedOff();
                packet1.acc_x = packet1.acc_y = packet1.acc_z = 0xfe;
            }
            serialSendData(PRINTF_SERIAL_ID, (uint8_t *)&packet1, sizeof(packet1));
            serialSendByte(PRINTF_SERIAL_ID, '\n');
        }

        serialSendData(PRINTF_SERIAL_ID, (uint8_t *)&packet, sizeof(packet));
        serialSendByte(PRINTF_SERIAL_ID, '\n');

        while (timeAfter16(endTime, ALARM_TIMER_VALUE()));
    }
}
