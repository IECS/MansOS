import sys, string

######################################################

def toMilliseconds(x):
    value = x.value
    if x.suffix is None or x.suffix == '' or x.suffix == "ms":
        pass
    elif x.suffix == "s" or x.suffix == "sec":
        value *= 1000
    else:
        sys.stderr.write("Unknown suffix {0} for time value\n".format(x.suffix))
    return value

######################################################
class BranchCollection(object):
    branches = {}

    def generateCode(self, outputFile):
        for b in self.branches.iteritems():
            self.generateStartCode(b, outputFile)
            self.generateStopCode(b, outputFile)

    def generateStartCode(self, b, outputFile):
        number = b[0]
        useCases = b[1]
        outputFile.write("void branch{0}Start(void)\n".format(number))
        outputFile.write("{\n")
        if number == 0:
            branchCallbackName = ''
        else:
            branchCallbackName = "Branch{0}".format(number)
        for uc in useCases:
            outputFile.write("    {0}{1}Callback(NULL);\n".format(
                    uc.component.getNameCC(), branchCallbackName))
        outputFile.write("}\n")
        outputFile.write("\n")

    def generateStopCode(self, b, outputFile):
        number = b[0]
        useCases = b[1]
        outputFile.write("void branch{0}Stop(void)\n".format(number))
        outputFile.write("{\n")
        if number == 0:
            branchCallbackName = ''
        else:
            branchCallbackName = "Branch{0}".format(number)
        for uc in useCases:
            outputFile.write("    alarmRemove(&{0}{1}Alarm);\n".format(
                    uc.component.getNameCC(), branchCallbackName))
        outputFile.write("}\n")
        outputFile.write("\n")

    # Returns list of numbers [N] of conditions that must be fullfilled to enter this branch
    # * number N > 0: condition N must be true
    # * number N < 0: condition abs(N) must be false
    def getConditions(self, branchUseCases):
        assert len(branchUseCases)
        return branchUseCases[0].conditions
                             
    def getNumBranches(self):
        return len(self.branches)
                    

branchCollection = BranchCollection()

######################################################
class UseCase(object):
    def __init__(self, component, parameters, conditions, branchNumber):
        self.component = component
        self.parameters = dict(parameters)
        self.conditions = conditions
        self.branchNumber = branchNumber
        self.period = None
        if branchNumber != 0:
            self.branchName = "Branch{0}".format(branchNumber)
        else:
            self.branchName = ''
        if branchNumber in branchCollection.branches:
            branchCollection.branches[branchNumber].append(self)
        else:
            branchCollection.branches[branchNumber] = [self]

    def generateConstants(self, outputFile):
        x = self.parameters.get("period")
        if x:
            self.period = toMilliseconds(x)
            ucname = self.component.getNameUC()
            if self.branchNumber != 0:
                ucname += self.branchName.upper()
            outputFile.write(
                "#define {0}_PERIOD    {1}\n".format(
                    ucname, self.period))

    def generateVariables(self, outputFile):
        if self.period:
            outputFile.write(
                "Alarm_t {0}{1}Alarm;\n".format(
                    self.component.getNameCC(), self.branchName))

    def generateCallbacks(self, outputFile):
        ccname = self.component.getNameCC()
        ccname += self.branchName
        ucname = self.component.getNameUC()
        ucname += self.branchName.upper()
        # TODO
        if type(self.component) is Actuator:
            actFunction = "Toggle"
        else:
            actFunction = "Read"

        if self.period:
            outputFile.write('''
void {0}Callback(void *__unused)
{4}
    {1}{2}();
    alarmSchedule(&{0}Alarm, {3}_PERIOD);
{5}
'''
                             .format(ccname, self.component.getNameCC(), actFunction,
                                     ucname, '{', '}'))

    def generateAppMainCode(self, outputFile):
        ccname = self.component.getNameCC()
        ccname += self.branchName
        if self.period:
            outputFile.write("    alarmInit(&{0}Alarm, {0}Callback, NULL);".format(ccname))

######################################################
class Component(object):
    def __init__(self, name):
        self.name = name
        self.useCases = []

    def getNameUC(self):
        return self.name.upper()

    def getNameLC(self):
        return self.name.lower()

    def getNameCC(self):
        assert self.name != ''
        return string.lower(self.name[0]) + self.name[1:]

    def addUseCase(self, parameters, conditions, branchNumber):
        self.useCases.append(UseCase(self, parameters, conditions, branchNumber))

    def generateConstants(self, outputFile):
        for uc in self.useCases:
            uc.generateConstants(outputFile)

    def generateVariables(self, outputFile):
        for uc in self.useCases:
            uc.generateVariables(outputFile)

    def generateCallbacks(self, outputFile):
        for uc in self.useCases:
            uc.generateCallbacks(outputFile)

    def generateAppMainCode(self, outputFile):
        for uc in self.useCases:
            uc.generateAppMainCode(outputFile)

class Actuator(Component):
    def __init__(self, name):
        super(Actuator,self).__init__(name)

class Sensor(Component):
    def __init__(self, name):
        super(Sensor,self).__init__(name)

class Output(Component):
    def __init__(self, name):
        super(Output,self).__init__(name)

######################################################
class ComponentRegister(object):
    architecture = ''
    actuators = {}
    sensors = {}
    outputs = {}
    module = None

    # load all components for this platform from a file
    def load(self, architecture):
        # reset components
        self.actuators = {}
        self.sensors = {}
        self.outputs = {}
        self.architecture = architecture
        # import the module
        sourceFile = architecture + '_comp'
        module = __import__(sourceFile)
        # construct empty components from descriptions
        # (TODO: check for duplicates!)
        for n in module.actuators:
            self.actuators[n.lower()] = Actuator(n)
        for n in module.sensors:
            self.sensors[n.lower()]  = Sensor(n)
        for n in module.outputs:
            self.outputs[n.lower()] = Output(n)

    def findComponent(self, keyword, name):
        o = None
        # print "actuators:", self.actuators
        if keyword == "read":
            o = self.sensors.get(name, None)
        elif keyword == "use":
            o = self.actuators.get(name, None)
        elif keyword == "output":
            o = self.outputs.get(name, None)
        return o

    def hasComponent(self, keyword, name):
        # print "hasComponent? '" + keyword + "' '" + name + "'"
        return bool(self.findComponent(keyword.lower(), name.lower()))

    def useComponent(self, keyword, name, parameters, conditions, branchNumber):
        o = self.findComponent(keyword.lower(), name.lower())
        assert o != None
        o.addUseCase(parameters, conditions, branchNumber)

    def getAllComponents(self):
        return set(self.actuators.values()).union(set(self.sensors.values())).union(set(self.outputs.values()))

######################################################
componentRegister = ComponentRegister()
