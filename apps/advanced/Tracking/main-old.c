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
#include <hil/extflash.h>

#include <lib/nmea/nmea_types.h>
#include <lib/nmea/nmea.h>
#include <lib/nmea/nmea_stream.h>
#include <string.h>

#define MAIN_LOOP_LENGTH 1 // seconds

#define enableAdcPin(port, pin) \
    pinAsFunction(port, pin); \
    pinAsInput(port, pin);

#define enableSensorPower() \
    pinAsOutput(1, 7); \
    pinSet(1, 7);

// Artis: uz 2.6 pinu jaapadod 0
#define enableGps() \
    pinAsOutput(2, 6); \
    pinClr(2, 6);

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
    return flashStreamWrite(p, sizeof(*p));
}

// bool writeData() {
//     uint8_t buffer[100];

//     Datablock_t *db;
//     Packet_t *pck;
//     AccelGyroData_t *agd;
//     uint8_t *s;

//     uint16_t i;

//     toggleGreenLed();

//     for (i = 0; i < NUM_RECORDS; ++i) {
//     // -- a packet
//     db = (Datablock_t *) buffer;
//     db->magic = DATA_MAGIC_NUMBER;
//     db->dataType = DB_PACKET;
//     db->__padding1 = 0;
//     db->blockLen = sizeof(Datablock_t) + sizeof(Packet_t);
//     db->timestamp = getRealTime();

//     pck = (Packet_t *)db->data;
//     pck->originator = localNetworkAddress;
//     pck->light = 123;
//     pck->temperature = 20;
//     pck->activity = 13;
//     pck->gpsLat = 111111;
//     pck->gpsLon = 222222;

//     if (!flashStreamWrite(buffer, db->blockLen)) return false;

//     // -- accel and gyro data
//     db = (Datablock_t *) buffer;
//     db->magic = DATA_MAGIC_NUMBER;
//     db->dataType = DB_ACCEL_GYRO;
//     db->__padding1 = 0;
//     db->blockLen = sizeof(Datablock_t) + sizeof(AccelGyroData_t);
//     db->timestamp = getRealTime();

//     agd = (AccelGyroData_t *)db->data;
//     agd->acc_x = 1;
//     agd->acc_y = 2;
//     agd->acc_z = 3;
//     agd->gyro_x = 45;
//     agd->gyro_y = 90;
//     agd->__padding = 0;

//     if (!flashStreamWrite(buffer, db->blockLen)) return false;

//     // -- just a debug string
// #define TEST_STRING  "hello world"
//     db = (Datablock_t *) buffer;
//     db->magic = DATA_MAGIC_NUMBER;
//     db->dataType = DB_STRING;
//     db->__padding1 = 0;
//     db->blockLen = sizeof(Datablock_t) + sizeof(TEST_STRING) - 1;
//     db->timestamp = getRealTime();

//     s = (uint8_t *)db->data;
//     memcpy(s, TEST_STRING, sizeof(TEST_STRING) - 1);
//     if (!flashStreamWrite(buffer, db->blockLen)) return false;

//     }

//     flashStreamFlush();

//     PRINTF("all data written!\n");

//     toggleGreenLed();
//     return true;
// }

//-------------------------------------------
// GPS
//-------------------------------------------
static GPSFix_t fix;

enum {
    // get GPS fix every hour
    // GPS_SAMPLE_PERIOD = 60ull * 60ull * 1000ull,
    // search for satellites for 60s
    // GPS_RETRY_TIMEOUT = 60ull * 1000ull

    GPS_SAMPLE_PERIOD = 1000ul,

    //GPS_RETRY_TIMEOUT = 1000ul,
    //GPS_RETRY_TIMEOUT = 10000ul,
    GPS_RETRY_TIMEOUT = 60000ul,
};

#define CHECK_CMD(cmd) \
    if (nmeaBufState[NMEA_CMD_##cmd] == BS_READY) { \
        nmeaBufState[NMEA_CMD_##cmd] = BS_PROCESSING; \
        parseRMC(nmeaBuf[NMEA_CMD_##cmd] + 1, \
            MAX_NMEA_CMD_SIZE - 1, &fix); \
        nmeaBufState[NMEA_CMD_##cmd] = BS_EMPTY; \
        waitCmd++; \
    }

uint_t gpsFixOk(GPSFix_t *fix)
{
    return fix->d.year >= 10 // year >= 2010
        && fix->d.mon > 0
        && fix->d.day > 0
        && fix->q.fix != FT_NO_FIX;
}

// try to get GPS fix for some period of time. Parse GGA, GSA & RMC commands
// return 1 on success, 0 otherwise
uint_t tryGPSFix(uint32_t tryEnd)
{
    uint32_t thisTime = getRealTime();
    // turn on RX interrupt
    //    USARTEnableRX(0);

    // wait the whole sequence of commands one after another
    static uint_t waitCmd = NMEA_CMD_GGA;
    uint_t fixOk = 0;

    // invalidate fix;
    memset(&fix, 0, sizeof(fix));

    // check cmd buffers, wait for data to arrive
    while (thisTime < tryEnd) {
        switch (waitCmd) {
        case NMEA_CMD_GGA:
            CHECK_CMD(GGA);
            break;
        case NMEA_CMD_GSA:
            CHECK_CMD(GSA);
            break;
        case NMEA_CMD_RMC:
            CHECK_CMD(RMC);
            break;
        }

        // PRINTF("waitCmd=%u\n", waitCmd);

        thisTime = getRealTime();

        if (waitCmd == NMEA_CMD_COUNT) {
            // all commands parsed, check fix quality
            if (gpsFixOk(&fix)) {
                fixOk = 1;
                break;
            }
            else {
                PRINTF("year=%u mon=%u day=%u fix=%u\n",
                        fix.d.year, fix.d.mon, fix.d.day, fix.q.fix);
            }
        }

        busyWait(100000); // play some rock
    }
    //USARTDisableRX(0);

    return fixOk;
}

void sampleGPS()
{
    // read GPS periodically
    static uint32_t lastGPSfix = 0;
    const uint32_t thisTime = getRealTime();

    if (thisTime - lastGPSfix >= GPS_SAMPLE_PERIOD) {
        if (tryGPSFix(thisTime + GPS_RETRY_TIMEOUT)) {
            // save time of last successful fix
            lastGPSfix = thisTime;
            PRINTF("gps ok lat=%u.%u long=%u.%u\n",
                    fix.lat.deg, fix.lat.min,
                    fix.lon.deg, fix.lat.min);
            PRINTF("   fix=%u dop=%u satcnt=%u\n",
                    fix.q.fix, fix.q.dop, fix.q.satcnt);
        }
        else {
            PRINTF("gps not ok\n");
        }
    }
}

void gpsCharRecv(uint8_t b)
{
    static char gpsMemBuffer[8000];
    static uint16_t position;
    static uint32_t startTime;
    if (startTime == 0) startTime = getRealTime();
    gpsMemBuffer[position++] = b;
    if (position >= sizeof(gpsMemBuffer)) {
        PRINTF("buffer end reached in %lu milliseconds\n", getRealTime() - startTime);
        USARTSetReceiveHandle(0, NULL);
        position = 0;
        startTime = getRealTime();
    }
    if ((position & 0x7f) == 0) PRINT(".");

/*
    static uint8_t position;
    static char buffer[100];
    buffer[position++] = b;
    if (position >= sizeof(buffer) - 1 || b == NMEAEND) {
        position = 0;
        PRINTF(buffer);
        PRINT("\n");
        memset(buffer, 0, sizeof(buffer));
    }
*/
}

//-------------------------------------------
// Sensors
//-------------------------------------------
void readSensors()
{
    uint8_t i;
    uint16_t a[TOTAL_ADC];
    static uint16_t old[TOTAL_ADC];

//    while (1) {
        for (i = 0; i < TOTAL_ADC; ++i) {
            adcSetChannel(i);
            a[i] = adcReadFast();
        }
        for (i = 0; i < TOTAL_ADC; ++i) {
            PRINTF("%u ", abs(a[i] - old[i]) >> 4);
            old[i] = a[i];
        }
        PRINT("\n");
//        mdelay(100);
//    }
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINT_INIT(128); 

    // initialize the external flash
    // extFlashWake();

    enableGps();

    // init UART0 for communication with GPS
    if (USARTInit(0, 115200, 0) == 0) redLedOn();
    // USARTSetReceiveHandle(0, nmeaCharRecv);
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

    for (;;) {
//        uint32_t endTime = getRealTime() + MAIN_LOOP_LENGTH * 1000;
//        Packet_t pck;

//        sampleGPS();
//        readSensors();

//        pck.version = PACKET_FORMAT_VERSION;
//        pck.originator = ORIGINATOR_ID;
//        pck.light = light;
//        pck.temperature = temp;
//        pck.humidity = humidity;
//        pck.activity = 13; // TODO
//        pck.gpsLat = fix.lat;
//        pck.gpsLon = fix.lon;
        // alternative: use real timestamp or use sequence number
        // pck.timestamp = getRealTime();
//        static uint32_t seqnum;
//        pck.timestamp = seqnum++;

        // log the packet to external flash
//        PRINTF("write to flash...\n");
//        flashWritePacket(&pck);

        // sleep for some time
//        PRINTF("sleep...\n");
//        while (timeAfter(endTime, getRealTime()));
    }
}
