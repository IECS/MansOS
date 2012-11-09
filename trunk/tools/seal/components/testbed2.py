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
# Components for TelosB platform
#

from msp430 import *


squarewave = SquareWaveSensor()
trianglewave = TriangleWaveSensor()
sawwave = SawtoothWaveSensor()
sinewave = SineWaveSensor()

class ADS8638Sensor(SealSensor):
    def __init__(self):
        super(ADS8638Sensor, self).__init__("ADS8638")
        self.useFunction.value = "ads8638ReadChannel(1)"
        self.readFunction.value = "ads8638ReadChannel(1)"
        self.extraConfig.value = "USE_ADS8638=y"
        self.extraIncludes.value = "#include <ads8638/ads8638.h>"
        self.channel = SealParameter(0, ["0", "1", "2", "3", "4", "5", "6", "7"])
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        # print "calculateParameterValue: ", parameter
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        channel = self.getParameterValue("channel", useCaseParameters)
        if channel is None: channel = 1
        return "ads8638ReadChannel({})".format(channel)

# initialization
ads8638 = ADS8638Sensor()
