TYPE_ACTUATOR = 1
TYPE_SENSOR = 2
TYPE_OUTPUT = 3

components = []

class ComponentSpecification(object):
    def __init__ (self, typeCode, name, parameters):
        self.typeCode = typeCode
        self.name = name
        self.parameters = parameters
        components.append(self)
