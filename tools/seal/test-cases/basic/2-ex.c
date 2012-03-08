// read Light, period=2s;
// read Humidity, period=2s;
//
// sink Serial, baudrate=38400, packet;

// --------------------------

// config:
// USE_LIGHT=y
// USE_HUMIDITY=y

/* enum { */
/*     LIGHT_INDEX, */
/*     HUMIDITY_INDEX, */
/*     TOTAL_INDEXES, */
/* }; */

#include "stdmansos.h"

typedef struct SerialPacket_s {
    uint16_t light;
    uint16_t humidity;
} SerialPacket_t;

#define LIGHT_NO_VALUE    0xffff
#define HUMIDITY_NO_VALUE 0xffff

#define LIGHT_PERIOD      2000
#define HUMIDITY_PERIOD   2000

#define SERIAL_PACKET_NUM_FIELDS 2

SerialPacket_t serialPacket;
uint16_t serialPacketNumFieldsFull;

Alarm_t lightAlarm;
Alarm_t humidityAlarm;

// ----------------------------------

static inline void serialPrintU16(const char *name, uint16_t value)
{
    PRINTF("%s=%u\n", name, value);
}

static inline void serialPacketPrint(void)
{
    PRINT("======================\n");
    serialPrintU16("light", serialPacket.light);
    serialPrintU16("humidity", serialPacket.humidity);
}

static inline void serialPacketInit(void)
{
    serialPacketNumFieldsFull = 0;
    serialPacket.light = LIGHT_NO_VALUE;
    serialPacket.humidity = HUMIDITY_NO_VALUE;
}

static inline void serialPacketSend(void)
{
    serialPacketPrint();
    serialPacketInit();
}

static inline bool serialPacketIsFull(void)
{
    /* if (p->light == LIGHT_NO_VALUE) return false; */
    /* if (p->humidity == HUMIDITY_NO_VALUE) return false; */
    /* return true; */
    return serialPacketNumFieldsFull >= SERIAL_PACKET_NUM_FIELDS;
}

// ----------------------------------

void lightAlarmCallback(void *param)
{
    if (serialPacket.light != LIGHT_NO_VALUE) {
        serialPacketSend();
    }
    serialPacket.light = readLight();
    serialPacketNumFieldsFull++;
    if (serialPacketIsFull()) {
        serialPacketSend();
    }

    alarmSchedule(&lightAlarm, LIGHT_PERIOD);
}

// ----------------------------------

void humAlarmCallback(void *param)
{
    if (serialPacket.humidity != HUMIDITY_NO_VALUE) {
        serialPacketSend();
    }
    serialPacket.humidity = readHumidity();
    serialPacketNumFieldsFull++;
    if (serialPacketIsFull()) {
        serialPacketSend();
    }

//  serialPacketCheck(&serialPacket, HUMIDITY_INDEX);
//  serialPacket.humidity = readHumidity();
//  serialPacketCheck(&serialPacket, HUMIDITY_INDEX);

    alarmSchedule(&humidityAlarm, HUMIDITY_PERIOD);
}

// ----------------------------------

void appMain(void)
{
    serialPacketInit();

    alarmInit(&lightAlarm, lightAlarmCallback, NULL);
    alarmSchedule(&lightAlarm, LIGHT_PERIOD);

    alarmInit(&humidityAlarm, humAlarmCallback, NULL);
    alarmSchedule(&humidityAlarm, HUMIDITY_PERIOD);
}
