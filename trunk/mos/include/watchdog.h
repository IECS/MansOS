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

#ifndef MANSOS_WATCHDOG_H
#define MANSOS_WATCHDOG_H

/// \file
/// Hardware watchdog API
///

//===========================================================
// Procedures
//===========================================================

///
/// Stop the watchdog timer
///
extern inline void watchdogStop(void);

//! Watchdog timer expiry
typedef enum {
    WATCHDOG_EXPIRE_1000MS,
    WATCHDOG_EXPIRE_250MS,
    WATCHDOG_EXPIRE_16MS,
    WATCHDOG_EXPIRE_2MS, // 1.9 actually
} WatchdogMode_e;

///
/// Start the watchdog timer. Resets system on expiration.
///
/// The timer is restarted every time this function is called.
///
extern inline void watchdogStart(WatchdogMode_e mode);

///
/// Reboot the mote by using watchdog interface.
///
/// Resets the mote configuration before rebooting
/// (useful for staring from a clean, known state)
///
void watchdogReboot(void);

///
/// Similar to watchdogReboot(), but does not attempt to change
/// any configuration before rebooting
///
extern inline void watchdogRebootSimple(void);

// implementation
#include <watchdog_hal.h>

#endif
