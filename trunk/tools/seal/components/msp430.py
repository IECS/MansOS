#
# Components common for all MSP430 platforms
#

from component_hierarchy import *

serial = SerialOutput()
radio = RadioOutput()
internalFlash = InternalFlashOutput()

printing = PrintAct()

constant = ConstantSensor()
random = RandomSensor()
