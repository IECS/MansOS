#
# MSP430-specific components
#

TYPE_ACTUATOR = 1
TYPE_SENSOR = 2
TYPE_OUTPUT = 3

components = []

def ComponentSpecification(object):
    def __init__(self, typeCode, name, parameters):
        self.typeCode = typeCode
        self.name = name
        self.parameters = parameters
        components.append(self)

################################################

commonParameters = [("period", 1000)]

################################################

serialParameters = commonParameters + [("aggregate", False), ("baudrate", 38400)]
packetOutputParameters = commonParameters + [("aggregate", True)]
radioParameters = packetOutputParameters + [("crc", True)]

serial = ComponentSpecification(TYPE_OUTPUT, 'Serial', serialParameters)
radio = ComponentSpecification(TYPE_OUTPUT, 'Radio', radioParameters)
ifflash = ComponentSpecification(TYPE_OUTPUT, 'InternalFlash', packetOutputParameters)
