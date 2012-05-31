#
# Scheduling test platform components
#

from telosb import *

######################################################
# component definition
class SlowReadSensor1(SealSensor):
    def __init__(self):
        super(SlowReadSensor1, self).__init__("SlowRead1")
        self.cacheable = True
        self.useFunction.value = "slowRead1()"
        self.readFunction.value = "slowRead1()"
        self.prereadFunction.value = "slowPreread1()"
        # deterministic maximal read time in milliseconds
        self.readTime = 500
        # update readings no faster than once in 10 seconds
        self.cacheTime = 10000

class SlowReadSensor2(SealSensor):
    def __init__(self):
        super(SlowReadSensor2, self).__init__("SlowRead2")
        self.cacheable = True
        self.useFunction.value = "slowRead2()"
        self.readFunction.value = "slowRead2()"
        self.prereadFunction.value = "slowPreread2()"
        # deterministic maximal read time in milliseconds
        self.readTime = 300
        # update readings no faster than once in 5 seconds
        self.cacheTime = 5000

class SlowReadSensor3(SealSensor):
    def __init__(self):
        super(SlowReadSensor3, self).__init__("SlowRead3")
        self.cacheable = True
        self.useFunction.value = "slowRead3()"
        self.readFunction.value = "slowRead3()"
        self.prereadFunction.value = "slowPreread3()"
        # deterministic maximal read time in milliseconds
        self.readTime = 100
        # update readings no faster than once in 1 seconds (the default)
        # self.cacheTime = 1000

class FastReadSensor(SealSensor):
    def __init__(self):
        super(FastReadSensor, self).__init__("FastRead")
        self.cacheable = True # actually make sense
        self.useFunction.value = "fastRead()"
        self.readFunction.value = "fastRead()"
        # do not cache
        self.cacheTime = 0

######################################################
# component instantiation
s1 = SlowReadSensor1()
s2 = SlowReadSensor2()
s3 = SlowReadSensor3()
f1 = FastReadSensor()
