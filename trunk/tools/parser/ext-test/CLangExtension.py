from testarch import *

class CLangExtSensor(SealSensor):
    def __init__(self):
        super(CLangExtSensor, self).__init__("CLangExtSensor")
        self.useFunction.value = "clangExtSensorRead()"
        self.readFunction.value = self.useFunction.value
        self.extraIncludes.value = '#include "ext.h"'

clangext = CLangExtSensor()
