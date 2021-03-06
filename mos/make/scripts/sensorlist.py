#!/usr/bin/env python

#
# Copyright (c) 2013 the MansOS team. All rights reserved.
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
# This file semi-automatically generates the list of sensors
# on a specific platform, taking into account configuration options.
# The results is saved in file "sensorlist_<platform>.py"
#

import sys, subprocess, string, re

WMP_SENSOR_LIGHT    = 1
WMP_SENSOR_HUMIDITY = 2
WMP_SENSOR_BATTERY  = 3
WMP_SENSOR_ADC0     = 4
WMP_SENSOR_ADC1     = 5
WMP_SENSOR_ADC2     = 6
WMP_SENSOR_ADC3     = 7
WMP_SENSOR_ADC4     = 8
WMP_SENSOR_ADC5     = 9
WMP_SENSOR_ADC6     = 10
WMP_SENSOR_ADC7     = 11

WMP_OUTPUT_SERIAL   = 1
WMP_OUTPUT_SDCARD   = 2
WMP_OUTPUT_FILE     = 3

platform = "telosb"
configComponents = set()

allSensors = []
allOutputs = []
allLeds = []

def escape(name):
    return name.replace(" ", "")


def generateHeader(outputFile):
    s = """\
# This file was automatically generated by the script "sensorlist.py"

from wmp import *

class Component(object):
    def __init__(self, name, varname, code):
        self.name = name
        self.varname = varname
        self.code = code

class Sensor(Component):
    def __init__(self, name, varname, code):
        super(Sensor, self).__init__(name, varname, code)
        self.period = 0

class Output(Component):
    def __init__(self, name, varname, code):
        super(Output, self).__init__(name, varname, code)
        self.isSelected = False

class Led(Component):
    def __init__(self, name, varname, code):
        super(Led, self).__init__(name, varname, code)
        self.isOn = False

"""
    outputFile.write(s)


def generateSensor(outputFile, name, varname):
    codename = "WMP_SENSOR_" + varname.upper()
    outputFile.write(varname + ' = Sensor("' + name + '", "' + varname + '", ' + codename + ')\n')
    allSensors.append(varname)


def generateOutput(outputFile, name, varname):
    codename = "WMP_OUTPUT_" + varname.upper()
    outputFile.write(varname + ' = Output("' + name + '", "' + varname + '", ' + codename + ')\n')
    allOutputs.append(varname)


def generateLed(outputFile, name, code):
    varname = name[0].lower() + name[1:]
    outputFile.write(varname + ' = Led("' + name + '", "' + varname + '", ' + str(code) + ')\n')
    allLeds.append(varname)


def generateBody(outputFile):
    global allSensors
    global allOutputs

    hasLight = True
    hasHumidity = "HUMIDITY" in configComponents
    hasADC = "ADC" in configComponents
    hasBattery = hasADC

    # MSP430 supports 16 channels, but actually only 0, 1, 2, 6 and 7
    # are externally connected on Tmote Sky
    numAdcChannels = 8
    if platform == "testbed" or platform == "testbed2":
        # use external instead of internal ADC
        numAdcChannels = 8
        hasLight = False
        hasHumidity = False
        hasBattery = False
    elif platform == "atmega":
        # only channels 0..5 available (?)
        numAdcChannels = 6
        hasLight = False
        hasHumidity = False

    if hasLight: generateSensor(outputFile, "Light", "light")
    if hasHumidity: generateSensor(outputFile, "Humidity", "humidity")
    if hasBattery: generateSensor(outputFile, "Battery (internal voltage)", "battery")
    if hasADC:
       for i in range(numAdcChannels):
            generateSensor(outputFile, "ADC channel " + str(i), "ADC" + str(i))

    outputFile.write("\n")
    generateOutput(outputFile, "Serial port", "serial")
    outputFile.write("serial.isSelected = True\n") # enabled by default
    if platform == "testbed"\
            or platform == "testbed2"\
            or platform == "sm3":
        # these platforms can have SD card too
        generateOutput(outputFile, "SD card (without using filesystem)", "SDcard")
        generateOutput(outputFile, "File (on SD card)", "file")

    outputFile.write("\n")
    generateLed(outputFile, "Red", 0)
    generateLed(outputFile, "Green", 1)
    generateLed(outputFile, "Blue", 2)


def generateFooter(outputFile):
    # footer
    outputFile.write("\n")
    outputFile.write("sensors = [" + ", ".join(allSensors) + "]\n")
    outputFile.write("outputs = [" + ", ".join(allOutputs) + "]\n")
    outputFile.write("leds = [" + ", ".join(allLeds) + "]\n")


def main():
    global platform
    global configComponents

    if len(sys.argv) != 3:
        print('Usage: ' + sys.argv[0] + ' <platform> <flags>')
        sys.exit(1)

    platform = sys.argv[1]

    configComponents = set()
    for f in sys.argv[2].split():
        f = f.strip().split("=")[0]
        if f[:6] == "-DUSE_":
            configComponents.add(f[6:])

    with open("sensorlist_" + platform + ".py", "w") as f:
        generateHeader(f)
        generateBody(f)
        generateFooter(f)

    return 0

if __name__ == '__main__':
    main()
