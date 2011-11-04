/*
Ko tai progai vajadzeetu dariit:
 - sampleet gps
 - lasiit accel + gyro (100x sekundee vai kaut kur taa...)
 - glabaat datus flashaa
 - peec rebuuta nomarkjeet ("te ir bijis reboots")
 - peec paarprogrammeeshanas maaceet datus sutiit laukaa pa seriaalo portu
 - uz PC vajag programmu, kas tos datus noparsee un noglabaa kaut kaadaa csv failaa.
*/

#include "stdmansos.h"
#include "dprint.h"
#include <hil/sleep.h>
#include <hil/timers.h>
#include <user_button.h>
//#include <lib/nmea/nmea_types.h>
//#include <lib/nmea/nmea.h>
//#include <lib/nmea/nmea_stream.h>
#include <string.h>
#include "mmc.h"
#include "sdcardstream.h"

#define MAIN_LOOP_LENGTH 10 // milliseconds

void gpsFlush(void);
void gpsReset(void);

#define enableAdcPin(port, pin) \
    pinAsFunction(port, pin); \
    pinAsInput(port, pin);

#define enableSensorPower() \
    pinAsOutput(1, 7); \
    pinSet(1, 7);

// Artis: uz 2.6 pinu jaapadod 0
#define enableGps() \
    pinAsOutput(2, 6); \
    pinClear(2, 6);

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

// ------------------------------------------
// SD card support
// ------------------------------------------

unsigned char sdCardStatus = 1;

void initSdCard()
{
    unsigned int timeout = 0;

    //Initialisation of the MMC/SD-card
    while (sdCardStatus != 0) {                      // if return in not NULL an error did occur and the
        // MMC/SD-card will be initialized again 
        sdCardStatus = mmcInit();
        timeout++;
        if (timeout == 150) {                     // Try 50 times till error
            //printf ("No MMC/SD-card found!! %x\n", status);
            break;
        }
    }

    while ((mmcPing() != MMC_SUCCESS));      // Wait till card is inserted
}

// ------------------------------------------
// user button support
// ------------------------------------------

uint16_t totalUbChanges;

void buttonStateChanged() {
    ++totalUbChanges;
    if (totalUbChanges <= 2) return;

//    if (userButtonGet() == BUTTON_RELEASED) {
//        redLedOn();
//        sdcardStreamClear();
//        mdelay(1000);
//        redLedOff();
//    }

    if (userButtonGet() == BUTTON_RELEASED) {
        gpsFlush();
    }
}

// ------------------------------------
// flash writing stuff
// ------------------------------------

#define DATA_MAGIC_NUMBER  0xbaadf00dul

typedef struct Packet {
    uint32_t magic;
    uint32_t timestamp;
    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;
    uint16_t gyro_x;
    uint16_t gyro_y;
    uint16_t __padding;
} Packet_t PACKED;

bool flashWritePacket(Packet_t *p) {
    p->magic = DATA_MAGIC_NUMBER;
    p->timestamp = getRealTime();
    return sdcardStreamWrite((void *) p, sizeof(*p));
}

//-------------------------------------------
// GPS
//-------------------------------------------

static char gpsMemBuffer[8000];
static uint16_t gpsMemBufferPosition;
static bool gpsFlushed;

void gpsFlush(void)
{
    if (gpsFlushed) return;

    static uint8_t zero[20] = {0};
    sdcardStreamWrite(zero, sizeof(zero));
    sdcardStreamWrite((void *) gpsMemBuffer, gpsMemBufferPosition);

    char ca[] = "----------------------------------\n";
    sdcardStreamWrite((void *) ca, sizeof(ca));

    sdcardStreamFlush();
    gpsFlushed = true;
}

void gpsReset(void)
{
    gpsMemBufferPosition = 0;
    gpsFlushed = false;
}

void gpsCharRecv(uint8_t b)
{
    static uint32_t startTime;
    if (startTime == 0) startTime = getRealTime();
    gpsMemBuffer[gpsMemBufferPosition++] = b;
    if (gpsMemBufferPosition >= sizeof(gpsMemBuffer)) {
        PRINTF("buffer end reached in %lu milliseconds\n", getRealTime() - startTime);
        USARTSetReceiveHandle(0, NULL);
        gpsFlush();
    }
}

//-------------------------------------------
// Sensors
//-------------------------------------------

void readSensors()
{
    Packet_t packet;
    adcSetChannel(ACCEL_X);
    packet.acc_x = adcReadFast();
    adcSetChannel(ACCEL_Y);
    packet.acc_y = adcReadFast();
    adcSetChannel(ACCEL_Z);
    packet.acc_z = adcReadFast();

    adcSetChannel(GYRO_X);
    packet.gyro_x = adcReadFast();
    adcSetChannel(GYRO_Y);
    packet.gyro_y = adcReadFast();

    flashWritePacket(&packet);
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // PRINT_INIT(128); 

    // mdelay(1000);

    userButtonEnable(buttonStateChanged);

    // initialize SD card
    initSdCard();

    if (sdCardStatus) {
        for (;;) {
            redLedOn();
            mdelay(100);
            redLedOff();
            mdelay(100);
        }
    }

    // XXX: always
    // if (!sdcardStreamVerifyChecksums()) {
    //     sdcardStreamClear();
    //     redLedOn();
    //     mdelay(200);
    //     redLedOff();
    //     mdelay(200);
    //     redLedOn();
    //     mdelay(200);
    //     redLedOff();
    // }

//    static uint8_t ca[] = "---------------------------------xxxxxxx";
//    sdcardStreamWrite(ca, sizeof(ca));
//    sdcardStreamFlush();
//    mmcWriteBlock(0x10000ul, 512, ca);
//    return;

    enableGps();

    // init UART0 for communication with GPS
    USARTInit(0, 115200, 0);
    USARTSetReceiveHandle(0, gpsCharRecv);

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

    // turn on RX interrupt
    USARTEnableRX(0);

    // we are working, signal it with LED state!
    redLedOn();

    while (!gpsFlushed) {
        uint32_t endTime = getRealTime() + MAIN_LOOP_LENGTH;
        readSensors();
        while (timeAfter(endTime, getRealTime()));
    }
}
