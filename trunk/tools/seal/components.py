import sys, string
from structures import *

# pre-allocated packet field ID
PACKET_FIELD_ID_COMMAND    = 0
PACKET_FIELD_ID_TIMESTAMP  = 1
PACKET_FIELD_ID_ADDRESS    = 2
# first free packet field ID (note that ID 31, 63, 95, 127 etc. are reserved for extension)
PACKET_FIELD_ID_FIRST_FREE = 3

######################################################

def generateSerialFunctions(intSizes, outputFile):
    for (size, dataType) in intSizes:
        if dataType[0] == 'u': formatSpecifier = "u"
        else: formatSpecifier = "d"
        if size == 4: formatSpecifier = "l" + formatSpecifier
        outputFile.write("static inline void serialPrint_{0}(const char *name, {0} value)\n".format(dataType))
        outputFile.write("{\n")
        outputFile.write('    PRINTF("%s=%{}\\n", name, value);\n'.format(formatSpecifier))
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
        outputFile.write("}\n\n")

    def generateStopCode(self, b, outputFile):
        number = b[0]
        useCases = b[1]
        outputFile.write("void branch{0}Stop(void)\n".format(number))
        outputFile.write("{\n")
        # the default branch NEVER stops.
        if number == 0:
            outputFile.write("    // never executed\n")
        else:
            for uc in useCases:
                uc.generateBranchExitCode(outputFile)
        outputFile.write("}\n\n")

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
                componentRegister.userError("Parameter '{0}' not known for component {1}\n".format(
                        p[0], component.name))
                continue
            # update parameters with user's value, if given. If no value given, only name: treat it as 'True',
            # because value 'None' means that the parameter is supported, but not specified by the user.
            if p[1] is not None:
                self.parameters[p[0]] = p[1]
            else:
                self.parameters[p[0]] = Value(True)
        self.readFunctionSuffix = ""
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
            self.period = toMilliseconds(p, componentRegister)
        else:
            self.period = None
        p = self.parameters.get("pattern")
        if p:
            self.pattern = p.asString()
            if self.pattern: self.period = None
        else:
            self.pattern = None
        p = self.parameters.get("once")
        if p:
            self.once = bool(p.value)
            if self.once: self.period = None
        else:
            self.once = None

        self.on = False
        p = self.parameters.get("on")
        if p:
            self.on = bool(p.value)
            if self.on: self.once = True
            if self.once: self.period = None
        else:
            self.once = None

        self.off = False
        p = self.parameters.get("off")
        if p:
            self.off = bool(p.value)
            if self.off: self.once = True
            if self.once: self.period = None
        else:
            self.once = None

#        for x in ['average', 'stdev', 'filter']:
#            p = self.parameters.get(x)
#            if p and p.value != '':
#                    self.__setattr__(x, p.asString())
#            else:
#                self.__setattr__(x, None)

#        if (self.period and self.pattern) or (self.period and self.once) or (self.pattern and self.once):
#            if self.period and self.pattern:
#                componentRegister.userError("Both 'period' and 'pattern' specified for component '{0}' use case\n".format(component.name))
#            else:
#                componentRegister.userError("Both 'once' and 'period' or 'pattern' specified for component '{0}' use case\n".format(component.name))
        if (self.pattern and self.once):
            componentRegister.userError("Both 'once' and 'pattern' specified for component '{0}' use case\n".format(component.name))

        if component.isRemote:
            self.generateAlarm = False
        else:
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
#        global processStructInits

        if self.generateAlarm:
            outputFile.write(
                "Alarm_t {0}{1}Alarm{2};\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))
            if type(self) is Sensor:
                for s in self.component.subsensors:
                    outputFile.write("Alarm_t {0}PreAlarm;\n".format(s.getNameCC()))

#        if self.filter:
#            outputFile.write(
#                "Filter_t {}{}Filter{};\n".format(
#                    self.component.getNameCC(), self.branchName, self.numInBranch))
#            processStructInits.append("    {}{}Filter{} = filterInit({});\n".format(
#                    self.component.getNameCC(), self.branchName, self.numInBranch,
#                    self.filter))

#        if self.average:
#            outputFile.write(
#                "Average_t {}{}Average{};\n".format(
#                    self.component.getNameCC(), self.branchName, self.numInBranch))
#            processStructInits.append("    {}{}Average{} = avgInit({});\n".format(
#                    self.component.getNameCC(), self.branchName, self.numInBranch,
#                    self.average))

#        if self.stdev:
#            outputFile.write(
#                "Stdev_t {}{}Stdev{};\n".format(
#                    self.component.getNameCC(), self.branchName, self.numInBranch))
#            processStructInits.append("    {}{}Stdev{} = stdevInit({});\n".format(
#                    self.component.getNameCC(), self.branchName, self.numInBranch,
#                    self.stdev))

    def generateCallbacks(self, outputFile, outputs):
        ccname = self.component.getNameCC()
        ccname += self.branchName
        ucname = self.component.getNameUC()
        ucname += self.branchName.upper()

#        print "useFunction = ", useFunction

#            outputFile.write("void {0}{1}Callback(uint32_t value)\n".format(ccname, self.numInBranch))
#            outputFile.write("{\n")
#            if type(self.component) is Sensor:
#                pass
#            outputFile.write("}\n\n")

        if self.generateAlarm or self.component.isRemote:
            if self.generateAlarm: argumentType = "void *" 
            else: argumentType = "uint32_t"
            outputFile.write("void {0}{1}Callback({2} __unused)\n".format(
                    ccname, self.numInBranch, argumentType))
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
                elif self.on:
                    onFunction = self.component.getDependentParameterValue(
                        "onFunction", self.parameters)
                    outputFile.write("    {0};\n".format(onFunction))
                elif self.off:
                    offFunction = self.component.getDependentParameterValue(
                        "offFunction", self.parameters)
                    outputFile.write("    {0};\n".format(offFunction))
                else:
                    useFunction = self.component.getDependentParameterValue(
                        "useFunction", self.parameters)
                    outputFile.write("    {0};\n".format(useFunction))
            elif type(self.component) is Sensor:
                intTypeName = self.component.getDataType()
                outputFile.write("    bool isFilteredOut = false;\n")
                outputFile.write("    {0} {1}Value = {2}ReadProcess{3}(&isFilteredOut);\n".format(
                        intTypeName, self.component.getNameCC(),
                        self.component.getNameCC(), self.readFunctionSuffix))
                if not self.component.syncOnlySensor:
                    outputFile.write("    if (!isFilteredOut) {\n")
                    for o in outputs:
                        o.generateCallbackCode(self.component, outputFile, self.readFunctionSuffix)
                        outputFile.write("    }\n")
                elif self.period:
                    for s in self.component.subsensors:
                        if s.getParameterValue("prereadFunction") is None: continue
                        prereadTime = s.specification.readTime
                        if prereadTime == 0: continue
                        outputFile.write("    alarmSchedule(&{0}PreAlarm, {2}_PERIOD{1} - {3});\n".format(
                                s.getNameCC(), self.numInBranch, ucname, prereadTime))

#                if self.filter:
#                    filterName = "{}{}Filter{}".format(self.component.getNameCC(),
#                                self.branchName, self.numInBranch)
#                    outputFile.write("    if(addFilter(&{}, &{}Value)) ".format(
#                         filterName, self.component.getNameCC()))
#                    outputFile.write("{\n")
#                    if self.average:
#                        outputFile.write("        addAverage(&{}{}Average{}, &getValue(&{}));\n".format(
#                            self.component.getNameCC(), self.branchName,
#                            self.numInBranch, filterName))
#                    if self.stdev:
#                        outputFile.write("        addStdev(&{}{}Stdev{}, &getValue(&{}));\n".format(
#                            self.component.getNameCC(), self.branchName,
#                            self.numInBranch, filterName))
#                    self.checkDependencies(self.component.getNameCC(), 2, outputFile)
#                if True:
#                    for o in outputs:
#                        o.generateCallbackCode(self.component, outputFile)
#                    outputFile.write("    }\n")
#                else:
#                    if self.average:
#                        outputFile.write("    addAverage(&{}{}Average{}, &{}Value);\n".format(
#                            self.component.getNameCC(), self.branchName,
#                            self.numInBranch, self.component.getNameCC()))
#                    if self.stdev:
#                        outputFile.write("    addStdev(&{}{}Stdev{}, &{}Value);\n".format(
#                            self.component.getNameCC(), self.branchName,
#                            self.numInBranch, self.component.getNameCC()))
#                    self.checkDependencies(self.component.getNameCC(), 1, outputFile)
#                    for o in outputs:
#                        o.generateCallbackCode(self.component, outputFile)

            if self.component.isRemote:
                pass
            elif self.once:
                pass
            elif self.period:
                outputFile.write("    alarmSchedule(&{0}Alarm{1}, {2}_PERIOD{1});\n".format(
                        ccname, self.numInBranch, ucname))
            elif self.pattern:
                outputFile.write("    alarmSchedule(&{0}Alarm{1}, __pattern_{2}[__pattern_{2}Cursor]);\n".format(
                        ccname, self.numInBranch, self.pattern))
                outputFile.write("    __pattern_{0}Cursor++;\n".format(self.pattern))
                outputFile.write("    __pattern_{0}Cursor %= sizeof(__pattern_{0}) / sizeof(*__pattern_{0});\n".format(
                        self.pattern))
            outputFile.write("}\n\n")

    # Checks for dependencies for given name
#    def checkDependencies(self, name, indent, outputFile):
#        for x in componentRegister.getDependentProcess(name):
#            x[1].generateCallbackCode(outputFile, name, indent, self.checkDependencies)

    def generateAppMainCode(self, outputFile):
        ccname = self.component.getNameCC()
        ccname += self.branchName
        if self.generateAlarm:
            outputFile.write("    alarmInit(&{0}Alarm{1}, {0}{1}Callback, NULL);\n".format(
                   ccname, self.numInBranch))
            if type(self) is Sensor:
                for s in self.component.subsensors:
                    outputFile.write("    alarmInit(&{0}PreAlarm, {0}PreReadCallback, NULL);\n".format(s.getNameCC()))
        elif self.component.isRemote:
            outputFile.write("    sealCommRegisterInterest({}, {}{}Callback);\n".format(
                    self.component.systemwideID, self.component.getNameCC(), self.numInBranch))

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
            return self.parameters[parameter].getCodeForGenerator(componentRegister, None)
        return self.component.getParameterValue(parameter, defaultValue)


######################################################
class Component(object):
    def __init__(self, name, specification):
        self.name = name
        # create dictionary for parameters
        self.parameters = {}
        for p in dir(specification):
            if type(specification.__getattribute__(p)) is componentRegister.module.SealParameter:
                self.parameters[p] = specification.__getattribute__(p).value
        self.useCases = []
        self.markedAsUsed = False
        self.usedForConditions = False
        # save specification (needed for dependent parameters)
        self.specification = specification
        self.functionTree = None
        self.isRemote = False

    def markAsUsed(self):
        self.markedAsUsed = True

    def markAsUsedForConditions(self):
        self.markedAsUsed = True
        self.usedForConditions = True

    def isUsed(self):
        return self.markedAsUsed or bool(len(self.useCases))

    def getNameUC(self):
        return self.name.upper()

    def getNameLC(self):
        return self.name.lower()

    def getNameCC(self):
        assert self.name != ''
        return string.lower(self.name[0]) + self.name[1:]

    def getNameTC(self):
        assert self.name != ''
        return string.upper(self.name[0]) + self.name[1:]

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
                d = componentRegister.parameterDefines.get(p[1].asString())
                if d is None:
                    componentRegister.userError("No parameter define with name '{0}' is present (for component '{1}')\n".format(
                            p[1].asString(), self.name))
                else:
                    for pd in d.parameters:
                        if pd[0] in finalParameters:
                            componentRegister.userError("Parameter '{0}' already specified for component '{1}'\n".format(pd[0], self.name))
                        else:
                            finalParameters[pd[0]] = pd[1]
            else:
                if p[0] in finalParameters:
                    componentRegister.userError("Parameter '{0}' already specified for component '{1}'\n".format(p[0], self.name))
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
        # if self.isUsed():
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
        if type(self) is Output:
            return
        if type(self) is Sensor:
#            print self.specification.readFunctionDependsOnParams
#            print self.useCases
#            print self.usedForConditions
            if self.specification.readFunctionDependsOnParams:
                for uc in self.useCases:
                    self.generateReadFunctions(outputFile, uc)
                if self.usedForConditions:
                    self.generateReadFunctions(outputFile, None)
            else:
                self.generateReadFunctions(outputFile, None)

            if self.doGenerateSyncCallback:
                self.generateSyncCallback(outputFile, outputs)

            for s in self.subsensors:
                s.generatePrereadCallback(outputFile)

        for uc in self.useCases:
            uc.generateCallbacks(outputFile, outputs)

    def generateAppMainCode(self, outputFile):
        for uc in self.useCases:
            uc.generateAppMainCode(outputFile)
#        if self.isRemote:
#            outputFile.write("    sealCommRegisterInterest({}, {}Callback);\n".format(
#                    self.systemwideID, self.getNameCC()))

    def getConfig(self, outputFile):
        if self.isUsed():
            config = self.getSpecialValue("extraConfig")
            if config is not None:
                return config
        return None

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
    def __init__(self, name, specification):
        super(Actuator, self).__init__(name, specification)

######################################################
class Sensor(Component):
    def __init__(self, name, specification):
        super(Sensor, self).__init__(name, specification)
        if self.specification.minUpdatePeriod is None:
            self.minUpdatePeriod = 1000 # default value
        else:
            self.minUpdatePeriod = self.specification.minUpdatePeriod
        self.cacheNeeded = False
        cnParam = self.getParameterValue("cache")
        if cnParam is not None:
            self.cacheNeeded = cnParam
        self.cacheNumber = 0
        self.readFunctionNum = 0
        self.generatedDefaultReadFunction = False
        if name.lower() == "command" or name.lower() == "remotecommand":
            self.systemwideID = PACKET_FIELD_ID_COMMAND
        else:
            self.systemwideID = componentRegister.allocateSensorId()

        self.doGenerateSyncCallback = False
        self.syncOnlySensor = False
        self.subsensors = []

    def updateParameters(self, dictionary):
        for p in dictionary.iteritems():
            self.parameters[p[0]] = p[1].value
        cnParam = self.getParameterValue("cache")
        if cnParam is not None:
            self.cacheNeeded = cnParam

    def getDataSize(self):
#        size = self.getParameterValue("dataSize")
#        if size is None:
#            size = 2 # TODO: issue warning
#        return size
        return 4 # TODO: optimize

    def getDataType(self):
#        dataType = self.getParameterValue("dataType")
#        if dataType is not None:
#            return dataType
#        return "uint{}_t".format(self.getDataSize() * 8)
        return "int32_t" # TODO: optimize

    def getMaxValue(self):
        # return "0x" + "ff" * self.getDataSize()
        return "LONG_MAX"

    def getMinValue(self):
        # return "0"
        return "LONG_MIN"

    def getNoValue(self):
        return "0x" + "ff" * self.getDataSize()

    def generateConstants(self, outputFile):
        super(Sensor, self).generateConstants(outputFile)
        if self.isUsed() or self.doGenerateSyncCallback:
            outputFile.write("#define {0}_NO_VALUE    {1}\n".format(self.getNameUC(), self.getNoValue()))
            outputFile.write("#define {0}_TYPE_ID     0x{1:x}\n".format(self.getNameUC(), (1 << self.systemwideID)))

    def isCacheNeeded(self, numCachedSensors):
        if self.cacheNeeded: return True
        for uc in self.useCases:
            if uc.period is not None and uc.period < self.minUpdatePeriod:
                self.cacheNeeded = True
                self.cacheNumber = numCachedSensors
                break

        return self.cacheNeeded

    def isCacheNeededForCondition(self):
        global componentRegister
        if self.cacheNeeded: return True
        conditionEvaluatePeriod = 1000 # once in second
        if conditionEvaluatePeriod < self.minUpdatePeriod:
            self.cacheNeeded = True
            self.cacheNumber = componentRegister.numCachedSensors
            componentRegister.numCachedSensors += 1
        return self.cacheNeeded

    def generateSyncCallback(self, outputFile, outputs):
        useFunction = self.getDependentParameterValue("useFunction", self.parameters)

        outputFile.write("void {0}SyncCallback(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    bool isFilteredOut = false;\n")
        outputFile.write("    {0} {1}Value = {1}ReadProcess(&isFilteredOut);\n".format(
                self.getDataType(), self.getNameCC()))
        outputFile.write("    if (!isFilteredOut) {\n")
        for o in outputs:
            o.generateCallbackCode(self, outputFile, "")
        outputFile.write("    }\n")
        outputFile.write("}\n")

    def generatePrereadCallback(self, outputFile):
        func = self.getParameterValue("prereadFunction")
        if func is None: func = ""

        outputFile.write("void {0}PreReadCallback(void *__unused)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    " + func + ";\n")
        outputFile.write("}\n\n")

    def addSubsensors(self):
        for a in self.functionTree.arguments:
            sensor = componentRegister.findComponentByName(a.generateSensorName())
            assert sensor
            assert type(sensor) is Sensor
            sensor.doGenerateSyncCallback = True
            self.subsensors.append(sensor)

    #########################################################################
    # Start of reading and processing function generation
    #########################################################################

    def getRawReadFunction(self, suffix, root):
        if self.isRemote:
            # remote sensors require specialHandling
            return "sealCommReadValue({})".format(self.systemwideID)

        rawReadFunc = "{}ReadRaw{}()".format(self.getNameCC(), suffix)
#        if len(suffix) == 0 and self.cacheNeeded:
        cacheNeeded = False
        if self.cacheNeeded: cacheNeeded = True
        if root and root.cacheNeeded: cacheNeeded = True
        if cacheNeeded:
            dataFormat = str(self.getDataSize() * 8)
            return "cacheReadSensorU{0}({1}, &{2}, {3})".format(
                dataFormat, self.cacheNumber, rawReadFunc, self.minUpdatePeriod)
        return rawReadFunc

    def getGeneratedFunctionName(self, fun):
        readFunctionSuffix = str(self.readFunctionNum)
        self.readFunctionNum += 1
        return self.getNameCC() + "Read" + toTitleCase(fun) + readFunctionSuffix

    def generateAbsFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(1, componentRegister):
            return ""
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("abs")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return value < 0 ? -value : value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateNegFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(1, componentRegister):
            return ""
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("neg")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = -{1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateMapFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(5, componentRegister):
            return ""
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("map")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return map(value, {0}, {1}, {2}, {3});\n".format(
                functionTree.arguments[1], functionTree.arguments[2],
                functionTree.arguments[3], functionTree.arguments[4]))
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateUnaryMinFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("min")
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    static {0} minValue = {1};\n".format(self.getDataType(), self.getMaxValue()))
        outputFile.write("    bool b = false, *isFilteredOut = &b;\n")
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    if (!*isFilteredOut) {\n")
        outputFile.write("        if (minValue > value) minValue = value;\n")
        outputFile.write("    }\n")
        outputFile.write("    return minValue;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateUnaryMaxFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root, root)

        funName = self.getGeneratedFunctionName("min")
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    static {0} maxValue = {1};\n".format(self.getDataType(), self.getMinValue()))
        outputFile.write("    bool b = false, *isFilteredOut = &b;\n")
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    if (!*isFilteredOut) {\n")
        outputFile.write("        if (maxValue < value) maxValue = value;\n")
        outputFile.write("    }\n")
        outputFile.write("    return maxValue;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateNaryMinFunction(self, outputFile, functionTree, root):
        subReadFunctions = []
        for a in functionTree.arguments:
            subReadFunctions.append(self.generateSubReadFunctions(
                    outputFile, None, a, root))

        funName = self.getGeneratedFunctionName("min")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} tmp, value = {1};\n".format(self.getDataType(), self.getMaxValue()))
        for f in subReadFunctions:
            outputFile.write("    tmp = {}; if (tmp < value) value = tmp;\n".format(f))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateNaryMaxFunction(self, outputFile, functionTree, root):
        subReadFunctions = []
        for a in functionTree.arguments:
            subReadFunctions.append(self.generateSubReadFunctions(
                    outputFile, None, a, root))
        funName = self.getGeneratedFunctionName("max")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} tmp, value = {1};\n".format(self.getDataType(), self.getMinValue()))
        for f in subReadFunctions:
            outputFile.write("    tmp = {}; if (tmp > value) value = tmp;\n".format(f))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateMinFunction(self, outputFile, functionTree, root):
        if len(functionTree.arguments) == 1:
            if functionTree.arguments[0].function == "take":
                return self.generateTakeFunction(outputFile, functionTree.arguments[0], "min")
            return self.generateUnaryMinFunction(outputFile, functionTree, root)
        return self.generateNaryMinFunction(outputFile, functionTree, root)

    def generateMaxFunction(self, outputFile, functionTree, root):
        if len(functionTree.arguments) == 1:
            return self.generateUnaryMaxFunction(outputFile, functionTree, root)
            if functionTree.arguments[0].function == "take":
                return self.generateTakeFunction(outputFile, functionTree.arguments[0], "min")
        return self.generateNaryMaxFunction(outputFile, functionTree, root)

    def generateSquareFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(1, componentRegister):
            return ""
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("square")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1} * {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateSqrtFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(1, componentRegister):
            return ""
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        componentRegister.additionalConfig.add("algo")

        funName = self.getGeneratedFunctionName("sqrt")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = intSqrt({1});\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"
    

    def generateAvgFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(1, componentRegister):
            return ""
        if functionTree.arguments[0].function == "take":
            return self.generateTakeFunction(outputFile, functionTree.arguments[0], "avg")
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("avg")
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    static int32_t totalSum;\n")
        outputFile.write("    static uint16_t totalCount;\n")
        outputFile.write("    bool b = false, *isFilteredOut = &b;\n")
        outputFile.write("    {0} tmp = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    if (!*isFilteredOut) {\n")
        outputFile.write("        totalSum += tmp;\n")
        outputFile.write("        totalCount++;\n")
        outputFile.write("    }\n")
        outputFile.write("    return totalCount ? totalSum / totalCount : 0;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateStdevFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(1, componentRegister):
            return ""
        if functionTree.arguments[0].function == "take":
            return self.generateTakeFunction(outputFile, functionTree.arguments[0], "stdev")
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        componentRegister.additionalConfig.add("algo")

        funName = self.getGeneratedFunctionName("stdev")
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        # stdev = sqrt(squared_average - average_squared)
        outputFile.write("    static int32_t totalSum;\n")
        # XXX: does not work correctly without the volatile - compiler bug?
        outputFile.write("    static volatile uint64_t totalSquaredSum;\n")
        outputFile.write("    static uint16_t totalCount;\n")
        outputFile.write("    bool b = false, *isFilteredOut = &b;\n")
        outputFile.write("    {0} tmp = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    if (!*isFilteredOut) {\n")
        outputFile.write("        totalSum += tmp;\n")
        outputFile.write("        totalSquaredSum += (uint32_t)tmp * tmp;\n")
        outputFile.write("        totalCount++;\n")
        outputFile.write("    }\n")
        outputFile.write("    int32_t average = totalCount ? totalSum / totalCount : 0;\n")
        outputFile.write("    uint32_t squaredAverage = totalCount ? totalSquaredSum / totalCount : 0;\n")
        outputFile.write("    return intSqrt(squaredAverage - average * average);\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateSumFunction(self, outputFile, functionTree, root):
        if len(functionTree.arguments) == 1 and functionTree.arguments[0].function == "take":
            return self.generateTakeFunction(outputFile, functionTree.arguments[0], "sum")
        subReadFunctions = []
        for a in functionTree.arguments:
            subReadFunctions.append(self.generateSubReadFunctions(
                    outputFile, None, a, root))

        funName = self.getGeneratedFunctionName("sum")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = 0;\n".format(self.getDataType()))
        for f in subReadFunctions:
            outputFile.write("    value += {};\n".format(f))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateArithmeticFunction(self, outputFile, functionTree, op, root):
        if not functionTree.checkArgs(2, componentRegister):
            return ""

        subReadFunction1 = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)
        subReadFunction2 = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[1], root)

        funName = self.getGeneratedFunctionName(functionTree.function)
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1} {2} {3};\n".format(
                self.getDataType(), subReadFunction1, op, subReadFunction2))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generatePowerFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(2, componentRegister):
            return ""

        power = functionTree.arguments[1].asConstant()
        if power is None:
            componentRegister.userError("Second argument of power() function is expected to be a constant!\n")
            return ""

        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("power")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} tmp = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    {0} value = 1;\n".format(self.getDataType()))
        for i in range(power):
            outputFile.write("    value *= tmp;\n")
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateFilterRangeFunction(self, outputFile, functionTree, op, root):
        if not functionTree.checkArgs(3, componentRegister):
            return ""

        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)

        thresholdMin = functionTree.arguments[1].asConstant()
        thresholdMax = functionTree.arguments[2].asConstant()
        if thresholdMin is None or thresholdMax is None:
            componentRegister.userError("Second and third arguments of filterRange() function is expected to be constants!\n")
            return ""

        funName = self.getGeneratedFunctionName(functionTree.function)
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1};\n".format(
                self.getDataType(), subReadFunction))
        outputFile.write("    if (value < {0} || value > {1}) *isFilteredOut = true;\n".format(
                thresholdMin, thresholdMax))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateFilterFunction(self, outputFile, functionTree, root):
        if not functionTree.checkArgs(2, componentRegister):
            return ""

        kind = functionTree.function[6:]

        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], root)
        threshold = functionTree.arguments[1].asConstant()
        if threshold is None:
            componentRegister.userError("Second argument of {}() function is expected to be a constant!\n".format(
                    functionTree.function))
            return ""

        funName = self.getGeneratedFunctionName(functionTree.function)
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1};\n".format(
                self.getDataType(), subReadFunction))
        if kind == "equal": op = "=="
        elif kind == "notequal": op = "!="
        elif kind == "less": op = "<"
        elif kind == "lessorequal": op = "<="
        elif kind == "more": op = ">"
        elif kind == "moreorequal": op = ">="
        else:
            componentRegister.userError("Unhandled kind of filter: '{}'\n".format(functionTree.function))
            op = '=='
        outputFile.write("    if (!(value {0} {1})) *isFilteredOut = true;\n".format(op, threshold))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateSyncFunction(self, outputFile, useCase, functionTree, root):
        assert self.syncOnlySensor

        funName = self.getGeneratedFunctionName(functionTree.function)
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        for s in self.subsensors:
            outputFile.write("    extern void {}SyncCallback(void);\n".format(s.getNameCC()))
            outputFile.write("    {}SyncCallback();\n".format(s.getNameCC()))
        outputFile.write("    return 0;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateTakeFunction(self, outputFile, functionTree, aggregateFunction):
        if not functionTree.checkArgs(2, componentRegister):
            return ""
        subReadFunction = self.generateSubReadFunctions(
            outputFile, None, functionTree.arguments[0], None)
        numToTake = functionTree.arguments[1].asConstant()
        if numToTake is None:
            componentRegister.userError("Second argument of take() function is expected to be a constant!\n")
            return ""

        funName = self.getGeneratedFunctionName("take")
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    static {0} values[{1}];\n".format(self.getDataType(), numToTake))
        outputFile.write("    static uint16_t valuesCursor;\n")
        outputFile.write("    bool b = false, *isFilteredOut = &b;\n")
        outputFile.write("    {0} tmp = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    if (!*isFilteredOut) {\n")
        outputFile.write("        values[valuesCursor] = tmp;\n")
        outputFile.write("        valuesCursor = (valuesCursor + 1) % {};\n".format(numToTake))
        outputFile.write("    }\n")
        outputFile.write("    {} value;\n".format(self.getDataType()))
        # MINIMUM
        if aggregateFunction == "min":
            outputFile.write("    value = {};\n".format(self.getMaxValue()))
            outputFile.write("    int i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        if (values[i] < value) value = values[i];\n")
        # MAXIMUM
        elif aggregateFunction == "max":
            outputFile.write("    value = {};\n".format(self.getMinValue()))
            outputFile.write("    int i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        if (values[i] > value) value = values[i];\n")
        # SUM
        elif aggregateFunction == "sum":
            outputFile.write("    value = 0;\n")
            outputFile.write("    int i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        value += values[i];\n")
        # AVERAGE
        elif aggregateFunction == "avg":
            outputFile.write("    value = 0;\n")
            outputFile.write("    int i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        value += values[i];\n")
            outputFile.write("    value /= {};\n".format(numToTake))
        # STANDARD DEVIATION
        elif aggregateFunction == "stdev":
            outputFile.write("    int32_t avg = 0;\n")
            outputFile.write("    int i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        avg += values[i];\n")
            outputFile.write("    avg /= {};\n".format(numToTake))
            outputFile.write("    value = 0;\n")
            outputFile.write("    for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        value += abs(values[i] - avg);\n")
            outputFile.write("    value /= {};\n".format(numToTake))
        # OTHER
        else:
            componentRegister.userError("take(): unknown aggregate function {}()!\n".format(aggregateFunction));
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateSubReadFunctions(self, outputFile, useCase, functionTree, root):
        if functionTree is None:
            # a physical sensor; generate just raw read function

            if self.specification.readFunctionDependsOnParams:
                readFunctionSuffix = str(self.readFunctionNum)
                self.readFunctionNum += 1
            else:
                readFunctionSuffix = ""

            self.markAsUsed()

            outputFile.write("static inline {0} {1}ReadRaw{2}(void)\n".format(
                    self.getDataType(), self.getNameCC(), readFunctionSuffix))
            outputFile.write("{\n")

            if self.specification.readFunctionDependsOnParams:
                if useCase: params = useCase.parameters
                else: params = root.parameters
                specifiedReadFunction = self.getDependentParameterValue("readFunction", params)
            else:
                specifiedReadFunction = self.getParameterValue("readFunction", None)

#        useFunction = self.component.getDependentParameterValue("useFunction", self.parameters)
#        print "useFunction = ", useFunction

            if specifiedReadFunction is None:
                componentRegister.userError("Sensor '{}' has no valid read function!\n".format(self.name))
                specReadFunction = "0"
            outputFile.write("    return {};\n".format(specifiedReadFunction))
            outputFile.write("}\n\n")

            if useCase is None: self.generatedDefaultReadFunction = True

            # return either this, just generated function or cacheRead()
            return self.getRawReadFunction(readFunctionSuffix, root)

        # if this is a sensor name with no arguments, find the sensor and recurse
        if len(functionTree.arguments) == 0:
            # TODO: add special case for constants!
            # print "functionTree.function =", functionTree.function
            if type(functionTree.function) is Value:
                return functionTree.function.asString()
            else:
                sensor = componentRegister.findComponentByName(functionTree.function)
                assert sensor
                assert type(sensor) is Sensor
                return sensor.generateSubReadFunctions(outputFile, useCase, None, root)

        # run through the tree and generate all needed
        if functionTree.function == "abs":
            return self.generateAbsFunction(outputFile, functionTree, root)
        if functionTree.function == "neg":
            return self.generateNegFunction(outputFile, functionTree, root)
        if functionTree.function == "map":
            return self.generateMapFunction(outputFile, functionTree, root)
        if functionTree.function == "min":
            return self.generateMinFunction(outputFile, functionTree, root)
        if functionTree.function == "max":
            return self.generateMaxFunction(outputFile, functionTree, root)
        if functionTree.function == "square":
            return self.generateSquareFunction(outputFile, functionTree, root)
        if functionTree.function == "sqrt":
            return self.generateSqrtFunction(outputFile, functionTree, root)
        if functionTree.function == "avg":
            return self.generateAvgFunction(outputFile, functionTree, root)
        if functionTree.function == "stdev":
            return self.generateStdevFunction(outputFile, functionTree, root)
        if functionTree.function == "sum":
            return self.generateSumFunction(outputFile, functionTree, root)
        if functionTree.function == "minus":
            return self.generateArithmeticFunction(outputFile, functionTree, '-', root)
        if functionTree.function == "multiply":
            return self.generateArithmeticFunction(outputFile, functionTree, '*', root)
        if functionTree.function == "divide":
            return self.generateArithmeticFunction(outputFile, functionTree, '/', root)
        if functionTree.function == "modulo":
            return self.generateArithmeticFunction(outputFile, functionTree, '%', root)
        if functionTree.function == "power":
            return self.generatePowerFunction(outputFile, functionTree, root)
        if functionTree.function == "filterRange":
            return self.generateFilterRangeFunction(outputFile, functionTree, root)
        if functionTree.function[:6] == "filter":
            return self.generateFilterFunction(outputFile, functionTree, root)
        if functionTree.function == "sync":
            if functionTree != self.functionTree:
                componentRegister.userError("sync() function only allowed at the top level!\n")
            else:
                return self.generateSyncFunction(outputFile, useCase, functionTree, root)
        if functionTree.function == "take":        
            componentRegister.userError("take() function can be used only as an argument to one of the following:\n" +
                      "    min(), max(), sum(), avg(), stdev()!\n")
            return "0"
        componentRegister.userError("unhandled function {}()\n".format(functionTree.function))
        return "0"

    def generateReadFunctions(self, outputFile, useCase):
        if not self.isUsed(): return

        if useCase is not None:
            readFunctionSuffix = str(self.readFunctionNum)
            useCase.readFunctionSuffix = readFunctionSuffix
            self.readFunctionNum += 1
        else:
            readFunctionSuffix = ""

        # do not generate same read function twice
        if self.generatedDefaultReadFunction: return

        subReadFunction = self.generateSubReadFunctions(
            outputFile, useCase, self.functionTree, self)

        # generate reading and processing function
        outputFile.write("static inline {0} {1}ReadProcess{2}(bool *isFilteredOut)\n".format(
                self.getDataType(), self.getNameCC(), readFunctionSuffix))
        outputFile.write("{\n")
        outputFile.write("    return {};\n".format(subReadFunction))
        outputFile.write("}\n\n")

######################################################
class PacketField(object):
    def __init__(self, sensorID, sensorName, dataSize, dataType, isRealSensor=True):
        self.sensorID = sensorID
        self.sensorName = sensorName
        # always use 4 bytes, because decoding otherwise is too messy
        # (could and should be optimized if the need arises...)
        self.dataSize = 4
        self.dataType = "uint32_t"
        self.isRealSensor = isRealSensor

class Output(Component):
    def __init__(self, name, specification):
        super(Output, self).__init__(name, specification)
        self.isAggregateCached = None
        self.packetFields = []
        self.headerMasks = []
        self.useOnlyFields = None
        self.numSensorFields = 0

    def setFields(self, fields):
        self.useOnlyFields = set(fields)

    def getParameterValue(self, parameter, defaultValue = None):
        if len(self.useCases) != 0 and parameter in self.useCases[0].parameters:
            return self.useCases[0].parameters[parameter]
        return super(Output, self).getParameterValue(parameter, defaultValue)

    def isAggregate(self):
        if self.isAggregateCached is None:
            self.isAggregateCached = self.getParameterValue("aggregate", False)
        return self.isAggregateCached

    def generateVariables(self, outputFile):
        if self.isAggregate() and len(self.packetFields) != 0:
            outputFile.write("{0}Packet_t {1}Packet;\n".format(self.getNameTC(), self.getNameCC()))
            outputFile.write("uint_t {0}PacketNumFieldsFull;\n".format(self.getNameCC()))

    def addPacketTypes(self, s):
        # special case for dummy sync sensors
        if s.syncOnlySensor:
            for subsensor in s.subsensors:
                self.addPacketTypes(subsensor)
            return
        # normal case
        self.packetFields.append(PacketField(
                s.systemwideID, s.getNameCC(), s.getDataSize(), s.getDataType()))
        self.numSensorFields += 1

    def definePacketType(self):
        # TODO: handle the case of multiple use cases of the same sensor with different value params! (marginal)
        self.packetFields = []
        self.numSensorFields = 0
        for s in componentRegister.sensors.itervalues():
            if not s.isUsed():
                continue
            if self.useOnlyFields:
                # check if this field is used
                if s.name.lower() not in self.useOnlyFields:
                    continue
            self.addPacketTypes(s)

        if self.getParameterValue("timestamp"):
            # 4-byte timestamp (seconds since system uptime)
            self.packetFields.append(PacketField(
                    PACKET_FIELD_ID_TIMESTAMP, "timestamp", 4, "uint32_t", False))

        if self.getParameterValue("address"):
            # 2-byte address
            self.packetFields.append(PacketField(
                    PACKET_FIELD_ID_ADDRESS, "address", 2, "uint16_t", False))

        self.packetFields = sorted(self.packetFields, key=lambda f: f.sensorID)

    def generateConstants(self, outputFile):
        super(Output, self).generateConstants(outputFile)
        if self.numSensorFields:
            outputFile.write("#define {0}_PACKET_NUM_FIELDS    {1}\n".format(
                    self.getNameUC(), self.numSensorFields))

    def generatePacketType(self, outputFile):
        if len(self.packetFields) == 0: return

        outputFile.write("struct {0}Packet_s {1}\n".format(self.getNameTC(), '{'))

        # --- generate header
        outputFile.write("    uint16_t magic;\n")  # 2-byte magic number
        outputFile.write("    uint16_t crc;\n")    # 2-byte crc (always)
        # type masks
        headerField = 0
        numField = 1
        for f in self.packetFields:
            if f.sensorID >= numField * 32:
                headerField |= 1 << 31
                outputFile.write("    uint32_t typeMask{};\n".format(numField))
                self.headerMasks.append(headerField)
                headerField = 0
                numField += 1
            bit = f.sensorID
            bit -= 32 * (numField - 1)
            headerField |= (1 << bit)
        if headerField:
            outputFile.write("    uint32_t typeMask{};\n".format(numField))
            self.headerMasks.append(headerField)

        # --- generate the body of the packet
        reservedNum = 0
        packetLen = 0
        for f in self.packetFields:
            paddingNeeded = packetLen % f.dataSize
            if paddingNeeded:
                outputFile.write("    uint{}_t __reserved{};\n".format(
                        paddingNeeded * 8, reservedNum))
                packetLen += paddingNeeded
                reservedNum += 1
            outputFile.write("    {} {};\n".format(f.dataType, f.sensorName))
            packetLen += f.dataSize

        # --- finish the packet
        outputFile.write("} PACKED;\n\n")
        # add a typedef
        outputFile.write("typedef struct {0}Packet_s {0}Packet_t;\n\n".format(self.getNameTC()))

    def generateSerialOutputCode(self, outputFile, sensorsUsed):
        usedSizes = set()
        if self.isAggregate():
            for f in self.packetFields:
                usedSizes.add((f.dataSize, f.dataType))
        else:
            for s in sensorsUsed:
                usedSizes.add((s.getDataSize(), s.getDataType()))
        generateSerialFunctions(usedSizes, outputFile)

        if self.isAggregate():
            outputFile.write("static inline void serialPacketPrint(void)\n")
            outputFile.write("{\n")
            outputFile.write("    PRINT(\"======================\\n\");\n")
            for f in self.packetFields:
                outputFile.write("    serialPrint_{0}(\"{1}\", serialPacket.{2});\n".format(
                        f.dataType, toTitleCase(f.sensorName), f.sensorName))
            outputFile.write("}\n\n")


    def generateOutputCode(self, outputFile, sensorsUsed):
        if self.name.lower() == "serial":
            # special code for serial sink
            self.generateSerialOutputCode(outputFile, sensorsUsed)
            if not self.isAggregate():
                return

        outputFile.write("static inline void {0}PacketInit(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    {0}PacketNumFieldsFull = 0;\n".format(self.getNameCC()))
        for f in self.packetFields:
            if f.isRealSensor:
                outputFile.write("    {0}Packet.{1} = {2}_NO_VALUE;\n".format(
                        self.getNameCC(), f.sensorName, f.sensorName.upper()))
        outputFile.write("}\n\n")

        outputFile.write("static inline void {0}PacketSend(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    {0}Packet.magic = SEAL_MAGIC;\n".format(self.getNameCC()))
        for i in range(len(self.headerMasks)):
            outputFile.write("    {0}Packet.typeMask{1} = 0x{2:x};\n".format(
                    self.getNameCC(), i + 1, self.headerMasks[i]))
        if self.getParameterValue("timestamp"):
            outputFile.write("    {0}Packet.timestamp = getUptime();\n".format(self.getNameCC()))
        if self.getParameterValue("address"):
            outputFile.write("    {0}Packet.address = localAddress;\n".format(self.getNameCC()))
            componentRegister.additionalConfig.add("addressing")
        if True:
            outputFile.write("    {0}Packet.crc = crc16((const uint8_t *) &{0}Packet + 4, sizeof({0}Packet) - 4);\n".format(
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

    def generateCallbackCode(self, sensor, outputFile, suffix):
#        if not self.isAggregate():
#            # this must be serial, because all other sinks require packets!
#            outputFile.write("        {0}Print_{1}(\"{2}\", {2}Value);\n".format(
#                    self.getNameCC(),
#                    sensor.getDataType(),
#                    sensor.getNameCC()))
#            return

        # a packet; more complex case
        found = False
        for f in self.packetFields:
            if f.sensorName.lower() == sensor.name.lower():
                found = True
                break
        if not found: return

        outputFile.write("        if ({0}Packet.{1} == {2}_NO_VALUE) {3}\n".format(
                self.getNameCC(), sensor.getNameCC(), sensor.getNameUC(), '{'))
        outputFile.write("            {0}PacketNumFieldsFull++;\n".format(self.getNameCC()))
        outputFile.write("        }\n")

        outputFile.write("        {0}Packet.{1} = {1}Value;\n".format(
                self.getNameCC(), sensor.getNameCC()))

        outputFile.write("        if ({0}PacketIsFull()) {1}\n".format(self.getNameCC(), '{'))
        outputFile.write("            {0}PacketSend();\n".format(self.getNameCC()))
        outputFile.write("        }\n\n")

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

    def addComponent(self, name, spec):
        s = None
        if spec.typeCode == self.module.TYPE_ACTUATOR:
            if name in self.actuators:
                return None
            s = self.actuators[name] = Actuator(name, spec)
        elif spec.typeCode == self.module.TYPE_SENSOR:
            if name in self.sensors:
                return None
            s = self.sensors[name] = Sensor(name, spec)
        elif spec.typeCode == self.module.TYPE_OUTPUT:
            if name in self.outputs:
                return None
            s = self.outputs[name] = Output(name, spec)
        return s

    # load all components for this platform from a file
    def load(self, architecture):
        # reset components
        self.actuators = {}
        self.sensors = {}
        self.outputs = {}
        self.systemParams = []
        self.systemStates = {}
        self.systemConstants = {}
        self.parameterDefines = {}
        self.virtualComponents = {}
        self.patterns = {}
        self.numCachedSensors = 0
        self.additionalConfig = set()
        self.isError = False
        self.architecture = architecture
        # import the module (residing in "components" directory and named "<architecture>.py")
        self.module = __import__(architecture)
        self.nextFreeSensorID = PACKET_FIELD_ID_FIRST_FREE
        # construct empty components from descriptions
        for spec in self.module.components:
            # print "load", spec.name
            c = self.addComponent(spec.name.lower(), spec)
            if not c:
                componentRegister.userError("Component '{0}' duplicated for platform '{1}', ignoring\n".format(
                        spec.name, architecture))

    #######################################################################
    def findComponentByName(self, componentName):
        c = self.sensors.get(componentName, None)
        if c is not None: return c
        c = self.actuators.get(componentName, None)
        if c is not None: return c
        c = self.outputs.get(componentName, None)
        if c is not None: return c
        return None

    def findComponentByKeyword(self, keyword, name):
        if keyword == "use":
            # accept any type of object
            return self.findComponentByName(name)
        if keyword == "read":
            return self.sensors.get(name, None)
        elif keyword == "output":
            return self.outputs.get(name, None)

    def hasComponent(self, keyword, name):
        #print "hasComponent? '" + keyword + "' '" + name + "'"
        return bool(self.findComponentByKeyword(keyword.lower(), name.lower()))

    def allocateSensorId(self):
        result = self.nextFreeSensorID
        self.nextFreeSensorID += 1
        # values 31, 63, 95, 127 are reserved
        if ((self.nextFreeSensorID + 1) & 31) == 0:
            self.nextFreeSensorID += 1
        return result

    #######################################################################
    def addVirtualComponent(self, c):
        if self.virtualComponents.get(c.name, None) is not None:
            self.userError("Virtual component '{0}' duplicated for platform '{1}', ignoring\n".format(
                    c.name, architecture))
            c.isError = True
            return
        if self.findComponentByName(c.name) is not None:
            self.userError("Virtual component '{0}' duplicated with real one for platform '{1}', ignoring\n".format(
                        c.name, self.architecture))
            c.isError = True
            return
        self.virtualComponents[c.name] = c

    def continueAddingVirtualComponent(self, c):
        assert not c.isError
        if c.added: return

        c.added = True
        c.base = None

        basenames = c.getAllBasenames()

        for basename in basenames:
            if c.name == basename:
                self.userError("Virtual component '{0}' depends on self, ignoring\n".format(self.name))
                c.isError = True
                return

        numericalValue = None
        for basename in basenames:
            # constant value
            if basename[:7] == '__const':
                numericalValue = int(basename[7:], 0)
                continue
            # a real sensor
            base = self.findComponentByName(basename)
            if base is not None:
                continue
            # a virtual sensor
            virtualBase = self.virtualComponents.get(basename, None)
            if virtualBase is None:
                self.userError("Virtual component '{0}' has unknown base component '{1}', ignoring\n".format(
                        c.name, basename))
                c.isError = True
                return
            # add the base virtau first (because of parameters)
            self.continueAddingVirtualComponent(virtualBase)

        immediateBaseName = c.getImmediateBasename()
        immediateBase = self.virtualComponents.get(immediateBaseName, None)
        if immediateBase is not None:
            # inherit parameters from parent virtual sensor
            c.parameterDictionary = immediateBase.parameterDictionary
            # inherit the base too
            c.base = immediateBase.base  
        else:
            c.base = self.findComponentByName(immediateBaseName)
        assert c.base

        # fill the parameter dictionary
        for p in c.parameterList:
            if p[1] is not None:
                c.parameterDictionary[p[0]] = p[1]
            else:
                c.parameterDictionary[p[0]] = Value(True)

        if numericalValue is not None:
            c.parameterDictionary["value"] = Value(numericalValue)

    def finishAddingVirtualComponent(self, c):
        assert not c.isError
        assert c.base

        s = self.addComponent(c.name, c.base.specification)
        if not s:
            componentRegister.userError("Virtual component '{0}' duplicated, ignoring\n".format(
                    c.name))
            return

        s.updateParameters(c.parameterDictionary)
        s.functionTree = c.functionTree
        if s.functionTree.function == "sync":
            s.syncOnlySensor = True

    #######################################################################
    def addRemote(self, name):
        base = self.sensors.get(name[6:], None)
        if base is None:
            self.userError("Remote component '{0}' has no base sensor component for architecture '{1}'\n".format(
                    name, self.architecture))
            return None
        o = self.addComponent(name, base.specification)
        o.isRemote = True
        componentRegister.additionalConfig.add("seal_comm")
        return o

    def useComponent(self, keyword, name, parameters, fields, conditions, branchNumber):
        # find the component
        o = self.findComponentByKeyword(keyword, name)
        if o is None:
            # check if this is a remote component
            if name[:6] == "remote":
                o = self.addRemote(name)
                if o == None: return
            # not a remote, treat as error.
            else:
                self.userError("Component '{0}' not known or not supported for architecture '{1}'\n".format(
                        name, self.architecture))
                return
        # check if it can be used
        if type(o) is Output:
            if len(o.useCases):
                self.userError("Output component '{0}': can be used just once in whole program!\n".format(name))
                return
            o.setFields(fields)
        # add use case to the component
        o.addUseCase(parameters, conditions, branchNumber)

    def setState(self, name, value, conditions, branchNumber):
        # add the state itself, if not already
        if name not in self.systemStates:
            self.systemStates[name] = []
            # also add a kind of sensor, to later allow these state values to be read
            if name in self.sensors:
                self.userError("State '{0}' name duplicates a real sensors for architecture '{1}'\n".format(
                        name, self.architecture))
                return
            # create a custom specification for this sensor
            spec = componentRegister.module.SealSensor("Null")
            spec.useFunction.value = name
            spec.readFunction.value = spec.useFunction.value
            spec.dataType = componentRegister.module.SealParameter(value.getType())
            # add the sensor to array
            self.sensors[name] = Sensor(name, spec)
        # add new use case for this state
        self.systemStates[name].append(StateUseCase(name, value, conditions, branchNumber))

    def generateVariables(self, outputFile):
        for s in self.systemStates.itervalues():
            s[0].generateVariables(outputFile)
        for p in self.patterns.itervalues():
            p.generateVariables(outputFile)

    def getAllComponents(self):
        return set(self.actuators.values()).union(set(self.sensors.values())).union(set(self.outputs.values()))

#    def getDependentProcess(self, name):
#        result = []
#        # Find any process statement that is dependent from this component
#        for x in self.process.iteritems():
#            if x[1].target == name.lower():
#                result.append(x)
#                print x[1].name, "depends on", name
#        return result

    def markSyncSensors(self):
        for s in self.sensors.itervalues():
            if s.syncOnlySensor:
                s.addSubsensors()

    def markCachedSensors(self):
        self.numCachedSensors = 0
        for s in self.sensors.itervalues():
            if s.isCacheNeeded(self.numCachedSensors):
                self.numCachedSensors += 1

    def replaceCode(self, componentName, parameterName, condition):
        if type(componentName) is Value:
            return componentName.getCode()

        c = self.findComponentByName(componentName)
#        if c == None:
#            c = self.process.get(componentName, None)
#            if c != None:
#                if parameterName == 'value':
#                    return c.name

        # here

        # not found?
        if c == None:
            # check if this is a remote component
            if componentName[:6] == "remote":
                c = self.addRemote(componentName)
        if c == None:
            if parameterName == 'ispresent':
                return "false"
            else:
                self.userError("Component '{0}' not known\n".format(componentName))
                return "false"

        # in case the component is remote: make condition dependent on it
        if c.isRemote:
            assert condition
            condition.dependentOnSensors.append(c)

        # found
        if parameterName == 'ispresent':
            return "true"
        if parameterName == 'iserror':
            errorFunction = c.getParameterValue("errorFunction", None)
            if errorFunction:
                c.markAsUsed()
                return errorFunction
            else:
                self.userError("Parameter '{0}' for component '{1}' has no error attribute!'\n".format(parameterName, componentName))
                return "false"
        if parameterName == 'value':
            if type(c) is not Sensor:
                self.userError("Parameter '{0}' for component '{1} is not readable!'\n".format(parameterName, componentName))
                return "false"
            # test if cache is needed and if yes, update the flag
            c.isCacheNeededForCondition()
            # otherwise unused component might become usable because of use in condition.
            c.markAsUsedForConditions()
            # return the right read function
            return c.getNameCC() + "ReadProcess(&isFilteredOut)"

#            readFunction = c.getParameterValue("readFunction", None)
#            if readFunction:
#                # otherwise unused component might become usable because of use in condition.
#                c.markAsUsed()
#                return readFunction
#            else:
#                userError("Parameter '{0}' for component '{1} is not readable!'\n".format(parameterName, componentName))
#                return "false"

#        if parameterName in ['average', 'stdev', 'filter']:
#            return "get{}Value(&{}{})".format(toTitleCase(parameterName),
#                                              componentName,
#                                              toTitleCase(parameterName))

        self.userError("Unknown parameter '{0}' for component '{1}'\n".format(parameterName, componentName))
        return "false"

    def userError(self, msg):
        self.isError = True
        # TODO JJ - redirect to IDE
        sys.stderr.write(msg)


######################################################
# global variables
componentRegister = ComponentRegister()
branchCollection = BranchCollection()
conditionCollection = ConditionCollection()
#processStructInits = list()

def clearGlobals():
    global componentRegister
    global branchCollection
    global conditionCollection
    componentRegister = ComponentRegister()
    branchCollection = BranchCollection()
    conditionCollection = ConditionCollection()
