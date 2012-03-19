# -*- coding: utf-8 -*-
#
# Copyright (c) 2008-2012 the MansOS team. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import parameter
import globals as g

class SealSyntax():
    def __init__(self):
        # All defined platforms
        self.platforms = ["telosb", "sadmote", "atmega", "waspmote"]

        # All actuators and other keywords goes here
        self.actuators = {
            'use': {
                'objects': ['Led', 'RedLed', 'GreenLed', 'BlueLed'],
                'parameters': [
                    parameter.ParameterDefinition('period', ['', '100ms', '200ms', '500ms', '1s', '2s', '5s']),
                    parameter.ParameterDefinition('on_at', ['', '100ms', '200ms', '500ms', '1s', '2s', '5s']),
                    parameter.ParameterDefinition('off_at', ['', '100ms', '200ms', '500ms', '1s', '2s', '5s']),
                    parameter.ParameterDefinition('blinkTimes', ['', '1', '2', '3', '5', '10', '25']),
                    parameter.ParameterDefinition('blink', None),
                    parameter.ParameterDefinition('blinkTwice', None),
                    parameter.ParameterDefinition('turn_on', None),
                    parameter.ParameterDefinition('turn_off', None),
                ],
                'role': g.STATEMENT
            },
            'read': {
                'objects': ['temperature', 'humidity'],
                'parameters': [
                    parameter.ParameterDefinition('period', ['', '100ms', '200ms', '500ms', '1s', '2s', '5s'])
                ],
                'role': g.STATEMENT
            },
            'output': {
                'objects': ['serial', 'radio'],
                'parameters': [
                    parameter.ParameterDefinition('aggregate', None),
                    parameter.ParameterDefinition('crc', None),
                    parameter.ParameterDefinition('baudrate', ['', '2400', '4800', '9600', '19200', '38400', '57600', '115200']),
                ],
                'role': g.STATEMENT
            },
            'when': {
                'objects': ["System.time < 5s", "System.isDaytime"],
                'parameters': [],
                'role': g.CONDITION
            },
            'else': {
                'objects': [],
                'parameters': [],
                'role': g.CONDITION
            },
            'end': {
                'objects': [],
                'parameters': [],
                'role': g.END
            }
        }
