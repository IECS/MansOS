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

#include "threads.h"
#ifdef MCU_MSP430
#include "msp430/context_switch.h"
#elif MCU_AVR
#include "avr/context_switch.h"
#else
#error Define context switch macros for this architecture!
#endif
#include <lib/assert.h>
#include <lib/dprint.h>
#include <hil/sleep.h>
#include <string.h>

uint8_t threadStackBuffer[THREAD_STACK_SIZE * NUM_USER_THREADS];
static Thread_t threads[NUM_THREADS];

Thread_t *currentThread;
uint16_t jiffiesToSleep;

#if 0
#define THREADS_PRINTF(...) PRINTF(__VA_ARGS__)
#else
#define THREADS_PRINTF(...) do {} while (0)
#endif

// ------------------------------------------------------

#ifdef DEBUG_THREADS
void checkThreadLockups(void)
{
    uint16_t i;
    uint32_t now = (uint32_t) getJiffies();
    // note: this could be used for gathering time statistics as well!
    setSeenRunning(currentThread);
    for (i = 0; i < NUM_USER_THREADS; ++i) {
        switch (threads[i].state) {
        case THREAD_SLEEPING:
            if (timeAfter32(threads[i].sleepEndTime + LOCKUP_DETECT_TIME, now)) {
                PRINTF("thread %d not waking from sleep, now in thread #%d!\n", i, currentThread->index);
                goto fail;
            }
            break;
        case THREAD_READY:
            if (timeAfter32(now, threads[i].lastSeenRunning + LOCKUP_DETECT_TIME)) {
                PRINTF("thread %d not getting to run, now in thread #%d!\n", i, currentThread->index);
                PRINTF("now=%lu, last=%lu\n", now, threads[i].lastSeenRunning);
                goto fail;
            }
            break;
        default:
            break;
        }
    }
    return;

  fail:
    panic();
}
#endif // DEBUG_THREADS

static void threadWrapper(void)
{
    ASSERT(currentThread);
    setSeenRunning(currentThread);
    // execute the thread's function (may never return)
    currentThread->function();
    // mark the thread as having quit
    currentThread->state = THREAD_UNUSED;
    // switch to another thread (never returns)
    yield();
#ifdef DEBUG_THREADS
    ASSERT(false);
#endif
}

void threadCreate(uint_t index, ThreadFunc function)
{
    MemoryAddress_t stackAddress = (MemoryAddress_t) 
            (threadStackBuffer + index * THREAD_STACK_SIZE);

    threads[index].index = index;
    threads[index].state = THREAD_READY;
    threads[index].function = function;
    threads[index].priority = 0;

    // stack grows to the bottom; initial pointer must be at the end of memory region
    stackAddress += THREAD_STACK_SIZE;

    CONTEXT_SWITCH_PREAMBLE(threadWrapper, threads[index].sp);
}

void startThreads(ThreadFunc userThreadFunction, ThreadFunc kernelThreadFunction)
{
    THREADS_PRINTF("threadStackBuffer = 0x%04x - 0x%04x\n",
            threadStackBuffer, threadStackBuffer + sizeof(threadStackBuffer));
    memset(threadStackBuffer, 0, sizeof(threadStackBuffer));

    // create user thread
    threadCreate(0, userThreadFunction);

    // create kernel (default) thread
    threads[KERNEL_THREAD_INDEX].index = KERNEL_THREAD_INDEX;
    threads[KERNEL_THREAD_INDEX].state = THREAD_READY;
    threads[KERNEL_THREAD_INDEX].function = kernelThreadFunction;

    // save current execution point
    currentThread = &threads[KERNEL_THREAD_INDEX];
    PUSH_VAR(kernelThreadFunction);
    SAVE_ALL_REGISTERS();
    GET_SP(currentThread->sp);

    // start the user thread
    currentThread = &threads[0];
    currentThread->state = THREAD_RUNNING;
    SET_SP(currentThread->sp);
    RESTORE_ALL_REGISTERS();

    // return explicitly, without the usual epilogue,
    // which would try to restore registers and fail
    ASM_VOLATILE("ret");
}

void threadWakeup(uint16_t threadIndex, ThreadState_t newState)
{
    ASSERT(threadIndex < NUM_THREADS);
    Thread_t *thread = &threads[threadIndex];
    THREADS_PRINTF("threadWakeup, state=%d, new=%d\n",
            thread->state, newState);
    if (thread->state == THREAD_SLEEPING) {
        thread->state = newState;
    }
}

// --------------------------------------------------------------
// local variables used in function schedule()

#if NUM_USER_THREADS == 1

//
// Single user thread ("policy" has no effect and is ignored)
//
#define SELECT_NEXT_THREAD(policy) do {                                 \
    nextThread = currentThread;                                         \
    tmpThread = &threads[currentThread->index ^ 1];                     \
    switch (tmpThread->state) {                                         \
    case THREAD_READY:                                                  \
        nextThread = tmpThread;                                         \
        break;                                                          \
    case THREAD_SLEEPING:                                               \
        if (currentThread->state < THREAD_SLEEPING) {                   \
            nextThread = tmpThread;                                     \
            break;                                                      \
        }                                                               \
        if (currentThread->state > THREAD_SLEEPING) {                   \
            break;                                                      \
        }                                                               \
        if (timeAfter32(nextThread->sleepEndTime, tmpThread->sleepEndTime)) { \
            nextThread = tmpThread;                                     \
            if (!timeAfter32(tmpThread->sleepEndTime, now)) {           \
                nextThread->state = THREAD_READY;                       \
            }                                                           \
        }                                                               \
    default:                                                            \
        break;                                                          \
    }                                                                   \
    } while (0)

#else

//
// Multiple user threads + multiple scheduling policies
//
#define SELECT_NEXT_THREAD(policy)                                      \
    tmpThread = currentThread;                                          \
    do {                                                                \
        switch (tmpThread->state) {                                     \
        case THREAD_SLEEPING:                                           \
            if (!timeAfter32(tmpThread->sleepEndTime, now)) {           \
                tmpThread->state = THREAD_READY;                        \
            }                                                           \
            break;                                                      \
        case THREAD_READY:                                              \
        case THREAD_RUNNING:                                            \
            break;                                                      \
        default:                                                        \
            tmpThread = NULL;                                           \
            break;                                                      \
        }                                                               \
                                                                        \
        if (!tmpThread || !nextThread) {                                \
            if (!tmpThread) continue;                                   \
            goto useTmp;                                                \
        }                                                               \
        if (tmpThread->state != nextThread->state) {                    \
            if (tmpThread->state < nextThread->state) continue;         \
            goto useTmp;                                                \
        }                                                               \
        /* for sleeping threads, select the one that will wake up sooner */ \
        if (tmpThread->state == THREAD_SLEEPING) {                      \
            if (timeAfter32(tmpThread->sleepEndTime, nextThread->sleepEndTime)) { \
                continue;                                               \
            }                                                           \
        } /* always prioritize kernel */                                \
        else if (tmpThread->index == KERNEL_THREAD_INDEX) {             \
            goto useTmp;                                                \
        } else if (nextThread->index == KERNEL_THREAD_INDEX) {          \
            continue;                                                   \
        }                                                               \
        else if (policy == SCHEDULING_POLICY_ROUND_ROBIN) {             \
            /* for awake threads, select the one that was running least recently */  \
            if (timeAfter32(getLastSeenRunning(tmpThread), getLastSeenRunning(nextThread)))  { \
                continue;                                               \
            }                                                           \
        } else if (policy == SCHEDULING_POLICY_PRIORITY_BASED) {        \
            /* select the one with higher priority */                   \
            if (tmpThread->priority <= nextThread->priority) {          \
                continue;                                               \
            }                                                           \
        }                                                               \
      useTmp:                                                           \
        nextThread = tmpThread;                                         \
                                                                        \
        tmpThread++;                                                    \
        if (tmpThread >= threads + NUM_THREADS) {                       \
            tmpThread = threads;                                        \
        }                                                               \
    } while (tmpThread != currentThread);
#endif

//
// Schedule a new thread
//
NO_EPILOGUE void schedule(void)
{
    static Thread_t *nextThread;
    static Thread_t *tmpThread;
    static uint32_t now;

    SAVE_ALL_REGISTERS();
    now = (uint32_t)jiffies;
    // if 'jiffiesToSleep' is nonzero the current thread will be put to sleep
    currentThread->sleepEndTime = now + jiffiesToSleep;
    if (currentThread->state == THREAD_RUNNING) {
        currentThread->state = jiffiesToSleep ? THREAD_SLEEPING : THREAD_READY;
    }

    // find a thread to run now / after wakeup.
    // the thread with the shortest sleep time is selected and
    // system's sleep time adjusted accordingly;
    // if there are no threads ready to run at the present moment,
    // the system will go in low power mode.
    SELECT_NEXT_THREAD(SCHEDULING_POLICY);

    if (currentThread != nextThread) {
        SWITCH_THREADS(currentThread, nextThread);
        jiffiesToSleep = currentThread->sleepEndTime - now;
    }
    if (currentThread->state == THREAD_READY) {
        currentThread->state = THREAD_RUNNING;
    }

    THREADS_PRINTF("schedule: next thread will be %s\n",
            currentThread->index == KERNEL_THREAD_INDEX ? "system" : "user");

    if (currentThread->state == THREAD_SLEEPING) {
        THREADS_PRINTF("schedule: go to sleep for %u jiffies\n", jiffiesToSleep);
        doMsleep(jiffies2ms(jiffiesToSleep));
    } else {
        THREADS_PRINTF("schedule: keep running\n");
    }
    setSeenRunning(currentThread);
    RESTORE_ALL_REGISTERS();
    ASM_VOLATILE("ret");
}
