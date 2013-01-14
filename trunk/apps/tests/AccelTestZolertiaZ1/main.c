#include <stdmansos.h>

#define MAIN_LOOP_LENGTH 1000 // ms

#define START_DELAY    1000

//
// Sample 3D Accelerometer on Zolertia Z1 platform, print output to serial
//
typedef struct Packet {
    uint32_t timestamp;
    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;
} Packet_t;

static inline void printPacket(Packet_t *p) {
    PRINTF("%lu %d %d %d\n", p->timestamp,
            p->acc_x, p->acc_y, p->acc_z);
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINTF("\n\nAccelerometer API test app\n");

    accelOn();

    mdelay(START_DELAY);

    redLedOn();

    for (;;) {
        uint32_t now = getTimeMs();
        uint32_t endTime = now + MAIN_LOOP_LENGTH;

        Packet_t p;
        p.timestamp = now;

        p.acc_x = accelReadX();
        p.acc_y = accelReadY();
        p.acc_z = accelReadZ();;

        printPacket(&p);

        while (timeAfter32(endTime, getTimeMs()));
    }
}
