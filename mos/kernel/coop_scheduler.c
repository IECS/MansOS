/**
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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

#include "coop_scheduler.h"
#include "alarm.h"


//----------------------------------------------------------
// file scope globals
//----------------------------------------------------------
static Alarm_t alarm;
//TODO: must use single 16bit word for all signals and set flags
// as individual bits.
static uint8_t signalAlarm = 0;
static struct TaskConfig_t tasks[MAX_TASK_COUNT];


//----------------------------------------------------------
// local functions
//----------------------------------------------------------
static void handleAlarm(void *x) {
    signalAlarm = 1;
    return;
}

static void setAlarm(uint16_t timerLenght) {
    alarmInitAndRegister(&alarm, handleAlarm, timerLenght, true, NULL);
    return;
}

static uint16_t addWaitingTask(uint8_t currentTask, uint16_t oldTimerLenght) {
    uint8_t i;
    uint16_t newTimerLenght;
    // Sanity check the requested amount of time.
    if(taskRequest.timeToWait >= MIN_TIME_TO_WAIT && \
            taskRequest.timeToWait <= MAX_TIME_TO_WAIT) {
        newTimerLenght = taskRequest.timeToWait;
        // If there was no timer set.
        if(oldTimerLenght == 0) {
            // Set the alarm.
            setAlarm(newTimerLenght);
            tasks[currentTask].offsetTime = 0;
            return newTimerLenght;
        // If there is a que.
        } else {
            // If the current task can go first.
            if(oldTimerLenght >= newTimerLenght) {
                setAlarm(newTimerLenght);
                // Look for other waiting tasks in the que.
                for(i=0; i < MAX_TASK_COUNT; i++) {
                    if(i != currentTask && tasks[i].state == WAITING) {
                        // Increase relative timeToWait because now the timer is shorter.
                        tasks[i].offsetTime += (oldTimerLenght - \
                                newTimerLenght);
                    }
                    // We are the first in the que, so no extra time to wait after the alarm.
                    tasks[currentTask].offsetTime = 0;
                }
                return newTimerLenght;
            // If the current task must line up with the others.
            } else {
                // We need to wait longer than the first one in the que by the time diff.
                tasks[currentTask].offsetTime = newTimerLenght - oldTimerLenght;
                return oldTimerLenght;
            }
        }
    // Sanity check fail.
    } else {
        tasks[currentTask].state = READY;
        return oldTimerLenght;
    }
}



//----------------------------------------------------------
// visible functions
//----------------------------------------------------------
void registerTask(CoroPointer_t coroPointer) {
    uint8_t i;
    for(i = 0; i < MAX_TASK_COUNT; i++) {
        if(tasks[i].state == STOPPED) {
            tasks[i].coroPointer = coroPointer;
            tasks[i].state = READY;
            tasks[i].offsetTime = 0;
            return;
        }
    }
    return;
}


void coopSchedStart() {
    uint16_t timerLenght = 0;
    uint8_t currentTask;
    uint8_t i;

    // Fill the task array with stopped tasks.
    for(currentTask=0; currentTask < MAX_TASK_COUNT; currentTask++) {
        tasks[currentTask].state = STOPPED;    
    }

    // Let the user app to init and register tasks.
    appMain();

    // Main loop of the scheduler.
    while(1) {
        //Check the alarm flag to see if any waiting tasks should be made ready.
        if(signalAlarm) {
            signalAlarm = 0;
            timerLenght = -1u; // no one should have such great offset
            for(i=0; i < MAX_TASK_COUNT; i++) {
                if(tasks[i].state == WAITING) {
                    // Wake up the waiting tasks that are due.
                    if(tasks[i].offsetTime < MIN_TIME_TO_WAIT) { 
                        tasks[i].state = READY;
                    // Find the next smallest time for a new timer.
                    } else if(tasks[i].offsetTime < timerLenght) {
                        timerLenght = tasks[i].offsetTime;
                    }
                }
            }
            // If there is some more tasks that need to wait.
            if(timerLenght < -1u){
                // Set the new timer.
                setAlarm(timerLenght);
                // Decrease tasks timeToWait.
                for(i=0; i < MAX_TASK_COUNT; i++) {
                    if(tasks[i].state == WAITING) {
                        tasks[i].offsetTime -= timerLenght;
                    }
                }
            }
        }
        //TODO: check if all tasks are sleeping so we can go lowpower. (we can't as we are 
        // on user space and kernel threads might not be happy?)
    
        // Run tasks.
        for(currentTask = 0; currentTask < MAX_TASK_COUNT; currentTask++) {
            // If task is Ready.
            if( tasks[currentTask].state == READY ) {
                // Run and update state.
                tasks[currentTask].state = (tasks[currentTask].coroPointer)();
                // If task returned because of a time wait.
                if(tasks[currentTask].state == WAITING) {
                    timerLenght = addWaitingTask(currentTask, timerLenght);
                // If task is waiting for serrial msg.
                } else if (tasks[currentTask].state == BLOCKED) {
                    //...
                }
            // If current task is interested in serial buf.
            } else if (tasks[currentTask].state == BLOCKED) {
                //...
            }
        }
    }
    return;
}
