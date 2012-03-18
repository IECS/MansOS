class UseCase(object):
    condition = None
    parameters = []
    def __init__(self, condition, parameters):
        self.condition = condition
        self.parameters = parameters

class Component(object):
    name = ""
    useCases = []

    def __init__(self, name):
        self.name = name

    def addUseCase(self, condition, parameters):
        for p in parameters:
            addParameter(self, p) # XXX
        useCases.append(UseCase(condition, parameters))
        return None # TODO: error string, if any

    def addParameter(self, parameter):
        pass # TODO

    def clear(self):
        self.useCases = []

class Actuator(Component):
    def __init__(self, name):
        super(Actuator,self).__init__(name)

class Sensor(Component):
    def __init__(self, name):
        super(Sensor,self).__init__(name)

class Output(Component):
    def __init__(self, name):
        super(Output,self).__init__(name)

class ComponentRegister(object):
    actuators = {}
    sensors = {}
    outputs = {}
    module = None

    def __init__(self):
        pass

    # load all components for this platform from a file
    def load(self, architecture):
        sourceFile = architecture + '_comp'
        # import the module
        module = __import__(sourceFile)
        # construct the objects
        # (TODO: check for duplicates!)
        for n in module.actuators:
            self.actuators[n.lower()] = Actuator(n)
        for n in module.sensors:
            self.sensors[n.lower()]  = Sensor(n)
        for n in module.outputs:
            self.outputs[n.lower()] = Output(n)

    def findComponent(self, type, name):
        o = None
        if type == "read":
            o = self.sensors.get(name, None)
        elif type == "use":
            o = self.actuators.get(name, None)
        elif type == "output":
            o = self.outputs.get(name, None)
        return o

    def useComponent(self, type, name, parameters, condition):
        o = self.findComponent(type, name)
        if o == None:
            errorStr = "component %s not found" % name
            return errorStr
        return o.addUseCase(condition, parameters)

class Value(object):
    value = None
    suffix = None
    def __init__(self):
        pass
    def getCode():
        s = "{0}".format(self.value)
        if not (suffix is None):
            s += suffix
        return s

class Condition(object):
    left, right, op = None, None, None
    def getCode():
        if op == '&':
            return left.getCode() + " and " + right.getCode()
        if op == '|':
            return left.getCode() + " or " + right.getCode()
        if op == '!':
            return "not (" + right.getCode() + ")"
        return right


#################################################
componentRegister = ComponentRegister()
