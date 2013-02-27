/*
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

//----------------------------------------------------------
// MSP430 Interrupts
//----------------------------------------------------------

#ifndef _MSP430_INT_H_
#define _MSP430_INT_H_

#include <defines.h>

#ifdef __GNUC__

#define ENABLE_INTS()   ASM_VOLATILE("eint"::)
#define DISABLE_INTS()  ASM_VOLATILE("dint"::)

#define GET_INTERRUPT_STATUS(status)  do {          \
        ASM_VOLATILE("mov r2, %0" : "=r" (status)); \
        status &= GIE;                              \
    } while (0)

#define SET_INTERRUPT_STATUS(status)                \
    ASM_VOLATILE("bis %0, r2" : : "r" (status))

// get stack pointer
#define GET_SP(sp)                                  \
    ASM_VOLATILE("mov.w r1, %0" :"=r"(sp) : );

// set stack pointer
#define SET_SP(sp)                                  \
    ASM_VOLATILE("mov.w %0, r1" :: "r"(sp) );
#define SET_SP_IMMED(sp)                            \
    ASM_VOLATILE("mov.w %0, r1" :: ""(sp) );

// push a 2-byte variable on the stack
#define PUSH_VAR(var)                               \
    ASM_VOLATILE("push %0" :: "r"(var) );

#else // IAR

#define ENABLE_INTS() __enable_interrupt()
#define DISABLE_INTS() __disable_interrupt()

#define GET_INTERRUPT_STATUS(status)                \
    status = __get_interrupt_state();

#define SET_INTERRUPT_STATUS(status)                \
    __set_interrupt_state(status);

#define GET_SP(sp)                                  \
    ASM_VOLATILE("mov sp, r4");                     \
    sp = __get_R4_register();                       \

#define SET_SP(sp)                                  \
    __set_R4_register(sp);                          \
    ASM_VOLATILE("mov r4, sp");                     \

#define PUSH_VAR(var)                               \
    __set_R4_register((MemoryAddress_t)var);        \
    ASM_VOLATILE("push r4");                        \

#endif


//
// Atomic block start and end
//
#define ATOMIC_START(handle) do {               \
        GET_INTERRUPT_STATUS(handle);           \
        DISABLE_INTS();                         \
        MEMORY_BARRIER();                       \
    } while (0)

#define ATOMIC_END(handle) do {                 \
        MEMORY_BARRIER();                       \
        SET_INTERRUPT_STATUS(handle);           \
    } while (0);

#define INTERRUPT_ENABLED_START(handle) do {    \
        GET_INTERRUPT_STATUS(handle);           \
        ENABLE_INTS();                          \
        MEMORY_BARRIER();                       \
    } while (0)

#define INTERRUPT_ENABLED_END(handle) do {      \
        MEMORY_BARRIER();                       \
        SET_INTERRUPT_STATUS(handle);           \
    } while (0);

// structure for saving state of all interrupts
typedef struct Msp430InterruptContext {
    Handle_t interruptBit;
    int watchdogStatus;
#ifdef IE1
    int ie1, ie2;
#endif
} Msp430InterruptContext_t;

static inline void msp430ClearAllInterrupts(Msp430InterruptContext_t *interruptContext)
{
    // Mask all interrupts that can be masked
    GET_INTERRUPT_STATUS(interruptContext->interruptBit);
    DISABLE_INTS();
    // Disable watchdog timer
#define WDTRPW               0x6900  // Watchdog key returned by read
    interruptContext->watchdogStatus = WDTCTL;
    interruptContext->watchdogStatus ^= (WDTRPW ^ WDTPW);
    WDTCTL = WDTPW | WDTHOLD;
    // Disable nonmaskable interrupts
#ifdef IE1
    interruptContext->ie1 = IE1;
    IE1 = 0;
    interruptContext->ie2 = IE2;
    IE2 = 0;
#endif
}

static inline void msp430ClearAllInterruptsNosave(void)
{
    DISABLE_INTS();
    WDTCTL = WDTPW | WDTHOLD;
#ifdef IE1
    IE1 = 0;
    IE2 = 0;
#endif
}

static inline void msp430ClearAllInterruptsBeforeReboot(void)
{
    DISABLE_INTS();
#ifdef IE1
    IE1 = 0; IFG1 = 0;
    IE2 = 0; IFG2 = 0;
#endif
    P1IE = 0; P1IFG = 0;
    P2IE = 0; P2IFG = 0;
    // TODO portability: uncomment this code for those models of MSP430
    // that have interrupts on P3, P4, etc.
#if 0
    P3IE = 0; P3IFG = 0;
    P4IE = 0; P4IFG = 0;
    P5IE = 0; P5IFG = 0;
    P6IE = 0; P6IFG = 0;
#endif
}

static inline void msp430RestoreAllInterrupts(Msp430InterruptContext_t *interruptContext)
{
    // Restore interrupts
#ifdef IE1
    IE2 = interruptContext->ie2;
    IE1 = interruptContext->ie1;
#endif
    WDTCTL = interruptContext->watchdogStatus;
    SET_INTERRUPT_STATUS(interruptContext->interruptBit);
}

//
// Turn everything off, device drivers enable what is needed.
//
static inline void msp430InitPins(void)
{
    // All configured for digital I/O
#ifdef P1SEL
    P1SEL = 0;
#endif
#ifdef P2SEL
    P2SEL = 0;
#endif
#ifdef P3SEL
    P3SEL = 0;
#endif
#ifdef P4SEL
    P4SEL = 0;
#endif
#ifdef P5SEL
    P5SEL = 0;
#endif
#ifdef P6SEL
    P6SEL = 0;
#endif

    // All available inputs
#ifdef P1DIR
    P1DIR = 0;
    P1OUT = 0;
#endif
#ifdef P2DIR
//    P2DIR = 0;
//    P2OUT = 0;
    P2OUT = 0x30;
    P2DIR = 0x7b;
#endif
#ifdef P3DIR
//    P3DIR = 0;
//    P3OUT = 0;
    P3OUT = 0x00;
    P3DIR = 0xf1;
#endif
#ifdef P4DIR
//    P4DIR = 0;
//    P4OUT = 0;
    P4OUT = 0xdd;
    P4DIR = 0xfd;
#endif
#ifdef P5DIR
//    P5DIR = 0;
//    P5OUT = 0;
    P5OUT = 0xff;
    P5DIR = 0xff;
#endif
#ifdef P6DIR
//    P6DIR = 0;
//    P6OUT = 0;
    P6OUT = 0x00;
    P6DIR = 0xff;
#endif
#ifdef P7DIR
    P7DIR = 0;
    P7OUT = 0;
#endif
#ifdef P8DIR
    P8DIR = 0;
    P8OUT = 0;
#endif
    P1IE = 0;
    P2IE = 0;
}


#endif
