import sys, string
from structures import *

######################################################

def isConstantField(f):
    return False # TODO

def generateSerialFunctions(intSizes, outputFile):
    for size in intSizes:
        outputFile.write("static inline void serialPrintU{0}(const char *name, uint{0}_t value)\n".format(
            size * 8))
        outputFile.write("{\n")
        outputFile.write('    PRINTF("%s=%u\\n", name, value);' + "\n")
        outputFile.write("}\n")


######################################################
class BranchCollection(object):
    def __init__(self):
        self.branches = {0 : []} # default branch (code 0) is always present

    def generateCode(self, outputFile):
        for b in self.branches.iteritems():
            self.generateStartCode(b, outputFile)
            self.generateStopCode(b, outputFile)

    def generateStartCode(self, b, outputFile):
        number = b[0]
        useCases = b[1]
        outputFile.write("void branch{0}Start(void)\n".format(number))
        outputFile.write("{\n")
        for uc in useCases:
            uc.generateBranchEnterCode(outputFile)
        outputFile.write("}\n")
        outputFile.write("\n")

    def generateStopCode(self, b, outputFile):
        number = b[0]
        useCases = b[1]
        outputFile.write("void branch{0}Stop(void)\n".format(number))
        outputFile.write("{\n")
        for uc in useCases:
            uc.generateBranchExitCode(outputFile)
        outputFile.write("}\n")
        outputFile.write("\n")

    # Returns list of numbers [N] of conditions that must be fullfilled to enter this branch
    # * number N > 0: condition N must be true
    # * number N < 0: condition abs(N) must be false
    def getConditions(self, branchNumber):
        branchUseCases = self.branches[branchNumber]
        assert len(branchUseCases)
        assert len(branchUseCases[0].conditions)
        return branchUseCases[0].conditions

    def getNumBranches(self):
        return len(self.branches)


######################################################
class UseCase(object):
    def __init__(self, component, parameters, conditions, branchNumber, numInBranch):
        self.component = component
        self.parameters = {}
        # use component's parameters as defaults
        for p in component.parameters:
            self.parameters[p] = Value(component.parameters[p])

        # add user's parameters
        for p in parameters:
            if p[0] not in component.parameters:
                userError("Parameter '{0}' not known for component {1}\n".format(p[0], component.name))
                continue
            # update parameters with user's value, if given. If no value given, only name, treat it as 'True',
            # because value 'None' means that the parameter is supported, but not specified by user
            if p[1] is not None:
                self.parameters[p[0]] = p[1]
            else:
                self.parameters[p[0]] = Value(True)
        self.conditions = list(conditions)
        self.branchNumber = branchNumber
        if numInBranch == 0:
            self.numInBranch = ''
        else:
            self.numInBranch = "{0}".format(numInBranch)
        #print "add use case, conditions =", self.conditions
        #print "  branchNumber=", self.branchNumber

        # TODO: automate this using reflection!
        p = self.parameters.get("period")
        if p:
            self.period = toMilliseconds(p)
        else:
            self.period = None
        p = self.parameters.get("pattern")
        if p:
            self.pattern = p.asString()
        else:
            self.pattern = None
        p = self.parameters.get("once")
        if p:
            self.once = bool(p.value)
        else:
            self.once = None
        for x in ['average', 'stdev', 'filter']:
            p = self.parameters.get(x)
            if p and p.value != '':
                    self.__setattr__(x, p.asString())
            else:
                self.__setattr__(x, None)

        if (self.period and self.pattern) or (self.period and self.once) or (self.pattern and self.once):
            if self.period and self.pattern:
                userError("Both 'period' and 'pattern' specified for component '{0}' use case\n".format(component.name))
            else:
                userError("Both 'once' and 'period' or 'pattern' specified for component '{0}' use case\n".format(component.name))
        self.generateAlarm = self.once or self.pattern or self.period

        if branchNumber != 0:
            self.branchName = "Branch{0}".format(branchNumber)
        else:
            self.branchName = ''
        if branchNumber in branchCollection.branches:
            branchCollection.branches[branchNumber].append(self)
        else:
            branchCollection.branches[branchNumber] = [self]

        #print "after conditions =", branchCollection.branches[branchNumber][0][0].conditions
        #print "after conditions[1] =", branchCollection.branches[branchNumber][0][1]

    def generateConstants(self, outputFile):
        ucname = self.component.getNameUC()
        if self.period:
            if self.branchNumber != 0:
                ucname += self.branchName.upper()
            outputFile.write(
                "#define {0}_PERIOD{1}    {2}\n".format(
                    ucname, self.numInBranch, self.period))

    def generateVariables(self, outputFile):
        global processStructInits

        if self.generateAlarm:
            outputFile.write(
                "Alarm_t {0}{1}Alarm{2};\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))

        if self.filter:
            outputFile.write(
                "Filter_t {}{}Filter{};\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))
            processStructInits.append("    {}{}Filter{} = filterInit({});\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch,
                    self.filter))

        if self.average:
            outputFile.write(
                "Average_t {}{}Average{};\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))
            processStructInits.append("    {}{}Average{} = avgInit({});\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch,
                    self.average))

        if self.stdev:
            outputFile.write(
                "Stdev_t {}{}Stdev{};\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))
            processStructInits.append("    {}{}Stdev{} = stdevInit({});\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch,
                    self.stdev))

    def generateCallbacks(self, outputFile, outputs):
        ccname = self.component.getNameCC()
        ccname += self.branchName
        ucname = self.component.getNameUC()
        ucname += self.branchName.upper()

        useFunction = self.component.getDependentParameterValue("useFunction", self.parameters)

        # generate read function for cacheable sensors
        if self.generateAlarm and type(self.component) is Sensor and self.component.cacheNeeded:
            if not self.component.readFunctionOk:
                self.component.readFunctionOk = True
                outputFile.write("uint{0}_t read{1}(void)\n".format(self.component.getDataSize() * 8,
                                                                    self.component.name))
                outputFile.write("{\n")
                outputFile.write("    return {};\n".format(useFunction))
                outputFile.write("}\n\n")
            useFunction = self.component.getReadFunction()

        if self.generateAlarm:
            outputFile.write("void {0}{1}Callback(void *__unused)\n".format(ccname, self.numInBranch))
            outputFile.write("{\n")
            if type(self.component) is Actuator:
                if self.component.name.lower() == "print":
                    # special handling for print statements
                    formatString = self.getParameterValue("format", "")
                    outputFile.write("    PRINTF(\"{0}\"".format(formatString.strip('"')))
                    for i in range(100):
                        param = self.getParameterValue("arg{0}".format(i))
                        if param:
                            outputFile.write(", {0}".format(param))
                    outputFile.write(");\n")
                else:
                    outputFile.write("    {0};\n".format(useFunction))
            elif type(self.component) is Sensor:
                intTypeName = "uint{0}_t".format(self.component.getDataSize() * 8)
                outputFile.write("    {0} {1}Value = {2};\n".format(
                        intTypeName, self.component.getNameCC(), useFunction))
                if self.filter:
                    filterName = "{}{}Filter{}".format(self.component.getNameCC(),
                                self.branchName, self.numInBranch)
                    outputFile.write("    if(addFilter(&{}, &{}Value)) ".format(
                         filterName, self.component.getNameCC()))
                    outputFile.write("{\n")
                    if self.average:
                        outputFile.write("        addAverage(&{}{}Average{}, &getValue(&{}));\n".format(
                            self.component.getNameCC(), self.branchName,
                            self.numInBranch, filterName))
                    if self.stdev:
                        outputFile.write("        addStdev(&{}{}Stdev{}, &getValue(&{}));\n".format(
                            self.component.getNameCC(), self.branchName,
                            self.numInBranch, filterName))
                    self.checkDependencies(self.component.getNameCC(), 2, outputFile)
                    for o in outputs:
                        o.generateCallbackCode(self.component, outputFile)
                    outputFile.write("    }\n")
                else:
                    if self.average:
                        outputFile.write("    addAverage(&{}{}Average{}, &{}Value);\n".format(
                            self.component.getNameCC(), self.branchName,
                            self.numInBranch, self.component.getNameCC()))
                    if self.stdev:
                        outputFile.write("    addStdev(&{}{}Stdev{}, &{}Value);\n".format(
                            self.component.getNameCC(), self.branchName,
                            self.numInBranch, self.component.getNameCC()))
                    self.checkDependencies(self.component.getNameCC(), 1, outputFile)
                    for o in outputs:
                        o.generateCallbackCode(self.component, outputFile)

            if self.period:
                outputFile.write("    alarmSchedule(&{0}Alarm{1}, {2}_PERIOD{1});\n".format(
                        ccname, self.numInBranch, ucname))
            elif self.pattern:
                outputFile.write("    alarmSchedule(&{0}Alarm{1}, __pattern_{2}[__pattern_{2}Cursor]);\n".format(
                        ccname, self.numInBranch, self.pattern))
                outputFile.write("    __pattern_{0}Cursor++;\n".format(self.pattern))
                outputFile.write("    __pattern_{0}Cursor %= sizeof(__pattern_{0}) / sizeof(*__pattern_{0});\n".format(
                        self.pattern))
            else: # once
                pass
            outputFile.write("}\n")

    # Checks for dependencies for given name
    def checkDependencies(self, name, indent, outputFile):
        for x in componentRegister.getDependentProcess(name):
            x[1].generateCallbackCode(outputFile, name, indent, self.checkDependencies)

    def generateAppMainCode(self, outputFile):
        ccname = self.component.getNameCC()
        ccname += self.branchName
        if self.generateAlarm:
            outputFile.write("    alarmInit(&{0}Alarm{1}, {0}{1}Callback, NULL);\n".format(
                    ccname, self.numInBranch))

    def generateBranchEnterCode(self, outputFile):
        if type(self.component) is not Output and self.generateAlarm:
            outputFile.write("    {0}{1}{2}Callback(NULL);\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))

    def generateBranchExitCode(self, outputFile):
        if type(self.component) is not Output and self.generateAlarm:
            outputFile.write("    alarmRemove(&{0}{1}Alarm{2});\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))
            if self.pattern:
                # reset cursor position (TODO XXX: really?)
                outputFile.write("    __pattern_{0}Cursor = 0;\n".format(self.pattern))

    def getParameterValue(self, parameter, defaultValue = None):
        if parameter in self.parameters:
            return self.parameters[parameter].getCodeForGenerator(componentRegister)
        return self.component.getParameterValue(parameter, defaultValue)


######################################################
class Component(object):
    def __init__(self, specification):
        self.name = specification.name
        # create dictionary for parameters
        self.parameters = {}
        for p in dir(specification):
            if type(specification.__getattribute__(p)) is componentRegister.module.SealParameter:
                self.parameters[p] = specification.__getattribute__(p).value
        self.useCases = []
        self.markedAsUsed = False
        # save specification (needed for dependent parameters)
        self.specification = specification

    def markAsUsed(self):
        self.markedAsUsed = True

    def isUsed(self):
        return self.markedAsUsed or bool(len(self.useCases))

    def getNameUC(self):
        return self.name.upper()

    def getNameLC(self):
        return self.name.lower()

    def getNameCC(self):
        assert self.name != ''
        return string.lower(self.name[0]) + self.name[1:]

    def addUseCase(self, parameters, conditions, branchNumber):
        numInBranch = 0
        for uc in self.useCases:
            if uc.branchNumber == branchNumber:
                numInBranch += 1
        finalParameters = {}
        for p in parameters:
            if p[0].lower() == "parameters":
                #print "p[1] = ", p[1].asString()
                #print "componentRegister = ", componentRegister.defines.keys()
                d = componentRegister.defines.get(p[1].asString())
                if d is None:
                    userError("No define with name '{0}' is present (for component '{1}')\n".format(
                            p[1].asString(), self.name))
                else:
                    for pd in d.parameters:
                        if pd[0] in finalParameters:
                            userError("Parameter '{0}' already specified for component '{1}'\n".format(pd[0], self.name))
                        else:
                            finalParameters[pd[0]] = pd[1]
            else:
                if p[0] in finalParameters:
                    userError("Parameter '{0}' already specified for component '{1}'\n".format(p[0], self.name))
                else:
                    finalParameters[p[0]] = p[1]
        self.useCases.append(UseCase(self, list(finalParameters.iteritems()), conditions, branchNumber, numInBranch))

    # XXX: replacement for "getParamValue"
    def getSpecialValue(self, parameter):
        if parameter in self.parameters:
            value = self.parameters[parameter]
            if value is not None: return value
        return None

    def generateIncludes(self, outputFile):
        if self.isUsed():
            includes = self.getSpecialValue("extraIncludes")
            if includes is not None:
                # print "includes=", includes
                outputFile.write("{0}\n".format(includes))

    def generateConstants(self, outputFile):
        for uc in self.useCases:
            uc.generateConstants(outputFile)

    def generateVariables(self, outputFile):
        for uc in self.useCases:
            uc.generateVariables(outputFile)

    def generateCallbacks(self, outputFile, outputs):
        if type(self) is Output: return
        for uc in self.useCases:
            uc.generateCallbacks(outputFile, outputs)

    def generateAppMainCode(self, outputFile):
        for uc in self.useCases:
            uc.generateAppMainCode(outputFile)

    def generateConfig(self, outputFile):
        if self.isUsed():
            config = self.getSpecialValue("extraConfig")
            if config is not None:
                outputFile.write("{0}\n".format(config))

    def getParameterValue(self, parameter, defaultValue = None):
        if parameter in self.parameters:
            value = self.parameters[parameter]
            if value is not None: return value
        return defaultValue

    def getDependentParameterValue(self, parameter, useCaseParameters):
        if parameter not in self.parameters:
            return None
        return self.specification.calculateParameterValue(parameter, useCaseParameters)

######################################################
class Actuator(Component):
    def __init__(self, specification):
        super(Actuator, self).__init__(specification)

######################################################
class Sensor(Component):
    def __init__(self, specification):
        super(Sensor, self).__init__(specification)
        if self.specification.minUpdatePeriod is None:
            self.minUpdatePeriod = 1000 # default value
        else:
            self.minUpdatePeriod = self.specification.minUpdatePeriod
        self.cacheNeeded = False
        self.cacheNumber = 0
        self.readFunctionOk = False

    def getDataSize(self):
        size = self.getParameterValue("dataSize")
        if size is None:
            size = 2 # TODO: issue warning
        return size

    def getNoValue(self):
        return "0x" + "ff" * self.getDataSize()

    def generateConstants(self, outputFile):
        super(Sensor, self).generateConstants(outputFile)
        if self.isUsed():
            outputFile.write("#define {0}_NO_VALUE    {1}\n".format(self.getNameUC(), self.getNoValue()))

    def isCacheNeeded(self, numCacheableSensors):
        for uc in self.useCases:
            if uc.period is not None and uc.period < self.minUpdatePeriod:
                self.cacheNeeded = True
                self.cacheNumber = numCacheableSensors
                break

        return self.cacheNeeded

    def getReadFunction(self):
        if self.cacheNeeded:
            dataFormat = str(self.getDataSize() * 8)
            return "cacheReadSensorU{0}({1}, &read{2}, {3})".format(
                dataFormat, self.cacheNumber, self.name, self.minUpdatePeriod)
            # return "read".format(self.getNameCC())
        return c.getParameterValue("readFunction", None)

######################################################
class Output(Component):
    def __init__(self, specification):
        super(Output, self).__init__(specification)
        self.isAggregateCached = None
        self.usedFields = []

    def getParameterValue(self, parameter, defaultValue = None):
        if len(self.useCases) != 0 and parameter in self.useCases[0].parameters:
            return self.useCases[0].parameters[parameter]
        return super(Output, self).getParameterValue(parameter, defaultValue)

    def isAggregate(self):
        if self.isAggregateCached is None:
            self.isAggregateCached = self.getParameterValue("aggregate", False)
        return self.isAggregateCached

    def generateVariables(self, outputFile):
        if self.isAggregate() and len(self.usedFields) != 0:
            outputFile.write("{0}Packet_t {1}Packet;\n".format(self.name, self.getNameCC()))
            outputFile.write("uint_t {0}PacketNumFieldsFull;\n".format(self.getNameCC()))

    def cachePacketType(self, packetFields):
        if not self.isAggregate(): return

        self.usedFields = packetFields

        # TODO: use only explicitly specified fields

        # TODO: use constant fields

        if len(self.usedFields) == 0:
            userError("{0}Packet has no fields\n".format(name))
            return

    def generateConstants(self, outputFile):
        super(Output, self).generateConstants(outputFile)
        if len(self.usedFields):
            outputFile.write("#define {0}_PACKET_NUM_FIELDS    {1}\n".format(
                    self.getNameUC(), len(self.usedFields)))

    def generatePacketType(self, outputFile):
        if len(self.usedFields) == 0: return

        outputFile.write("struct {0}Packet_s {1}\n".format(self.name, '{'))

        packetSize = 0
        for f in self.usedFields:
            outputFile.write("    uint{0}_t {1};\n".format(f[0] * 8, toCamelCase(f[1])))
            packetSize += f[0]

        if self.getParameterValue("crc"):
            # 2-byte crc
            if packetSize & 0x1:
                # add padding field
                outputFile.write("    uint8_t __reserved;\n")
                packetSize += 1
            outputFile.write("    uint16_t crc;\n")

         # finish the packet
        outputFile.write("} PACKED;\n\n")
        # and add a typedef
        outputFile.write("typedef struct {0}Packet_s {0}Packet_t;\n\n".format(self.name))

    def generateSerialOutputCode(self, outputFile, sensorsUsed):
#        useByDefault = False # TODO
#        if len(self.useCases) == 0 and not useByDefault:
#            return # this output is not used

        usedSizes = set()
        if self.isAggregate():
            for f in self.usedFields:
                usedSizes.add(f[0])
        else:
            for s in sensorsUsed:
                usedSizes.add(s.getDataSize())
        generateSerialFunctions(usedSizes, outputFile)

        if self.isAggregate():
            outputFile.write("static inline void serialPacketPrint(void)\n")
            outputFile.write("{\n")
            outputFile.write("    PRINT(\"======================\\n\");\n")
            for f in self.usedFields:
                outputFile.write("    serialPrintU{0}(\"{1}\", serialPacket.{2});\n".format(
                        f[0] * 8,
                        f[1], toCamelCase(f[1])))
            outputFile.write("}\n\n")

    def generateOutputCode(self, outputFile, sensorsUsed):
        if self.name.lower() == "serial":
            # special code for serial sink
            self.generateSerialOutputCode(outputFile, sensorsUsed)
            if not self.isAggregate():
                return

        outputFile.write("static inline void {0}PacketInit(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")

        # count const fields
        numConstFields = 0
        for f in self.usedFields:
            if isConstantField(f): numConstFields += 1
        outputFile.write("    {0}PacketNumFieldsFull = {1};\n".format(
                self.getNameCC(), numConstFields))

        for f in self.usedFields:
            if isConstantField(f):
                outputFile.write("    {0}Packet.{1} = {2};\n".format(
                        self.getNameCC(), toCamelCase(f.name), f.value))
            else:
                outputFile.write("    {0}Packet.{1} = {2}_NO_VALUE;\n".format(
                        self.getNameCC(), toCamelCase(f[1]), f[1].upper()))

        outputFile.write("}\n\n")

        outputFile.write("static inline void {0}PacketSend(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")

        if self.getParameterValue("crc"):
            outputFile.write("    {0}Packet.crc = crc16((const uint8_t *) &{0}Packet, sizeof({0}Packet) - 2);\n".format(
                    self.getNameCC()))

        useFunction = self.getParameterValue("useFunction")
        if useFunction and useFunction.value:
            outputFile.write("    {0};\n".format(useFunction.value))
        outputFile.write("    {0}PacketInit();\n".format(self.getNameCC()))
        outputFile.write("}\n\n")

        outputFile.write("static inline bool {0}PacketIsFull(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    return {0}PacketNumFieldsFull >= {1}_PACKET_NUM_FIELDS;\n".format(
                self.getNameCC(), self.getNameUC()))
        outputFile.write("}\n\n")

    def generateCallbackCode(self, sensor, outputFile):
        if not self.isAggregate():
            # this must be serial, because all other sinks require packets!
            outputFile.write("    {0}PrintU{1}(\"{2}\", {2});\n".format(
                    self.getNameCC(),
                    sensor.getDataSize() * 8,
                    sensor.getNameCC()))
            return

        # a packet; more complex case
        outputFile.write("    if ({0}Packet.{1} == {2}_NO_VALUE) {3}\n".format(
                self.getNameCC(), sensor.getNameCC(), sensor.getNameUC(), '{'))
        outputFile.write("        {0}PacketNumFieldsFull++;\n".format(self.getNameCC()))
        outputFile.write("    }\n")

        outputFile.write("    {0}Packet.{1} = {1};\n".format(
                self.getNameCC(), sensor.getNameCC()))

        outputFile.write("    if ({0}PacketIsFull()) {1}\n".format(self.getNameCC(), '{'))
        outputFile.write("        {0}PacketSend();\n".format(self.getNameCC()))
        outputFile.write("    }\n\n")

    def generateAppMainCode(self, outputFile):
        if self.isUsed() and self.isAggregate():
            outputFile.write("    {0}PacketInit();\n".format(self.getNameCC()))

######################################################
class StateUseCase(object):
    def __init__(self, name, value, conditions, branchNumber):
        self.name = name
        self.value = value
        self.conditions = list(conditions)
        if branchNumber in branchCollection.branches:
            branchCollection.branches[branchNumber].append(self)
        else:
            branchCollection.branches[branchNumber] = [self]

    def generateVariables(self, outputFile):
        outputFile.write("{0} {1} = {2};\n".format(self.value.getType(), self.name, self.value.asString()))

    def generateBranchEnterCode(self, outputFile):
        outputFile.write("    {0} = {1};\n".format(self.name, self.value.asString()))

    def generateBranchExitCode(self, outputFile):
        pass


######################################################
class ComponentRegister(object):
    module = None

  # load all componentsi for this platform from a file
    def load(self, architecture):
        # reset components
        self.actuators = {}
        self.sensors = {}
        self.outputs = {}
        self.systemParams = {}
        self.systemStates = {}
        self.defines = {}
        self.patterns = {}
        self.process = {}
        self.architecture = architecture
        # import the module (residing in "components" directory and named "<architecture>.py")
        self.module = __import__(architecture)
        # construct empty components from descriptions
        for n in self.module.components:
            isDuplicate = False
            name = n.name.lower()
            # print "load", name
            if n.typeCode == self.module.TYPE_ACTUATOR:
                if name in self.actuators:
                    isDuplicate = True
                else:
                    self.actuators[name] = Actuator(n)
            elif n.typeCode == self.module.TYPE_SENSOR:
                if name in self.sensors:
                    isDuplicate = True
                else:
                    self.sensors[name] = Sensor(n)
            elif n.typeCode == self.module.TYPE_OUTPUT:
                if name in self.outputs:
                    isDuplicate = True
                else:
                    self.outputs[name] = Output(n)
            if isDuplicate:
                userError("Component '{0}' duplicated for platform '{1}', ignoring\n".format(
                        n.name, architecture))

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
        #print "hasComponent? '" + keyword + "' '" + name + "'"
        return bool(self.findComponent(keyword.lower(), name.lower()))

    def useComponent(self, keyword, name, parameters, conditions, branchNumber):
        o = self.findComponent(keyword.lower(), name.lower())
        # IDE works with unfinished statements, this assert eliminates them.
        #assert o != None
        if o == None:
            return
        o.addUseCase(parameters, conditions, branchNumber)

    def setState(self, name, value, conditions, branchNumber):
        if name not in self.systemStates:
            self.systemStates[name] = []
        self.systemStates[name].append(StateUseCase(name, value, conditions, branchNumber))

    def generateVariables(self, outputFile):
        global processStructInits

        for s in self.systemStates.itervalues():
            s[0].generateVariables(outputFile)
        for p in self.patterns.itervalues():
            p.generateVariables(outputFile)
        for p in self.process.itervalues():
            p.generateVariables(processStructInits, outputFile)

    def getAllComponents(self):
        return set(self.actuators.values()).union(set(self.sensors.values())).union(set(self.outputs.values()))

    def getDependentProcess(self, name):
        result = []
        # Find any process statement who is dependent from this component
        for x in self.process.iteritems():
            if x[1].target == name.lower():
                result.append(x)
                print x[1].name, "depends on", name
        return result

    def replaceCode(self, componentName, parameterName):
        c = self.sensors.get(componentName, None)
        if c == None:
            c = self.actuators.get(componentName, None)
        if c == None:
            c = self.outputs.get(componentName, None)
        if c == None:
            c = self.process.get(componentName, None)
            if c != None:
                if parameterName == 'value':
                    return c.name
        # not found?
        if c == None:
            if parameterName == 'ispresent':
                return "false"
            else:
                userError("Component '{0}' not known\n".format(componentName))
                return "false"
        # found
        if parameterName == 'ispresent':
            return "true"
        if parameterName == 'iserror':
            errorFunction = c.getParameterValue("errorFunction", None)
            if errorFunction:
                c.markAsUsed()
                return errorFunction
            else:
                userError("Parameter '{0}' for component '{1} has no error attribute!'\n".format(parameterName, componentName))
                return "false"
        if parameterName == 'value':
            # TODO: otherwise unused component might become usable because of this!
            # TODO FIXME: what if cache is used?!
            readFunction = c.getParameterValue("readFunction", None)
            if readFunction:
                c.markAsUsed()
                return readFunction
            else:
                userError("Parameter '{0}' for component '{1} is not readable!'\n".format(parameterName, componentName))
                return "false"

        if parameterName in ['average', 'stdev', 'filter']:
            return "get{}Value(&{}{})".format(toTitleCase(parameterName),
                                              componentName,
                                              toTitleCase(parameterName))

        userError("Unknown parameter '{0}' for component '{1}'\n".format(parameterName, componentName))
        return "false"


######################################################
# global variables
componentRegister = ComponentRegister()
branchCollection = BranchCollection()
conditionCollection = ConditionCollection()
processFunctionsUsed = dict()
processStructInits = list()

def clearGlobals():
    global componentRegister
    global branchCollection
    global conditionCollection
    global processFunctionsUsed
    global processStructInits
    componentRegister = ComponentRegister()
    branchCollection = BranchCollection()
    conditionCollection = ConditionCollection()
    processFunctionsUsed = dict()
    processStructInits = list()

