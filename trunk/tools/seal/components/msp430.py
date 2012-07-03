#
# Components common for all MSP430 platforms
#

from component_hierarchy import *

serial = SerialOutput()
radio = RadioOutput()
internalFlash = InternalFlashOutput()

printing = PrintAct()

analogIn = AnalogInputSensor()
digitalIn = DigitalInputSensor()

digitalout = DigitalOutputAct()
beeper = BeeperAct()

# all kind of pseudo sensors follow

nullSensor = SealSensor("Null") # hack: for virtual use, e.g. min of two sensors

command = CommandSensor()

constant = ConstantSensor()
random = RandomSensor()

counter = CounterSensor()
systime = SystemTimeSensor()

squarewave = SquareWaveSensor()
trianglewave = TriangleWaveSensor()
sawwave = SawtoothWaveSensor()
sinewave = SineWaveSensor()

