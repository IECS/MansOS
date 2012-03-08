// config:
// USE_LIGHT=y
//
// CONST_SERIAL_PORT_BAUDRATE 38400

#include "stdmansos.h"

#define LIGHT_PERIOD 2000

Alarm_t lightAlarm;

static inline void serialPrintU16(const char *name, uint16_t value)
{
    PRINTF("%s=%u\n", name, value);
}

void lightAlarmCallback(void *param)
{
    uint16_t light = readLight();
    serialPrintU16("light", light);
    alarmSchedule(&lightAlarm, LIGHT_PERIOD);
}

void appMain(void)
{
    alarmInit(&lightAlarm, lightAlarmCallback, NULL);
    alarmSchedule(&lightAlarm, LIGHT_PERIOD);
}
