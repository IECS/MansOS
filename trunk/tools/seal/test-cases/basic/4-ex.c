// read Light, period 2s;
// read Humidity, period 2s;
//
// sink Radio;

// --------------------------

// config:
// USE_LIGHT=y
// USE_HUMIDITY=y
// USE_RADIO=y

#include "stdmansos.h"

typedef struct RadioPacket_s {
    uint16_t light;
    uint16_t humidity;
} RadioPacket_t;

#define LIGHT_NO_VALUE    0xffff
#define HUMIDITY_NO_VALUE 0xffff

#define LIGHT_PERIOD      2000
#define HUMIDITY_PERIOD   2000

#define RADIO_PACKET_NUM_FIELDS 2

RadioPacket_t radioPacket;
uint16_t radioPacketNumFieldsFull;

Alarm_t lightAlarm;
Alarm_t humidityAlarm;

// ----------------------------------

static inline void radioPacketInit(void)
{
    radioPacketNumFieldsFull = 0;
    radioPacket.light = LIGHT_NO_VALUE;
    radioPacket.humidity = HUMIDITY_NO_VALUE;
}

static inline void radioPacketSend(void)
{
    // PRINT("send radio crap\n");
    radioSend(&radioPacket, sizeof(radioPacket));
    radioPacketInit();
}

static inline bool radioPacketIsFull(void)
{
    return radioPacketNumFieldsFull >= RADIO_PACKET_NUM_FIELDS;
}

// ----------------------------------

void lightAlarmCallback(void *param)
{
    if (radioPacket.light != LIGHT_NO_VALUE) {
        radioPacketSend();
    }
    radioPacket.light = readLight();
    radioPacketNumFieldsFull++;
    if (radioPacketIsFull()) {
        radioPacketSend();
    }

    alarmSchedule(&lightAlarm, LIGHT_PERIOD);
}

// ----------------------------------

void humidityAlarmCallback(void *param)
{
    if (radioPacket.humidity != HUMIDITY_NO_VALUE) {
        radioPacketSend();
    }
    radioPacket.humidity = readHumidity();
    radioPacketNumFieldsFull++;
    if (radioPacketIsFull()) {
        radioPacketSend();
    }

    alarmSchedule(&humidityAlarm, HUMIDITY_PERIOD);
}

// ----------------------------------

void appMain(void)
{
    radioPacketInit();

    alarmInit(&lightAlarm, lightAlarmCallback, NULL);
    alarmSchedule(&lightAlarm, LIGHT_PERIOD);

    alarmInit(&humidityAlarm, humidityAlarmCallback, NULL);
    alarmSchedule(&humidityAlarm, HUMIDITY_PERIOD);
}
