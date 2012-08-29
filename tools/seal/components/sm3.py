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
USE_SOFT_I2C=y
CONST_SDA_PORT=2
CONST_SDA_PIN=3
CONST_SCL_PORT=2
CONST_SCL_PIN=4"""
