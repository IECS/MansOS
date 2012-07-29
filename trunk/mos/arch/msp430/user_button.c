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

#include "user_button.h"

// On TelosB User button is connected to port 2.7
// Rising edge signals button release, falling: button press.

static ButtonFunc_p callback;
ButtonState_t state;

ButtonState_t userButtonGet(void) {
    return state;
}

void userButtonEnable(ButtonFunc_p handler) {
    // start with a released state and wait for falling edge (button press)
    callback = handler;
    state = BUTTON_RELEASED;
    pinEnableInt(USER_BUTTON_PORT, USER_BUTTON_PIN);
    pinIntFalling(USER_BUTTON_PORT, USER_BUTTON_PIN);
}

void userButtonDisable(void) {
    pinDisableInt(USER_BUTTON_PORT, USER_BUTTON_PIN);
}

XISR(USER_BUTTON_PORT, userButtonInterrupt)
{
    if (pinReadIntFlag(USER_BUTTON_PORT, USER_BUTTON_PIN)) {
        // PIN generated interrupt

        // switch between the int edge: rising/falling
        // and change the cached state
        if (pinIsIntRising(USER_BUTTON_PORT, USER_BUTTON_PIN)) {
            state = BUTTON_RELEASED;
            pinIntFalling(USER_BUTTON_PORT, USER_BUTTON_PIN);
        } else {
            state = BUTTON_PRESSED;
            pinIntRising(USER_BUTTON_PORT, USER_BUTTON_PIN);
        }
        if (callback) callback();
        // do not forget to clear the int flag!
        pinClearIntFlag(USER_BUTTON_PORT, USER_BUTTON_PIN);
    }
}
