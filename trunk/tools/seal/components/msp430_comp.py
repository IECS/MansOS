#
# MSP430-specific components
#

from component_def import *

commonParameters = [("period", 1000)]

serialParameters = commonParameters + [("aggregate", False), ("baudrate", 38400)]
packetOutputParameters = commonParameters + [("aggregate", True)]
radioParameters = packetOutputParameters + [("crc", True)]

serial = ComponentSpecification(TYPE_OUTPUT, 'Serial', serialParameters)
radio = ComponentSpecification(TYPE_OUTPUT, 'Radio', radioParameters)
ifflash = ComponentSpecification(TYPE_OUTPUT, 'InternalFlash', packetOutputParameters)
