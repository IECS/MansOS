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
# Scheduling test platform components
#

from msp430 import *

######################################################
# component definition
class SlowReadSensor1(SealSensor):
    def __init__(self):
        super(SlowReadSensor1, self).__init__("SlowRead1")
        self._cacheable = True
        self.useFunction.value = "slowRead1()"
        self.readFunction.value = "slowRead1()"
        self.preReadFunction.value = "slowPreread1()"
        # update readings no faster than once in 10 seconds
        self._minUpdatePeriod = 10000
        # deterministic upper bound of time-to-read in milliseconds
        self._readTime = 500

class SlowReadSensor2(SealSensor):
    def __init__(self):
        super(SlowReadSensor2, self).__init__("SlowRead2")
        self._cacheable = True
        self.useFunction.value = "slowRead2()"
        self.readFunction.value = "slowRead2()"
        self.preReadFunction.value = "slowPreread2()"
        # update readings no faster than once in 5 seconds
        self._minUpdatePeriod = 5000
        # deterministic upper bound of time-to-read in milliseconds
        self._readTime = 300

class SlowReadSensor3(SealSensor):
    def __init__(self):
        super(SlowReadSensor3, self).__init__("SlowRead3")
        self._cacheable = True
        self.useFunction.value = "slowRead3()"
        self.readFunction.value = "slowRead3()"
        self.preReadFunction.value = "slowPreread3()"
        # update readings no faster than once in 1 seconds (the default)
        # self.minUpdatePeriod = 1000
        # deterministic upper bound of time-to-read in milliseconds
        self._readTime = 100

class FastReadSensor(SealSensor):
    def __init__(self):
        super(FastReadSensor, self).__init__("FastRead")
        self._cacheable = True # actually makes sense
        self.useFunction.value = "fastRead()"
        self.readFunction.value = "fastRead()"
        # do not cache
        self._minUpdatePeriod = 0
        # time-to-read is negligible

######################################################
# component instantiation
s1 = SlowReadSensor1()
s2 = SlowReadSensor2()
s3 = SlowReadSensor3()
f1 = FastReadSensor()
