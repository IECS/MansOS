#include <stdmansos.h>

#define MAIN_LOOP_LENGTH 32 // ticks; approximately 1 ms

#define START_DELAY    1000

//
// Sample 3D Accelerometer on Zolertia Z1 platform, print output to serial
//
typedef struct Packet_s {
    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;
} Packet_t;


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINTF("Starting...\n");

    accelOn();

    mdelay(START_DELAY);

    redLedOn();

    for (;;) {
        uint16_t now = ALARM_TIMER_VALUE();
        uint16_t endTime = now + MAIN_LOOP_LENGTH;

//        PRINTF("%d %d %d\n", accelReadX(), accelReadY(), accelReadZ());
        Packet_t packet;
        packet.acc_x = accelReadX();
        serialSendData(PRINTF_SERIAL_ID, (uint8_t *)&packet, sizeof(packet));
        serialSendByte(PRINTF_SERIAL_ID, '\n');

        while (timeAfter16(endTime, ALARM_TIMER_VALUE()));
    }
}
