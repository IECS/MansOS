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

//===========================================================================
// Print to a serial port or radio, e.g. for debugging purposes
//
// Usage:
//      PRINTF( str, args...) - printf() style output
//
//      PRINTLN( str ) - print a string, skip to a new line
//
//===========================================================================

#ifndef MANSOS_DPRINT_H
#define MANSOS_DPRINT_H

#include <serial.h>
#include <stdtypes.h>

#ifndef DPRINT_TO_RADIO
// Define this to 1 to send print output to radio.
#define DPRINT_TO_RADIO 0
#endif

#if USE_PRINT

# define PRINT_INIT_DEFAULT(len) \
    static char _print_buf_local[ len +1 ];       \
    _print_buf = _print_buf_local;


# if DPRINT_TO_RADIO
//
// Print to radio
//
#  define PRINT_INIT(len) PRINT_INIT_DEFAULT(len)
#  define PRINT_FUNCTION radioPrint
#  define PRINTF(...) {                           \
    if (COUNT_PARMS(__VA_ARGS__) == 1) {          \
        radioPrintV(__VA_ARGS__);                 \
    } else {                                      \
        debugPrintf(radioPrint, __VA_ARGS__);     \
    } }

# else
//
// Print to serial port
//
#  define PRINT_INIT(len) \
     PRINT_INIT_DEFAULT(len);                                   \
     serialInit(PRINTF_SERIAL_ID, SERIAL_BAUDRATE, 0);          \
     serialEnableTX(PRINTF_SERIAL_ID);                          \
     serial[PRINTF_SERIAL_ID].function = SERIAL_FUNCTION_PRINT
#  define PRINT_FUNCTION serialPrint
#  define PRINTF(...)  {                          \
    if (COUNT_PARMS(__VA_ARGS__) == 1) {          \
        serialPrintV(__VA_ARGS__);                \
    } else {                                      \
        debugPrintf(serialPrint, __VA_ARGS__);    \
    } }

# endif // DPRINT_TO_RADIO

extern char *_print_buf;

// Print text and return 8-bit crc of the result string
#define PRINTF_CRC(...)                           \
    debugPrintfCrc(PRINT_FUNCTION, __VA_ARGS__)   \
// Print text with newline
#define PRINTLN(x) PRINT_FUNCTION(x "\n")

#else // USE_PRINT not defined

# define PRINT_INIT(len)
# define PRINTF(...)
# define PRINTF_CRC(...) 0
# define PRINTLN(x)

#define _print_buf NULL

#endif // USE_PRINT

//
// TPRINTF: print text with timestamp
//
#if PLATFORM_PC
#include <timing.h> // for getTimeMs()
#define TPRINTF(format, ...) debugPrintf(PRINT_FUNCTION, "[%u] " format, getTimeMs(), ##__VA_ARGS__)
#else
#define TPRINTF(...) PRINTF(__VA_ARGS__)
#endif


//
// DEBUG_PRINTF: print only when DEBUG=y
//
#if DEBUG
#define DEBUG_PRINTF(...) PRINTF(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...) PRINTF(__VA_ARGS__)
#endif

//
// The port used for network printing
//
#define DPRINT_PORT  113

// -------------------------------------- functions

void serialPrint(const char* str);
void radioPrint(const char* str);

// internal, neccessary for building correctly
static inline void serialPrintV(const char* str, ...) {
    serialPrint(str);
}
static inline void radioPrintV(const char* str, ...) {
    radioPrint(str);
}

typedef void (*PrintFunction_t)(const char* str);

void debugPrintfFormat(const char* str, ...);
void debugHexdump(void *data, unsigned len);

void printInit(void) WEAK_SYMBOL;

#define debugPrintf(outputFunction, ...)      \
    if (_print_buf) {                         \
        debugPrintfFormat(__VA_ARGS__);       \
        outputFunction(_print_buf);           \
    }

#define debugPrintfCrc(outputFunction, ...) ({         \
        debugPrintf(outputFunction, __VA_ARGS__);      \
        _print_buf ? crc8((uint8_t *) _print_buf, strlen(_print_buf)) : 0; \
    })

#endif
