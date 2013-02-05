/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#include "stdmansos.h"
#include <kernel/stack.h>

void f(int parameter);
void g(void);
void h(void);

static int staticVariable;

void onAlarm(void *x) {}

// main function
void appMain(void)
{
    Alarm_t alarm;

    int localVariable;
    ASSERT(isStackAddress(&localVariable));
    f(localVariable);

    alarmInit(&alarm, onAlarm, NULL);
    alarmSchedule(&alarm, 1000);

    PRINTF("done!\n");

    for (;;);
}

void f(int parameter)
{
    int anotherLocalVariable;
    PRINTF("f=%p\n", &anotherLocalVariable);

    g();

    static int functionStaticVariable;
    ASSERT(isStackAddress(&parameter));
    ASSERT(isStackAddress(&anotherLocalVariable));

    ASSERT(!isStackAddress(&staticVariable));
    ASSERT(!isStackAddress(&functionStaticVariable));
    ASSERT(!isStackAddress(f));
    ASSERT(!isStackAddress(appMain));
}

void g(void)
{
    int anotherLocalVariable2;
    PRINTF("g=%p\n", &anotherLocalVariable2);
    h();

// uncomment this to create recursion!
//    f(1);
}

void h(void)
{
    int anotherLocalVariable3;
    PRINTF("h=%p\n", &anotherLocalVariable3);
}
