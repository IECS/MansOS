#
# Copyright (c) 2012 Atis Elsts
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
# Components common for all MSP430 platforms
#

from component_hierarchy import *

serial = SerialOutput()
internalFlash = InternalFlashOutput()

printing = PrintAct()

analogIn = AnalogInputSensor()
digitalIn = DigitalInputSensor()

digitalout = DigitalOutputAct()
analogoput = AnalogOutputAct()

# all kind of pseudo sensors follow

nullSensor = SealSensor("Null") # hack: for virtual use, e.g. min of two sensors

command = CommandSensor()

constant = ConstantSensor()
counter = CounterSensor()
random = RandomSensor()

timecounter = TimeCounterSensor()
systime = SystemTimeSensor()
uptime = UptimeSensor()

squarewave = SquareWaveSensor()
trianglewave = TriangleWaveSensor()
sawwave = SawtoothWaveSensor()
sinewave = SineWaveSensor()

variables = VariableSensor()
constants = ConstantsSensor()


#
# Tmote-specific components
#
led = LedAct()
yellowLed = YellowLedAct()
