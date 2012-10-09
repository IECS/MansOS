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

//===========================================================================
// Print to a serial port or radio, e.g. for debugging purposes
//
// Usage:
//      PRINT_INIT( len )  - initialize the printing service, 
//              use buffer to accomodate up to 'len' characters, 
//              You can use 0 if no PRINTF is used
//
//      PRINT( str ) - print a string, no newline
//      PRINTLN( str ) - print a string, skip to a new line
//
//      PRINTF( str, args...) - printf() style output
//
//===========================================================================

#ifndef MANSOS_DPRINT_H
#define MANSOS_DPRINT_H

#include <serial.h>
#include <kernel/stdtypes.h>

//#define DPRINT_TO_RADIO 1

#if USE_PRINT

# define PRINT_INIT_DEF(len) \
    static char _print_buf_local[ len +1 ];       \
    _print_buf = _print_buf_local;

# ifdef PRINT
// code for printing using custom PRINT function provided by the user
#  define PRINT_INIT_NEW(len) PRINT_INIT_DEF(len);
extern void PRINT(const char *format, ...);
#  ifndef PRINTF
#   define PRINTF PRINT
#  endif
// PRINT and PRINTF are defined in user code
# elif defined DPRINT_TO_RADIO
// code for printing to radio
#  define PRINT_INIT_NEW(len) PRINT_INIT_DEF(len);
#  define PRINT radioPrint
#  define PRINTF(...) debugPrintf(PRINT, __VA_ARGS__);
# else
// code for printing to serial port
#  define PRINT_INIT_NEW(len) \
     PRINT_INIT_DEF(len);                                       \
     serialInit(PRINTF_SERIAL_ID, SERIAL_PORT_BAUDRATE, 0);     \
     serialEnableTX(PRINTF_SERIAL_ID);                          \
     serial[PRINTF_SERIAL_ID].function = SERIAL_FUNCTION_PRINT

#  define PRINT serialPrint
#  define PRINTF(...) debugPrintf(PRINT, __VA_ARGS__);
# endif // DPRINT_TO_RADIO

#else // USE_PRINT not defined

# define PRINT_INIT_DEF(len)
# define PRINT_INIT_NEW(len)
# define PRINTF(...)
# define PRINT(...)

#endif // USE_PRINT


#define PRINTLN(x) PRINT(x "\n")

#if PLATFORM_PC
#include <timers.h> // for getRealTime()
#define TPRINTF(format, ...) debugPrintf(PRINT, "[%u] " format, getRealTime(), ##__VA_ARGS__)
#else
#define TPRINTF(...) PRINTF(__VA_ARGS__)
#endif

// the port used for network printing
#define DPRINT_PORT  113

// -------------------------------------------------
// old version, kept for backwards compatibility, does nothing
#define PRINT_INIT(len)

// -------------------------------------- functions

void serialPrint(const char* str);
void radioPrint(const char* str);

typedef void (* PrintFunction_t)(const char* str);

void debugPrintf(PrintFunction_t outputFunction, const char* str, ...);
void debugHexdump(void *data, unsigned len);

void printInit(void) WEAK_SYMBOL;

#endif
