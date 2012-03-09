# -*- coding: UTF-8 -*-
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

import re
import string
import sealStruct 
import Parameter
import translater
import os
from time import gmtime, strftime
import globals as g
import sealParser_yacc

class ApiCore:
    def __init__(self):
        self.path = os.getcwd()
        # Read settings from file
        if os.path.exists(g.SETTING_FILE) & os.path.isfile(g.SETTING_FILE):
            f = open(g.SETTING_FILE, 'r')
            lines = f.readlines()
            self.__settings = {}
            for x in lines:
                if x != '':
                    key, value = x.split(":")
                    self.__settings[key] = value.strip()
            f.close()
        else:
            # All variables placed here will be saved to configuration file and 
            # reloaded next run time. See setSetting() and getSetting()
            self.__settings = {
                   "activeLanguage" : "ENG"
               }
        # Actuator roles
        self.STATEMENT = 0 # such as use, read, output
        self.CONDITION_START = 1 # such as when
        self.CONDITION_CONTINUE = 2 # such as else
        self.CONDITION_END = 3 # such as end
        self.IGNORE = 4 # such as comments, empty lines etc.

        # All defined platforms
        self.__platforms = ["telosb", "sadmote", "atmega", "waspmote"]
        
        # All actuators and other keywords goes here
        self.__actuators = {
            'use': {
                'objects': ['Led', 'RedLed','GreenLed','BlueLed'],
                'parameters': [
                    Parameter.ParameterDefinition('period', ['','100ms','200ms','500ms','1s','2s','5s']),
                    Parameter.ParameterDefinition('on_at', ['','100ms','200ms','500ms','1s','2s','5s']),
                    Parameter.ParameterDefinition('off_at', ['','100ms','200ms','500ms','1s','2s','5s']),
                    Parameter.ParameterDefinition('blinkTimes', ['','1', '2', '3', '5', '10', '25']),
                    Parameter.ParameterDefinition('blink', None),
                    Parameter.ParameterDefinition('blinkTwice', None),
                    Parameter.ParameterDefinition('turn_on', None),
                    Parameter.ParameterDefinition('turn_off', None),
                ],
                'role': self.STATEMENT
            },
            'read': {
                'objects': ['temperature', 'humidity'],
                'parameters': [
                    Parameter.ParameterDefinition('period', ['','100ms','200ms','500ms','1s','2s','5s'])
                ],
                'role': self.STATEMENT
            },
            'output': {
                'objects': ['serial', 'radio'],
                'parameters': [
                    Parameter.ParameterDefinition('aggregate', None),
                    Parameter.ParameterDefinition('crc', None),
                    Parameter.ParameterDefinition('baudrate', ['','2400', '4800', '9600', '19200', '38400', '57600', '115200']),
                ],
                'role': self.STATEMENT
            },
            'when': {
                'objects': ["System.time < 5s", "System.isDaytime"],
                'parameters': [],
                'role': self.CONDITION_START
            },
            'else': {
                'objects': [],
                'parameters': [],
                'role': self.CONDITION_CONTINUE
            },
            'end': {
                'objects': [],
                'parameters': [],
                'role': self.CONDITION_END
            }
        }
        # Compile regex for finding actuators
        self.__reActuators = re.compile(string.join(self.__actuators.keys(), '|'), re.I)
        
        self.sealParser = sealParser_yacc.SealParser()
        self.seal = sealStruct.Seal(self)
        
        self.translater = translater.Translater(self)
        
    # Return regex for finding actuators
    def getReActuators(self):
        return self.__reActuators
    
    def getRole(self, actuator):
        return self.__actuators[actuator]['role']
    
    def getStatementType(self, line):
        
        actuator = line.split(None, 1)
        if actuator != []:
            if actuator[0] in self.__actuators:
                return self.__actuators[actuator[0]]['role']
        return self.IGNORE
    
    def getActuatorInfo(self, actuator):
        if actuator in self.__actuators:
            return self.__actuators[actuator]
        # Return empty object
        return {
                'objects': [],
                'parameters': [],
                'role': self.IGNORE
                }
    
    # Get all actuators, who have role == self.STATEMENT
    def getAllStatementActuators(self):
        result = []
        for x in self.__actuators.keys():
            if self.__actuators[x]['role'] == self.STATEMENT:
                result.append(x)
        return result
    
    def getDefaultConditions(self):
        return self.__actuators['when']['objects']
    
    def getPlatforms(self):
        return self.__platforms
    
    def getSetting(self, setting):
        if setting in self.__settings:
            return self.__settings[setting]
        return ''
    
    def setSetting(self, name, value):
        self.__settings[name] = value
        # Make sure, that settings are saved on unexpected exit
        # XXX: is this correct???
        self.saveSettings()
        
    def saveSettings(self):
        os.chdir(self.path)
        f = open(g.SETTING_FILE, 'w')
        for key in self.__settings:
            f.write(key + ":" + self.__settings[key] + '\n')
        f.close()
    
    def logMsg(self, msgType, msgText):
        if msgType <= g.LOG:
            # Generate message
            dbgTime = str(strftime("%H:%M:%S %d.%m.%Y", gmtime())) + ": " 
            dbgMsg = g.LOG_TEXTS[msgType] + " - " + str(msgText) + '\n'
            if g.LOG_TO_CONSOLE:
                print dbgMsg
            if g.LOG_TO_FILE:
                path = os.getcwd()
                os.chdir(self.path)
                # File should be openned @startup and closed @shutdown, performace!
                f = open(g.LOG_FILE_NAME, "a")
                f.write(dbgTime + dbgMsg)
                f.close()
                os.chdir(path)
