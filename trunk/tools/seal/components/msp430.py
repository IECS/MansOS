#
# Components common for all MSP430 platforms
#

from component_hierarchy import *

serial = SerialOutput()
radio = RadioOutput()
network = NetworkOutput()
internalFlash = InternalFlashOutput()
localStorage = LocalStorageOutput()

printing = PrintAct()

analogIn = AnalogInputSensor()
digitalIn = DigitalInputSensor()

digitalout = DigitalOutputAct()
beeper = BeeperAct()

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


#
# Tmote-specific components
#
led = LedAct()
redLed = RedLedAct()
blueled = BlueLedAct()
greenled = GreenLedAct()

light = LightSensor()
humidity = HumiditySensor()

externalFlash = ExternalFlashOutput()


#
# MSP430FRxx platform-specific components
#
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
