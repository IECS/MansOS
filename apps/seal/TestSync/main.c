#include <stdmansos.h>
#include <string.h>
#include <lib/codec/crc.h>
#include <random.h>
#include <hil/humidity.h>
#include "schedtest.h"

// -----------------------------
// Constants

#define NUM_CONDITIONS 0
#define DEFAULT_CONDITION 0

#define SLOWREAD1_PERIOD    2000
#define SLOWREAD1_TIME      500
#define SLOWREAD1_NO_VALUE  0xffff

#define SLOWREAD2_PERIOD    2000
#define SLOWREAD2_TIME      300
#define SLOWREAD2_NO_VALUE  0xffff

#define SLOWREAD3_PERIOD    3000
#define SLOWREAD3_TIME      100
#define SLOWREAD3_NO_VALUE  0xffff
// -----------------------------
// Types, variables

bool oldConditionStatus[NUM_CONDITIONS + 1];
Alarm_t slowRead1Alarm;
Alarm_t slowRead2Alarm;
Alarm_t slowRead3Alarm;
// -----------------------------
// Outputs

// -----------------------------
// Callbacks

void slowPreread1Callback(void *__unused)
{
    slowPreread1();
    slowRead1Alarm.callback = slowRead1Callback;
    alarmSchedule(&slowRead1Alarm, SLOWREAD1_TIME);
}
void slowRead1Callback(void *__unused)
{
    uint16_t slowRead1 = slowRead1();
    slowRead1Alarm.callback = slowPreread1Callback;
    alarmSchedule(&slowRead1Alarm, SLOWREAD1_PERIOD - SLOWREAD1_TIME);
}

void slowPreread2Callback(void *__unused)
{
    slowPreread2();
    slowRead2Alarm.callback = slowRead2Callback;
    alarmSchedule(&slowRead2Alarm, SLOWREAD2_TIME);
}
void slowRead2Callback(void *__unused)
{
    uint16_t slowRead2 = slowRead2();
    slowRead2Alarm.callback = slowPreread2Callback;
    alarmSchedule(&slowRead2Alarm, SLOWREAD2_PERIOD - SLOWREAD2_TIME);

    // TODO: sync the alarm with slow read 1, because period & branch is the same...
}

void slowPreread3Callback(void *__unused)
{
    slowPreread3();
    slowRead3Alarm.callback = slowRead3Callback;
    alarmSchedule(&slowRead3Alarm, SLOWREAD3_TIME);
}
void slowRead3Callback(void *__unused)
{
    uint16_t slowRead3 = slowRead3();
    slowRead3Alarm.callback = slowPreread3Callback;
    alarmSchedule(&slowRead3Alarm, SLOWREAD3_PERIOD - SLOWREAD3_TIME);
}

// -----------------------------
// Conditions

// -----------------------------
// Branches

void branch0Start(void)
{
    // TODO: 
    // 1) calc max of read times (m_t)
    // 2) schedule preread callbacks at time (m_t - read_time_i)

    slowPreread1Callback(NULL);
    slowPreread2Callback(NULL);
    slowPreread3Callback(NULL);
}

// TODO: add branch continue function (

void branch0Stop(void)
{
    alarmRemove(&slowRead1Alarm);
    alarmRemove(&slowRead2Alarm);
    alarmRemove(&slowRead3Alarm);
}

// -----------------------------
// Main function

void appMain(void)
{
    alarmInit(&slowRead1Alarm, slowRead1Callback, NULL);
    alarmInit(&slowRead2Alarm, slowRead2Callback, NULL);
    alarmInit(&slowRead3Alarm, slowRead3Callback, NULL);

    for (;;) {
        uint32_t iterationEndTime = getRealTime() + 1000;

        bool newConditionStatus[NUM_CONDITIONS + 1];
        newConditionStatus[DEFAULT_CONDITION] = true;

        bool branch0OldStatus = oldConditionStatus[DEFAULT_CONDITION];

        bool branch0NewStatus = newConditionStatus[DEFAULT_CONDITION];

        if (branch0OldStatus != branch0NewStatus) {
            if (branch0NewStatus) branch0Start();
            else branch0Stop();
        }

        memcpy(oldConditionStatus, newConditionStatus, sizeof(oldConditionStatus));

        uint32_t now = getRealTime();
        if (timeAfter32(iterationEndTime, now)) {
            msleep(iterationEndTime - now);
        }
    }
}
