/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of  conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "alarm.h"
#include <platform.h>

//----------------------------------------------------------
// internal variables
//----------------------------------------------------------
// we will hold an ordered list of alarms where each
// element contains relative pause in milliseconds with respect
// to the previous alarm object
static Alarm_t *head; // first alarm in the list

//----------------------------------------------------------
// internal function declarations
//----------------------------------------------------------
static void setAlarmTimer(Alarm_t *alarm);
#define clockAdd(alarm1, alarm2) { alarm1->msecs += alarm2->msecs; }

//----------------------------------------------------------
// function implementations
//----------------------------------------------------------

void alarmInit(Alarm_t *obj, AlarmFunc_p callback,
        uint32_t msec, bool periodic, void *data) {
    obj->callback = callback;
    obj->data = data;
    obj->msecs = msec;
    obj->nextMsec = (periodic ? msec : 0);
}

void alarmInitAndRegister(Alarm_t *obj, AlarmFunc_p callback,
        uint32_t msec, bool periodic, void *data) {
    alarmInit(obj, callback, msec, periodic, data);
    alarmRegister(obj);
}


static Alarm_t *curr_p; // used to traverse list
void fireAlarm(void)
{
    curr_p = head;
    while(curr_p)
    {
        if (curr_p->msecs == 0)
        {
            curr_p->callback(curr_p->data);
            head = curr_p->next;

            if (curr_p->nextMsec != 0)
            {
                // reset the msecs to the appropriate value
                curr_p->msecs = curr_p->nextMsec;

                setAlarmTimer(curr_p);
            }
        }
        else {
            break;
        }
        curr_p = head;

    }
}

bool haveAlarms(void)
{
    return (head != 0);
}

Alarm_t* getNextAlarm(void)
{
   return head;
}


bool removeAlarm(Alarm_t *alarm)
{
    if (alarm == 0) return false;

    // we don't want the alarm system to service an alarm
    // while we're trying to remove one.
    platformTurnAlarmsOff();

    Alarm_t *current = head;
    Alarm_t *prev = 0;

    while (current)
    {
        if (current == alarm) {
            //remove this alarm
            current->nextMsec = 0;
            if (current == head) {
                //removing head element
                head = current->next;
            } else if (prev) {
                // somewhere in the middle
                prev->next = current->next;
            }
            if (current->next) {
                //add relative time to next timer
                clockAdd(current->next, current);
            }
            platformTurnAlarmsOn();
            return true;
        }
        prev = current;
        current = current->next;
    }

    platformTurnAlarmsOn();
    return false;
}

void alarmRegister(Alarm_t *new)
{
    platformTurnAlarmsOff();
    if (new->msecs == 0) {
        new->msecs = 1;
    }
    setAlarmTimer(new);
    platformTurnAlarmsOn();
}


static uint32_t totalMsecs;
static Alarm_t *satCurr, *satPrev;
static uint8_t satIter;

static void setAlarmTimer(Alarm_t *alarm)
{
    satIter = 0;
    totalMsecs = 0;
    satPrev = 0;

    // HACK - we should never have a duplicate alarm on the list
    removeAlarm(alarm);

    if (!head)
    {
        head = alarm;
        head->next = 0;
        return;
    }

    // init the alarm pointer after we ensure it's not on the list
    alarm->next = 0;

    for(satCurr = head; satCurr; satCurr = satCurr->next)
    {
        totalMsecs += satCurr->msecs;

        if (alarm->msecs < totalMsecs)
        {
            // middle of the list
            if (satPrev)
            {
                // somewhere in the middle, insert
                satPrev->next = alarm;
                alarm->next = satCurr;
                // recalculate the differences
                alarm->msecs -= totalMsecs - satCurr->msecs;
                satCurr->msecs -= alarm->msecs;
                return;
            } else {
                // head of the list
                alarm->next = head;
                head = alarm;
                // recalculate the differences
                alarm->next->msecs -= alarm->msecs;
                return;
            }
        }

        satPrev = satCurr;
    }

    // end of the list
    if (!satPrev) return;

    satPrev->next = alarm;
    alarm->next = 0;
    alarm->msecs -= totalMsecs;

    return;
}
