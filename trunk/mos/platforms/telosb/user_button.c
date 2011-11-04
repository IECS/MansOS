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

#include "user_button.h"

// User button is connected to Port 2.7
// rising edge signals button release. falling: button press

static ButtonFunc_p callback;
ButtonState_t state;

ButtonState_t userButtonGet(void) {
    return state;
}

void userButtonEnable(ButtonFunc_p handler) {
    // start with a released state and wait for falling edge (button press)
    callback = handler;
    state = BUTTON_RELEASED;
    PIN_ENABLE_INT(2, 7);
    PIN_INT_FALLING(2, 7);
}

void userButtonDisable(void) {
    PIN_DISABLE_INT(2, 7);
}

ISR(PORT2, user_button_interrupt)
{
    if (PIN_READ_INT_FLAG(2, 7)) {
        // PIN 7 generated interrupt

        // switch between the int edge: rising/falling
        // and change the cached state
        if (PIN_IS_INT_RISING(2, 7)) {
            state = BUTTON_RELEASED;
            PIN_INT_FALLING(2, 7);
        } else {
            state = BUTTON_PRESSED;
            PIN_INT_RISING(2, 7);
        }
        if (callback) callback();
        PIN_CLEAR_INT_FLAG(2, 7); // do not forget to clear the int flag!
    }
}
