#
# MSP430-specific components
#

from component_def import *

commonParameters = [("period", 1000), ("pattern", None)]

serialParameters = commonParameters + [("sendFunction", "serialPacketPrint()"), ("aggregate", False), ("baudrate", 38400)]
radioParameters = commonParameters + [("aggregate", True),
     ("sendFunction", "radioSend(&radioPacket, sizeof(radioPacket))"),("crc", True)]
iflashParameters = commonParameters + [("aggregate", True),
     ("sendFunction", "flashWrite(&internalFlashPacket, sizeof(internalFlashPacket))")]

serial = ComponentSpecification(TYPE_OUTPUT, 'Serial', serialParameters)
radio = ComponentSpecification(TYPE_OUTPUT, 'Radio', radioParameters)
ifflash = ComponentSpecification(TYPE_OUTPUT, 'InternalFlash', iflashParameters)

# allow up to 10 arguments
printParameters = commonParameters + [("format", None), ("arg0", None), ("arg1", None), 
                   ("arg2", None), ("arg3", None), ("arg4", None), ("arg5", None),
                   ("arg6", None), ("arg7", None), ("arg8", None), ("arg9", None)]
printComponent = ComponentSpecification(TYPE_ACTUATOR, 'Print', printParameters)
