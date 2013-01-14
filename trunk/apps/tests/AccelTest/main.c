#include "stdmansos.h"
#include <sleep.h>
#include <beeper.h>
#include <string.h>


#define MAIN_LOOP_LENGTH 10 // ms

#define START_DELAY    1000

#define enableAdcPin(port, pin) \
    pinAsFunction(port, pin); \
    pinAsInput(port, pin);

#define enableSensorPower() \
    pinAsOutput(1, 7); \
    pinSet(1, 7);

//
// Samples 3D Accelerometer + 2D Gyroscope, print output to serial
// Sparkfun 5DOF board connected to ADCx pins (see constants)
//
enum {
    ACCEL_X = 0,
    ACCEL_Y = 1,
    ACCEL_Z = 2,
    GYRO_X  = 3,
    GYRO_Y  = 4,
    TOTAL_ADC
};

typedef struct Packet {
    uint32_t timestamp;
    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;
    uint16_t gyro_x;
    uint16_t gyro_y;
} Packet_t;

static inline void printPacket(Packet_t *p) {
    PRINTF("%lu %u %u %u %u %u\n", p->timestamp,
            p->acc_x, p->acc_y, p->acc_z, p->gyro_x, p->gyro_y);
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINTF("\n\nhalo world\n");

    // Use AVCC and AVSS as voltage reference
    hplAdcUseSupplyRef();
    hplAdcOn();

    enableAdcPin(6, ACCEL_X);
    enableAdcPin(6, ACCEL_Y);
    enableAdcPin(6, ACCEL_Z);
    enableAdcPin(6, GYRO_X);
    enableAdcPin(6, GYRO_Y);

    // enable sensor board
    enableSensorPower();

    mdelay(START_DELAY);

    redLedOn();

    for (;;) {
        uint32_t now = getTimeMs();
        uint32_t endTime = now + MAIN_LOOP_LENGTH;

        Packet_t p;
        p.timestamp = now;

        adcSetChannel(ACCEL_X);
        p.acc_x = adcReadFast();
        adcSetChannel(ACCEL_Y);
        p.acc_y = adcReadFast();
        adcSetChannel(ACCEL_Z);
        p.acc_z = adcReadFast();
        adcSetChannel(GYRO_X);
        p.gyro_x = adcReadFast();
        adcSetChannel(GYRO_Y);
        p.gyro_y = adcReadFast();

        printPacket(&p);

        while (timeAfter32(endTime, getTimeMs()));
    }
}
