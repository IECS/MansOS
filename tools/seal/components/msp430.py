#
# Components common for all MSP430 platforms
#

from component_hierarchy import *

serial = SerialOutput()
radio = RadioOutput()
internalFlash = InternalFlashOutput()

printing = PrintAct()

# all kind of pseudo sensors follow

nullSensor = SealSensor("Null") # hack: for virtual use, e.g. min of two sensors

constant = ConstantSensor()
random = RandomSensor()

counter = CounterSensor()
timestamp = TimestampSensor()

squarewave = SquareWaveSensor()
trianglewave = TriangleWaveSensor()
sawwave = SawtoothWaveSensor()
sinewave = SineWaveSensor()
