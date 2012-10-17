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
# Components for SADmote v03
#

from msp430 import *

class SQ100LightSensor(SealSensor):
    def __init__(self):
        super(SQ100LightSensor, self).__init__("SQ100Light")
        self.useFunction.value = "sq100LightRead()"
        self.readFunction.value = "sq100LightRead()"
        self.extraConfig = SealParameter("""
USE_ADS1115=y
CONST_ADS_INT_PORT=2
CONST_ADS_INT_PIN=0""")
        self.extraIncludes = SealParameter("#include <hil/light.h>")

sq100Light = SQ100LightSensor()


# default light sensor for this platform is ISL29003
light.extraConfig.value="""
# for apds & isl
USE_ISL29003=y
USE_SOFT_I2C=y
CONST_SDA_PORT=2
CONST_SDA_PIN=3
CONST_SCL_PORT=2
CONST_SCL_PIN=4"""
