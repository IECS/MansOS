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

#ifndef _MANSOS_COOP_SCHEDULER_H_
#define _MANSOS_COOP_SCHEDULER_H_

#include "kernel/defines.h"

#ifdef USE_FIBERS


//----------------------------------------------------------
// constants
//----------------------------------------------------------
#ifndef MAX_TASK_COUNT
#define MAX_TASK_COUNT 4
#endif

#define MIN_TIME_TO_WAIT 10
#define MAX_TIME_TO_WAIT 1001 //XXX: what is the maximum time lenght?


//----------------------------------------------------------
// types
//----------------------------------------------------------
struct TaskRequest_t {
    uint16_t timeToWait;
    //...
};

typedef uint8_t (*CoroPointer_t)();

struct TaskConfig_t {
    CoroPointer_t coroPointer;
    enum  TaskState_e {STOPPED = 0, READY = 1, WAITING = 2, BLOCKED = 3} state;
    uint16_t offsetTime;
    //...
};


//----------------------------------------------------------
// globals
//----------------------------------------------------------
struct TaskRequest_t taskRequest;


//----------------------------------------------------------
// visible functions
//----------------------------------------------------------
void coopSchedStart();
void registerTask(CoroPointer_t);
void appMain();

//----------------------------------------------------------
// coroutine macros
//----------------------------------------------------------
#define REGISTER_TASK(coroPointer) registerTask(coroPointer)

#define DEF_TASK(taskName) uint8_t taskName()

#define BEGIN_TASK  static uint16_t TASK_LINE=0; \
                    switch(TASK_LINE) { \
                        case 0:;    \

#define END_TASK } return STOPPED

#define STOP_TASK return STOPPED

#define YIELD_TASK() do { \
                        TASK_LINE=__LINE__; \
                        return READY; \
                        case __LINE__:; \
                    } while(0)

#define WAIT_TASK(time) do { \
                        TASK_LINE=__LINE__; \
                        (&taskRequest)->timeToWait = time;  \
                        return WAITING; \
                        case __LINE__:; \
                    } while(0)

#define BLOCK_TASK(bufer) do { \
                        TASK_LINE=__LINE__; \
                        (&taskRequest)->buf = bufer; \
                        return BLOCKED; \
                        case __LINE__:; \
                    } while(0)


#endif // USE_FIBERS
#endif
