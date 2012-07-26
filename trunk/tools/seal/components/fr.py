#
# MSP430FRxx platform
#

from telosb import *

class CarrierSignalGeneratorAct(SealActuator):
    def __init__(self):
        super(CarrierSignalGeneratorAct, self).__init__("CarrierSignalGenerator")
        self.useFunction.value = "carrierSignalToggle()"
        self.readFunction.value = "0"
        self.onFunction.value = "carrierSignalOn()"
        self.offFunction.value = "carrierSignalOff()"
        self.duration = SealParameter(None, ["10", "20", "50", "100", "200", "500", "1000", "2000"])
        #self.extraConfig = ...
        #self.extraIncludes = ...

ca = CarrierSignalGeneratorAct()
