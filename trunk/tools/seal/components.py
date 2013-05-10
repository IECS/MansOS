#
# Copyright (c) 2012 Atis Elsts
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sys, string, copy
from .functions import *

# pre-allocated packet field ID
PACKET_FIELD_ID_COMMAND = 0
PACKET_FIELD_ID_SEQNUM = 1
PACKET_FIELD_ID_TIMESTAMP = 2
PACKET_FIELD_ID_ADDRESS = 3
PACKET_FIELD_ID_IS_SENT = 4
# first free packet field ID (note that ID 31, 63, 95, 127 etc. are reserved for extension)
PACKET_FIELD_ID_FIRST_FREE = 5

commonFields = {
    "command" :        PACKET_FIELD_ID_COMMAND,
    "sequencenumber" : PACKET_FIELD_ID_SEQNUM,
    "timestamp" :      PACKET_FIELD_ID_TIMESTAMP,
    "address" :        PACKET_FIELD_ID_ADDRESS,
    "issent" :         PACKET_FIELD_ID_IS_SENT,
}

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
        outputFile.write("static inline void serialPrintCrc_{0}(const char *name, {0} value)\n".format(dataType))
        outputFile.write("{\n")
        outputFile.write('    uint8_t crc = PRINTF_CRC("%s=%{}", name, value);\n'.format(formatSpecifier))
        outputFile.write('    PRINTF(",%02x\\n", crc);\n')
        outputFile.write("}\n")


def getUseCaseParameterValue(parameter, parameters):
    if parameter not in parameters: return None
    val = parameters[parameter]
    if isinstance(val, Value): return val.value
    return val

# Update existing parameters.
# 'parameters' - params in component format: <name, value>
# 'additionalParametrs' - params in use case format: <name, Value(value)>
# convertToParameterValue() must already have been called before.
def mergeParameters(parameters, additionalParametrs):
    result = parameters
    for p in additionalParametrs.items():
        if p[1] != None:
            result[p[0]] = p[1]
    return result

######################################################
class BranchCollection(object):
    def __init__(self):
        self.branches = {0 : []} # default branch (code 0) is always present
        self.conditions = {0 : []}

    def addBranch(self, n, conditions):
        # if the branch does not exist, it is initialized to empty list
        self.branches.setdefault(n, [])
        self.conditions.setdefault(n, list(conditions))

    def addUseCase(self, branchNumber, useCase):
        self.branches.setdefault(branchNumber, [])
        self.branches[branchNumber].append(useCase)

    def generateCode(self, outputFile):
        for b in self.branches.items():
            self.generateStartCode(b, outputFile)
            self.generateStopCode(b, outputFile)

    def generateLocalFunctions(self, outputFile):
        for n in self.branches:
            outputFile.write("static inline void branch{0}Start(void);\n".format(n))
            if n != 0: outputFile.write("static inline void branch{0}Stop(void);\n".format(n))

    def generateStartCode(self, b, outputFile):
        number = b[0]
        useCases = b[1]
        outputFile.write("static inline void branch{0}Start(void)\n".format(number))
        outputFile.write("{\n")
        for uc in useCases:
            uc.generateBranchEnterCode(outputFile)
        outputFile.write("}\n\n")

    def generateStopCode(self, b, outputFile):
        number = b[0]
        useCases = b[1]
        # the default branch NEVER stops.
        if number == 0: return
        outputFile.write("static inline void branch{0}Stop(void)\n".format(number))
        outputFile.write("{\n")
        for uc in useCases:
            uc.generateBranchExitCode(outputFile)
        outputFile.write("}\n\n")

    # Returns list of numbers [N] of conditions that must be matched to enter this branch
    # * number N > 0: condition Nr. N must be TRUE
    # * number N < 0: condition Nr. abs(N) must be FALSE
    def getConditions(self, branchNumber):
        return self.conditions[branchNumber]

    def getNumBranches(self):
        return len(self.branches)

    def getAssociatedBranches(self, condition):
        result = []
        for i in range(len(self.conditions)):
            for c in self.conditions[i]:
                if abs(c) == condition:
                    result.append(i)
        return result


######################################################
class UseCase(object):
    def __init__(self, component):
        # the parent component
        self.component = component
        # all the parameters of this, specific use case
        self.parameters = {}
        # associated use cases (e.g. "use print, format "%d", light" will case light readings to be generated)
        # self.associatedUseCases = set()

    def setup(self, parameters, conditions, branchNumber, numInBranch):
        self.associatedUseCase = None
        self.parentUseCase = None

        # print "new use case of " + component.name + ": ", parameters
        # use component's parameters as defaults
        for p in self.component.parameters:
            # insert it in parameter dictionary
            self.parameters[p] = self.component.convertToParameterValue(
                self.component.parameters[p], self, unsetAsTrue = False)

        # add user's parameters
        for p in parameters:
            paramName = self.component.specification.resolveAlias(p[0])
            #  print paramName, ":", p[1]

            if paramName == "associate":
                # use associated component here
                compname = p[1].asString().lower()
                comp = componentRegister.findComponentByName(compname)
                if comp is None:
                    componentRegister.userError("Associate parameter specifies unknown component '{}'\n".format(compname))
                else:
                    self.associatedUseCase = comp.addUseCase(
                        {}, conditions = [], branchNumber = 0)
                    self.associatedUseCase.parentUseCase = self
                continue

            if not isinstance(self.component, Output): # outputs are allowed all fields as params
                if paramName not in self.component.parameters:
                    componentRegister.userError("Parameter '{0}' not known for component '{1}'\n".format(
                            paramName, self.component.name))
                    continue

            # insert it in parameter dictionary (can replace component's default!)
            self.parameters[paramName] = p[1]

        self.readFunctionSuffix = ""
        self.conditions = list(conditions)
        self.branchNumber = branchNumber
        if numInBranch == 0:
            self.numInBranch = ''
        else:
            self.numInBranch = "{0}".format(numInBranch)
        #print "add use case, conditions =", self.conditions
        #print "  branchNumber=", self.branchNumber
        self.outputUseCase = None


        # TODO: automate this using reflection!
        p = self.parameters.get("period")
        if p:
            self.period = p.getRawValue()
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
        p = self.parameters.get("times")
        if p and p.value is not None:
            self.times = int(p.value)
        else:
            self.times = None
        p = self.parameters.get("duration")
        if p and p.value is not None:
            self.duration = p.getRawValue()
        else:
            self.duration = None


        p = self.parameters.get("sync")
        if p:
            self.sync = bool(p.value)
        else:
            self.sync = None

        p = self.parameters.get("blink")
        if p and p.value is not None:
            blink = int(p.value)
            if blink:
                self.times = 2
                self.period = blink

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

        p = self.parameters.get("interrupt")
        if p:
            self.interruptBased = bool(p.value)
        else:
            self.interruptBased = False

        if self.interruptBased:
            self.risingEdge = True
            p = self.parameters.get("rising")
            if p: self.risingEdge = bool(p.value)
            else:
                p = self.parameters.get("falling")
                if p: self.risingEdge = not bool(p.value)
            self.port = self.getParameterValueValue("interruptPort")
            if self.port is None: self.port = self.getParameterValueValue("port")
            self.pin = self.getParameterValueValue("interruptPin")
            if self.pin is None: self.pin = self.getParameterValueValue("pin")

        p = self.parameters.get("turnonoff")
        if p and bool(p.value):
            self.onCode = self.getParameterValueValue("onFunction")
            self.offCode = self.getParameterValueValue("offFunction")
            if not self.component.syncOnlySensor and self.onCode is None and self.offCode is None:
                componentRegister.userWarning("turnOnOff parameter specified for component '{0}', but the component has neither 'on' nor 'off' functions\n".format(self.component.name))
        else:
            self.onCode = None
            self.offCode = None

        if (self.pattern and self.once):
            componentRegister.userError("Both 'once' and 'pattern' specified for component '{0}' use case\n".format(self.component.name))
        if (self.times and self.once):
            componentRegister.userError("Both 'once' and 'times' specified for component '{0}' use case\n".format(self.component.name))

        if (self.times and not self.period):
            componentRegister.userError("'times', but not 'period' specified for component '{0}' use case\n".format(self.component.name))
            self.times = None

        if self.component.isRemote() \
                or isinstance(self.component, Output) \
                or self.interruptBased:
            self.generateAlarm = False
        else:
            self.generateAlarm = self.once or self.pattern or self.period

        if branchNumber != 0:
            self.branchName = "Branch{0}".format(branchNumber)
        else:
            self.branchName = ''

        componentRegister.branchCollection.addUseCase(branchNumber, self)

        #print "after conditions =", branchCollection.branches[branchNumber][0][0].conditions
        #print "after conditions[1] =", branchCollection.branches[branchNumber][0][1]

    def getParameterValue(self, parameter, defaultValue = None):
        if parameter in self.parameters:
            return self.parameters[parameter].getCodeForGenerator(componentRegister, None, None)
        return self.component.getParameterValue(parameter, defaultValue)

    def getParameterValueValue(self, parameter, defaultValue = None):
        if parameter in self.parameters:
            return self.parameters[parameter].value
        return self.component.getParameterValue(parameter, defaultValue)

    def generateConstants(self, outputFile):
        ucname = self.component.getNameUC()
        if self.period:
            if self.branchNumber != 0:
                ucname += self.branchName.upper()
            outputFile.write(
                "#define {0}_PERIOD{1}    {2}\n".format(
                    ucname, self.numInBranch, self.period))

    def generateVariables(self, outputFile):
        if self.generateAlarm:
            outputFile.write(
                "Alarm_t {0}{1}Alarm{2};\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))
            if type(self) is Sensor:
                for s in self.component.subsensors:
                    outputFile.write("Alarm_t {0}PreAlarm;\n".format(s.getNameCC()))

    def generateOutCode(self, outputFile):
        p = self.parameters.get("out")
        if p and p.value:
            if typeIsString(p.value):
                outName = p.value
            else:
                outName = p.value.asString()
        else:
            outName = None

        if outName is None: return False

        outComp = componentRegister.findComponentByName(outName)
        if outComp is None:
            componentRegister.userError("Component '{0}': 'out' parameter must point to a valid component!\n".format(outName))
            return True

        if isinstance(outComp, Output):
            outputFile.write("#if USE_PRINT\n")
            # at the moment radio and serial are supported
            if outComp.name not in ('radio', 'serial', 'network'):
                componentRegister.userError("Component '{0}': 'out' parameter should be either Serial, Radio, or Network.\n".format(outName))
                return
            outputFile.write('        debugPrintf({}Print, "%d\\n", {}Value);\n'.format(
                    outComp.name, self.component.getNameCC()))
            outputFile.write("#endif\n")
            return True

        if isinstance(outComp, Sensor):
            componentRegister.userError("Component '{0}': 'out' parameter points to another sensor?\n".format(outName))
            return True

        assert isinstance(outComp, Actuator)
        writeFunction = outComp.getDependentParameterValue("writeFunction", outComp.parameters)
        if writeFunction is None:
            componentRegister.userError("Component '{0}': 'out' parameter points to a component with write function!\n".format(outName))
            return True

        outputFile.write("    {\n")
        outputFile.write("        {0} value = {1}Value;\n".format(
                self.component.getDataType(), self.component.getNameCC()))
        outputFile.write("        {};\n".format(writeFunction))
        outputFile.write("    }\n")
        return True

    def warnIfNone(self, function, functionName):
        if function: return False
        componentRegister.userWarning("No '{1}' parameter for component '{0}'!\n".format(
                self.component.getNameCC(), functionName))
        return True

    # special handling for print statements
    def generatePrintCallbacks(self, outputFile):
        outputFile.write("    bool isFilteredOut;\n")
        outputFile.write("    (void)isFilteredOut;\n")

        argsUsed = set()
        for i in range(100):
            param = self.getParameterValueValue("arg{0}".format(i))
            if param:
                outputFile.write("    isFilteredOut = false;\n".format(i))
                outputFile.write("    int32_t arg{0} = {1};\n".format(i, param))
                # XXX: use 0 to specify "no value"
                outputFile.write("    if (isFilteredOut) arg{0} = 0;\n".format(i))
                argsUsed.add(i)

        formatString = self.getParameterValueValue("format", "")
        if not formatString:
            formatString = '"'
            formatString += '%d ' * len(argsUsed)
        else:
            formatString = formatString.rstrip('"')

        # always append newline
        formatString += '\\n"'

        outputTo = self.getParameterValueValue("out")
        # at the moment radio and serial are supported
        if not outputTo: outputTo = "serial"
        else:
            outputTo = outputTo.lower()
            if outputTo not in ('radio', 'serial', 'network'):
                componentRegister.userError("Component '{0}': 'out' parameter should be either Serial, Radio, or Network.\n".format(outName))
                return

        outputFile.write("#if USE_PRINT\n")       
        outputFile.write("    debugPrintf({}Print, {}".format(outputTo, formatString))
        for i in argsUsed:
            outputFile.write(", arg{0}".format(i))
        outputFile.write(");\n")
        outputFile.write("#endif\n")

    def generateLocalFunctions(self, outputFile):
        ccname = self.component.getNameCC()
        ccname += self.branchName

        if self.generateAlarm or self.interruptBased or self.component.isRemote():
            if not self.interruptBased:
                if self.generateAlarm:
                    argument = "void *isFromBranchStart"
                elif self.component.isRemote():
                    if len(self.component.remoteFields) > 1: argument = "int32_t *buffer"
                    else: argument = "uint16_t code, int32_t value"
                outputFile.write("void {0}{1}Callback({2});\n".format(
                        ccname, self.numInBranch, argument))

    def generateCallbacks(self, outputFile, outputs):
        ccname = self.component.getNameCC()
        ccname += self.branchName
        ucname = self.component.getNameUC()
        ucname += self.branchName.upper()

        if self.generateAlarm or self.interruptBased or self.component.isRemote():
            if self.interruptBased:
                # TODO: multiple sensors can share the same port!
                outputFile.write("ISR(PORT{0}, port{0}Interrupt)\n".format(self.port))
                outputFile.write("{\n")
                outputFile.write("    if (!pinReadIntFlag({0}, {1}))\n".format(self.port, self.pin))
                outputFile.write("        return;\n")
                outputFile.write("    pinClearIntFlag({0}, {1});\n".format(self.port, self.pin))
            else:
                if self.generateAlarm:
                    argument = "void *isFromBranchStart"
                elif self.component.isRemote():
                    if len(self.component.remoteFields) > 1: argument = "int32_t *buffer"
                    else: argument = "uint16_t code, int32_t value"

                outputFile.write("void {0}{1}Callback({2})\n".format(
                        ccname, self.numInBranch, argument))
                outputFile.write("{\n")

            # times limit check
            if self.times:
                outputFile.write("    static uint32_t times;\n")
                outputFile.write("    if (isFromBranchStart) times = 0;\n")
                outputFile.write("    if (++times > {}) return;\n".format(self.times))

            # duration limit check
            if self.duration:
                outputFile.write("    static uint32_t startTime;\n")
                outputFile.write("    if (isFromBranchStart) startTime = getJiffies();\n")
                outputFile.write("    if (timeAfter32((uint32_t)getJiffies(), startTime + {})) return;\n\n".format(
                        self.duration))

            if self.associatedUseCase:
                self.associatedUseCase.generateAssociatedStartCode(outputFile)

            if type(self.component) is Actuator:
                if self.component.name.lower() == "print":
                    self.generatePrintCallbacks(outputFile)
                elif self.on:
                    onFunction = self.component.getDependentParameterValue(
                        "onFunction", self.parameters)
                    if not self.warnIfNone(onFunction, "onFunction"):
                        outputFile.write("    {0};\n".format(onFunction))
                elif self.off:
                    offFunction = self.component.getDependentParameterValue(
                        "offFunction", self.parameters)
                    if not self.warnIfNone(offFunction, "offFunction"):
                        outputFile.write("    {0};\n".format(offFunction))
                else:
                    useFunction = self.component.getDependentParameterValue(
                        "useFunction", self.parameters)
                    if not self.warnIfNone(useFunction, "useFunction"):
                        outputFile.write("    {0};\n".format(useFunction))

            elif type(self.component) is Sensor:
                intTypeName = self.component.getDataType()

                if self.component.syncOnlySensor:
                    # TODO: generate on/off code for base sensors here?
                    p = self.parameters.get("turnonoff")
                    generateOnOffCode = p and bool(p.value)
                    if generateOnOffCode:
                        for s in self.component.subsensors:
                            onFunc = s.getParameterValue("onFunction")
                            if onFunc:
                                outputFile.write("    {};\n".format(onFunc))
                    if self.period:
                        for s in self.component.subsensors:
                            if s.getParameterValue("preReadFunction") is None: continue
                            preReadTime = s.specification._readTime
                            if preReadTime == 0: continue
                            outputFile.write("    alarmSchedule(&{0}PreAlarm, {2}_PERIOD{1} - {3});\n".format(
                                    s.getNameCC(), self.numInBranch, ucname, preReadTime))
                    outputFile.write("    bool isFilteredOut = false;\n")
                    outputFile.write("    {0}Value = {0}ReadProcess{1}(&isFilteredOut);\n".format(
                            self.component.getNameCC(), self.readFunctionSuffix))

#                    outputFile.write("    {0} {1}Value = {2}ReadProcess{3}(&isFilteredOut);\n".format(
#                            intTypeName, self.component.getNameCC(),
#                            self.component.getNameCC(), self.readFunctionSuffix))
#                    outputFile.write("    (void){0}Value;\n".format(
#                            self.component.getNameCC()))

                    if generateOnOffCode:
                        for s in self.component.subsensors:
                            offFunc = s.getParameterValue("offFunction")
                            if offFunc:
                                outputFile.write("    {};\n".format(offFunc))
                elif self.component.isRemote():
                    prefix = self.component.networkComponent.getPrefix()
                    if len(self.component.remoteFields) > 1:
                        # values must be read from memory using pointer to a buffer
                        outputFile.write("    {0} *read = ({0} *)buffer;\n".format(intTypeName))
                        for f in self.component.remoteFields:
                            outputFile.write("    {\n")
                            outputFile.write("        {0} {1}Value = *read;\n".format(
                                    intTypeName, prefix + toCamelCase(f)))
                            if self.generateOutCode(outputFile):
                                # 'out' parameter specified; ignore the regular outputs in this case
                                pass
                            else:
                                for o in outputs:
                                    o.generateCallbackCode(prefix + f, outputFile, self.readFunctionSuffix)
                            outputFile.write("    }\n")
                            outputFile.write("    read++;\n")
                    else:
                        # value is passed as argument
                        fieldName = prefix + self.component.remoteFields[0]
                        outputFile.write("    {0}Value = value;\n".format(fieldName))
#                        outputFile.write("    (void){0}Value;\n".format(fieldName))
                        for o in outputs:
                            o.generateCallbackCode(fieldName, outputFile, self.readFunctionSuffix)
                        if self.generateOutCode(outputFile):
                            # 'out' parameter specified; ignore the regular outputs in this case
                            pass
                        else:
                            for o in outputs:
                                o.generateCallbackCode(fieldName, outputFile, self.readFunctionSuffix)
                else:
                    if self.onCode: outputFile.write("    {};\n".format(self.onCode))
                    outputFile.write("    bool isFilteredOut = false;\n")
                    outputFile.write("    {0}Value = {0}ReadProcess{1}(&isFilteredOut);\n".format(
                            self.component.getNameCC(), self.readFunctionSuffix))
#                    outputFile.write("    {0} {1}Value = {2}ReadProcess{3}(&isFilteredOut);\n".format(
#                            intTypeName, self.component.getNameCC(),
#                            self.component.getNameCC(), self.readFunctionSuffix))
#                    outputFile.write("    (void){0}Value;\n".format(
#                            self.component.getNameCC()))
                    outputFile.write("    if (!isFilteredOut) {\n")
                    if self.generateOutCode(outputFile):
                        # 'out' parameter specified; ignore the regular outputs in this case
                        pass
                    else:
                        for o in outputs:
                            o.generateCallbackCode(self.component.name, outputFile, self.readFunctionSuffix)
                        conditionCollection.onSensorRead(outputFile, self.component.getNameCC())
                    outputFile.write("    }\n")
                    if self.offCode: outputFile.write("    {};\n".format(self.offCode))

            if self.component.isRemote() or self.interruptBased:
                pass
            elif self.once:
                pass
            elif self.period:
                if self.sync:
                    outputFile.write("    uint64_t nextTime = getSyncTimeMs64() + {0}_PERIOD{1};\n".format(
                            ucname, self.numInBranch))
                    outputFile.write("    nextTime -= nextTime % {0}_PERIOD{1};\n".format(
                            ucname, self.numInBranch))
                    outputFile.write("    alarmSchedule(&{0}Alarm{1}, (uint32_t)(nextTime - getSyncTimeMs64()));\n".format(
                            ccname, self.numInBranch))
                else:
                    outputFile.write("    alarmSchedule(&{0}Alarm{1}, {2}_PERIOD{1});\n".format(
                            ccname, self.numInBranch, ucname))
            elif self.pattern:
                outputFile.write("    alarmSchedule(&{0}Alarm{1}, pattern_{2}[pattern_{2}Cursor]);\n".format(
                        ccname, self.numInBranch, self.pattern))
                outputFile.write("    pattern_{0}Cursor++;\n".format(self.pattern))
                outputFile.write("    pattern_{0}Cursor %= sizeof(pattern_{0}) / sizeof(*pattern_{0});\n".format(
                        self.pattern))
            outputFile.write("}\n\n")

    def generateAppMainCode(self, outputFile):
        ccname = self.component.getNameCC()
        ccname += self.branchName
        if self.generateAlarm:
            outputFile.write("    alarmInit(&{0}Alarm{1}, {0}{1}Callback, NULL);\n".format(
                   ccname, self.numInBranch))
            if type(self) is Sensor:
                for s in self.component.subsensors:
                    outputFile.write("    alarmInit(&{0}PreAlarm, {0}PreReadCallback, NULL);\n".format(s.getNameCC()))
        elif self.component.isRemote():
            if len(self.component.remoteFields) > 1:
                outputFile.write("    {\n")
                outputFile.write("        static int32_t buffer[{}];\n".format(len(self.component.remoteFields)))
                outputFile.write("        const uint32_t typeMask = 0")
                for f in self.component.remoteFields:
                    outputFile.write("\n            | {}_TYPE_MASK".format(f.upper()))
                outputFile.write(";\n")
                outputFile.write("        sealNetPacketRegisterInterest(typeMask, {}{}Callback, buffer);\n".format(
                        self.component.getNameCC(), self.numInBranch))
                outputFile.write("    }\n")
            else:
                # TODO: do not generate any code here or above if this is not used
                typeID = self.component.remoteFields[0].upper() + "_TYPE_ID"
                outputFile.write("    sealNetRegisterInterest({}, {}{}Callback);\n".format(
                        typeID, self.component.getNameCC(), self.numInBranch))
        elif self.interruptBased:
            outputFile.write("    pinEnableInt({}, {});\n".format(self.port, self.pin))
            if self.risingEdge: outputFile.write("    pinIntRising({}, {});\n".format(self.port, self.pin))
            else: outputFile.write("    pinIntFalling({}, {});\n".format(self.port, self.pin))

        if self.component.specification._name == "DigitalOut":
            port = self.getParameterValueValue("port")
            pin = self.getParameterValueValue("pin")
            if port is not None and pin is not None:
                outputFile.write("    pinAsOutput({}, {});\n".format(port, pin))

    def generateBranchEnterCode(self, outputFile):
        # if this UC has parent, the parent will generate first call instead
        if self.parentUseCase: return

        if self.generateAlarm:
            if isinstance(self.component, Output):
                outputFile.write("    {0}{1}{2}Process();\n".format(
                        self.component.getNameCC(), self.branchName, self.numInBranch))
            else:
                outputFile.write("    {0}{1}{2}Callback(IS_FROM_BRANCH_START);\n".format(
                        self.component.getNameCC(), self.branchName, self.numInBranch))

    def generateBranchExitCode(self, outputFile):
        # should be able to execute this code even when this UC has a parent;
        # but all use cases with parent are in branch 0 anyway.

        if type(self.component) is not Output and self.generateAlarm:
            outputFile.write("    alarmRemove(&{0}{1}Alarm{2});\n".format(
                    self.component.getNameCC(), self.branchName, self.numInBranch))
            if self.pattern:
                # reset cursor position (TODO XXX: really?)
                outputFile.write("    pattern_{0}Cursor = 0;\n".format(self.pattern))

    def generateAssociatedStartCode(self, outputFile):
        assert self.parentUseCase
        if not self.generateAlarm: return
        if isinstance(self.component, Output): return
        outputFile.write("    {0}{1}{2}Callback(IS_FROM_BRANCH_START);\n".format(
                        self.component.getNameCC(), self.branchName, self.numInBranch))


######################################################
class Component(object):
    def __init__(self, name, specification):
        self.name = name
        # create dictionary for parameters
        self.parameters = {}
        for p in dir(specification):
            if isinstance(specification.__getattribute__(p), componentRegister.module.SealParameter):
                self.parameters[p] = specification.__getattribute__(p).value
        self.useCases = []
        self.markedAsUsed = False
        self.usedForNumberOfConditions = 0
        # save specification (needed for dependent parameters)
        self.specification = specification
        self.functionTree = None
        self.networkComponent = None
        self.conditionsDependentOnInterrupt = []

    def isRemote(self):
        return self.networkComponent is not None

    def isVirtual(self):
        return self.functionTree is not None

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
        return self.name[0].lower() + self.name[1:]

    def getNameTC(self):
        assert self.name != ''
        return self.name[0].upper() + self.name[1:]

    # this is needed to allow to write for example "...,identifier variables.localAddress, ..."
    def convertToParameterValue(self, pvalue, useCase, unsetAsTrue):
        if unsetAsTrue and pvalue is None:
            # Update parameters with user's value, if given. If no value given, only name: treat it as 'True',
            # because value 'None' means that the parameter is supported, but not specified by the user.
            return Value(True)
        if not isinstance (pvalue, Value):
            return Value(pvalue)
        if isinstance(pvalue.value, SealValue):
            return Value(pvalue.getCodeForGenerator(componentRegister, None, useCase))
        return pvalue
#        if pvalue is not None:
#            if not isinstance(pvalue, Expression): # filter out "where ..." conditions
#                if isinstance(pvalue.value, SealValue):
#                    return Value(pvalue.getCodeForGenerator(componentRegister, None, useCase))
#        return Value(pvalue)

    def updateParameters(self, dictionary):
        for p in dictionary.items():
            pvalue = self.convertToParameterValue(p[1], useCase = True, unsetAsTrue = True)
            self.parameters[p[0]] = pvalue

    def getParameterValue(self, parameter, defaultValue = None):
        if parameter in self.parameters:
            value = self.parameters[parameter]
            if value is not None: return value
        return defaultValue

    def getDependentParameterValue(self, parameter, useCaseParameters):
        if parameter not in self.parameters:
            return None
        return self.specification.calculateParameterValue(
            parameter, useCaseParameters)

    def getConfig(self):
        if not self.isUsed(): return ""
        result = ""
        if isinstance(self, Output) and len(self.useCases):
            for uc in self.useCases:
                params = self.useCases[0].parameters
                r = self.getDependentParameterValue("extraConfig", params)
                if r: result += r
        else:
            params = self.parameters
            r = self.getDependentParameterValue("extraConfig", params)
            if r: result += r
        return result

    def addUseCase(self, parameters, conditions, branchNumber):
        numInBranch = 0
        for uc in self.useCases:
            if uc.branchNumber == branchNumber:
                numInBranch += 1
        finalParameters = {}
        associatedUseCase = None
        uc = UseCase(self)
        for p in parameters.items():
            # resolve "parameters" parametes (should point to a define)
            pname = p[0].lower()
            pvalue = p[1]
            if pname == "parameters":
                #print "componentRegister = ", componentRegister.defines.keys()
                d = componentRegister.parameterDefines.get(pvalue.lower())
                if d is None:
                    componentRegister.userError("No parameter define with name '{0}' is present (for component '{1}')\n".format(
                            pvalue.asString(), self.name))
                else:
                    for pd in d.parameters:
                        if pd[0] in finalParameters:
                            componentRegister.userError("Parameter '{0}' already specified for component '{1}'\n".format(pd[0], self.name))
                        else:
                            finalParameters[pd[0]] = self.convertToParameterValue(pd[1], uc, unsetAsTrue = True)
            else:
                if pname in finalParameters:
                    componentRegister.userError("Parameter '{0}' already specified for component '{1}'\n".format(p[0], self.name))
                else:
                    finalParameters[pname] = self.convertToParameterValue(pvalue, uc, unsetAsTrue = True)

        uc.setup(finalParameters.items(), conditions, branchNumber, numInBranch)
        self.useCases.append(uc)
        return uc

    # XXX: replacement for "getParamValue"
    def getSpecialValue(self, parameter):
        if parameter in self.parameters:
            value = self.parameters[parameter]
            if value is not None: return value
        return None

    def isInterruptBased(self):
        if not self.getParameterValue("interrupt", False): return

        # TODO: optimize
        risingEdge = self.getParameterValue("rising", False)
        fallingEdge = self.getParameterValue("falling", False)
        if not risingEdge and not fallingEdge:
            risingEdge = True
        port = self.getParameterValue("interruptPort")
        if port is None: port = self.getParameterValue("port")
        pin = self.getParameterValue("interruptPin")
        if pin is None: pin = self.getParameterValue("pin")

        self.intPin = pin
        self.intPort = port
        self.risingEdge = risingEdge
        return True

    def isNetworkBased(self):
        return False # only for sensors

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

    def generateLocalFunctions(self, outputFile):
        for uc in self.useCases:
            uc.generateLocalFunctions(outputFile)

    def prepareToGenerateCallbacks(self, outputFile):
        # for outputs only
        for o in self.outputUseCases:
            if isinstance(o, FromFileOutputUseCase):
                o.prepareToGenerateCallbacks(outputFile)

    def generateCallbacks(self, outputFile, outputs):
        if isinstance(self, Output):
            for o in self.outputUseCases:
                if isinstance(o, FromFileOutputUseCase):
                    o.generateCallbacks(outputFile)
            return

        if isinstance(self, Sensor):
#            print self.specification.readFunctionDependsOnParams
#            print self.useCases
#            print self.usedForConditions
#            print "gen callbacks for ", self.name
            if self.specification._readFunctionDependsOnParams:
                # print "depends on spec, useCases:", self.useCases
                for uc in self.useCases:
                    # print "params: ", uc.parameters
                    self.generateReadFunctions(outputFile, uc)
                if self.markedAsUsed:
                    self.generateReadFunctions(outputFile, None)
            else:
                # print "NOT depends on spec"
                self.generateReadFunctions(outputFile, None)

            if self.doGenerateSyncCallback:
                self.generateSyncCallback(outputFile, outputs)

            for s in self.subsensors:
                s.generatePrereadCallback(outputFile)

        for uc in self.useCases:
            uc.generateCallbacks(outputFile, outputs)

        if self.isInterruptBased() and len(self.conditionsDependentOnInterrupt):
            outputFile.write("\n")
            outputFile.write("ISR(PORT{0}, port{0}Interrupt)\n".format(self.intPort))
            outputFile.write("{\n")
            outputFile.write("    if (!pinReadIntFlag({0}, {1}))\n".format(self.intPort, self.intPin))
            outputFile.write("        return;\n")
            outputFile.write("    pinClearIntFlag({0}, {1});\n".format(self.intPort, self.intPin))
            for c in self.conditionsDependentOnInterrupt:
                outputFile.write("    condition{}Callback();\n".format(c.id))
            outputFile.write("}\n")

    def generateAppMainCode(self, outputFile):
        for uc in self.useCases:
            uc.generateAppMainCode(outputFile)

#        if len(self.useCases) == 0 and self.usedForConditions:
        if len(self.useCases) == 0:
            if self.specification._name == "DigitalIn" and self.isInterruptBased() \
                    and len(self.conditionsDependentOnInterrupt):
                outputFile.write("    pinEnableInt({}, {});\n".format(self.intPort, self.intPin))
                if self.risingEdge: outputFile.write("    pinIntRising({}, {});\n".format(self.intPort, self.intPin))
                else: outputFile.write("    pinIntFalling({}, {});\n".format(self.intPort, self.intPin))

#            if self.specification._name == "DigitalOut":
#                port = self.getParameterValueValue("port")
#                pin = self.getParameterValueValue("pin")
#                if port is not None and pin is not None:
#                    outputFile.write("    pinAsOutput({}, {});\n".format(port, pin))


######################################################
class Actuator(Component):
    def __init__(self, name, specification):
        super(Actuator, self).__init__(name, specification)

######################################################
class Sensor(Component):
    def __init__(self, name, specification):
        super(Sensor, self).__init__(name, specification)
        if self.specification._minUpdatePeriod is None:
            self.minUpdatePeriod = 1000 # default value
        else:
            self.minUpdatePeriod = self.specification._minUpdatePeriod
        self.cacheNeeded = False
        cnParam = self.getParameterValue("cache")
        if cnParam is not None: self.cacheNeeded = cnParam
        self.cacheNumber = 0
        self.readFunctionNum = 0
        self.systemwideID = None
        self.alsoSensorIds = set()
        self.sensorReadFunctionParams = None
#        if name.lower() == "command":
#            self.systemwideID = PACKET_FIELD_ID_COMMAND
#        else:
#            self.systemwideID = componentRegister.allocateSensorId()
#            if self.systemwideID >= 31:
#                componentRegister.userError("Too many sensors! Sensor '{}' has id {}, but at the moment only ID up to 30 are supported.\n".format(name, self.systemwideID))

        self.doGenerateSyncCallback = False
        self.syncOnlySensor = False
        self.subsensors = []
        self.remoteFields = []
        self.containingOutputComponent = None
        self.generatedDefaultRawReadFunction = False

    def getSystemwideID(self):
        if self.systemwideID is not None: return self.systemwideID
        if self.name.lower() in commonFields:
            self.systemwideID = commonFields[self.name.lower()]
        else:
            self.systemwideID = componentRegister.allocateSensorID(self.name)
            if self.systemwideID >= 31:
                componentRegister.userError("Too many sensors! Sensor '{}' has id {}, but at the moment only ID up to 30 are supported.\n".format(name, self.systemwideID))
        return self.systemwideID

    def updateParameters(self, dictionary):
        super(Sensor, self).updateParameters(dictionary)
        cnParam = self.getParameterValue("cache")
        if cnParam is not None: self.cacheNeeded = cnParam

    def getDataSize(self):
        return self.specification._dataSize

    def getDataType(self):
        return self.specification._dataType

    def getMaxValue(self):
        # return "0x" + "ff" * self.getDataSize()
        return "LONG_MAX"

    def getMinValue(self):
        # return "0"
        return "LONG_MIN"

    def isNetworkBased(self):
        if self.functionTree is None: return False
        allSensors = self.functionTree.collectSensors()
        for name in allSensors:
            if name in componentRegister.networkComponents:
                return True
        return False

    def generateLocalFunctions(self, outputFile):
        super(Sensor, self).generateLocalFunctions(outputFile)
        if self.isUsed():
            outputFile.write("inline {} {}ReadRaw(bool *);\n".format(
                    self.getDataType(), self.getNameCC()))
        for s in self.subsensors:
            outputFile.write("static void {}SyncCallback(void);\n".format(s.getNameCC()))
        for c in self.conditionsDependentOnInterrupt:
            outputFile.write("static void condition{}Callback(void);\n".format(c.id))

    def generateConstants(self, outputFile):
        super(Sensor, self).generateConstants(outputFile)
        if self.isUsed() or self.networkComponent:
            outputFile.write("#define {0}_TYPE_ID     {1:}\n".format(
                    self.getNameUC(), self.getSystemwideID()))
            if self.name.lower() not in commonFields:
                # TODO FIXME: also all copies
                if len(self.alsoSensorIds) == 0:
                    mask = 1 << self.getSystemwideID()
                else:
                    mask = 0
                    for id in self.alsoSensorIds:
                        mask |= 1 << id
                outputFile.write("#define {0}_TYPE_MASK   {1:#x}\n".format(self.getNameUC(), mask))

    def generateVariables(self, outputFile):
        super(Sensor, self).generateVariables(outputFile)
        if self.isUsed():
            outputFile.write("static {} {}Value;\n".format(self.getDataType(), self.getNameCC()))

    def testIsCacheNeeded(self, numCachedSensors):
        if not self.specification._cacheable: return False
        if self.cacheNeeded: return True

        cnParam = self.getParameterValue("cache")
        if cnParam is not None: return cnParam

        for uc in self.useCases:
            if uc.period is not None and uc.period < self.minUpdatePeriod:
                self.cacheNeeded = True
                self.cacheNumber = numCachedSensors
                break

        return self.cacheNeeded

    def testIsCacheNeededForCondition(self):
        self.usedForNumberOfConditions += 1

        if not self.specification._cacheable: return False

        if self.cacheNeeded: return True

        cnParam = self.getParameterValue("cache")
        if cnParam is not None: return cnParam

        # once in second...
        conditionEvaluatePeriod = 1000
        # ..divided by the number of conditions this sensor is used for
        conditionEvaluatePeriod /= self.usedForNumberOfConditions

        if conditionEvaluatePeriod < self.minUpdatePeriod:
            self.cacheNeeded = True
            self.cacheNumber = componentRegister.numCachedSensors
            componentRegister.numCachedSensors += 1

        return self.cacheNeeded

    def generateSyncCallback(self, outputFile, outputs):
        useFunction = self.getDependentParameterValue("useFunction", self.parameters)

        outputFile.write("static void {0}SyncCallback(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    bool isFilteredOut = false;\n")
        outputFile.write("    {0}Value = {0}ReadProcess(&isFilteredOut);\n".format(
                self.getNameCC()))
#        outputFile.write("    {0} {1}Value = {1}ReadProcess(&isFilteredOut);\n".format(
#                self.getDataType(), self.getNameCC()))
#        outputFile.write("    (void){0}Value;\n".format(self.getNameCC()))
        outputFile.write("    if (!isFilteredOut) {\n")
        for o in outputs:
            o.generateCallbackCode(self.name, outputFile, "")
        # XXX: generateOutCode ?
        outputFile.write("    }\n")
        outputFile.write("}\n")

    def generatePrereadCallback(self, outputFile):
        func = self.getParameterValue("preReadFunction")
        if func is None: return

        outputFile.write("void {0}PreReadCallback(void *__unused)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    " + func + ";\n")
        outputFile.write("}\n\n")

    def addSubsensors(self):
        for a in self.functionTree.arguments:
            sensor = componentRegister.findComponentByName(a.generateSensorName())
            if not sensor: continue
            assert type(sensor) is Sensor
            sensor.doGenerateSyncCallback = True
            sensor.markAsUsed()
            self.subsensors.append(sensor)

    #########################################################################
    # Start of reading and processing function generation
    #########################################################################

    def getRawReadFunction(self, suffix, root):
        if self.isRemote():
            # remote sensors require specialHandling
            return "sealNetReadValue({}_TYPE_ID)".format(self.getNameUC())

        rawReadFunc = "{}ReadRaw{}".format(self.getNameCC(), suffix)
        if self.cacheNeeded:
            dataFormat = str(self.getDataSize() * 8)
            return "cacheReadSensor{0}({1}, &{2}, {3}, NULL)".format(
                dataFormat, self.cacheNumber, rawReadFunc, self.minUpdatePeriod)
        return rawReadFunc + "(NULL)"

    def getGeneratedFunctionName(self, fun):
        readFunctionSuffix = str(self.readFunctionNum)
        self.readFunctionNum += 1
        return self.getNameCC() + "Read" + toTitleCase(fun) + readFunctionSuffix

    def generateAbsFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("abs")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return value < 0 ? -value : value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateNegFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("neg")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = -{1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateMapFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("map")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return map(value, {0}, {1}, {2}, {3});\n".format(
                functionTree.arguments[1].asString(), functionTree.arguments[2].asString(),
                functionTree.arguments[3].asString(), functionTree.arguments[4].asString()))
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateUnaryMinFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

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
            outputFile, functionTree.arguments[0], root, root)

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
            subReadFunctions.append(self.generateSubReadFunctions(outputFile, a, root))

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
            subReadFunctions.append(self.generateSubReadFunctions(outputFile, a, root))
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
            if functionTree.arguments[0].function == "tuple":
                return self.generateTupleFunction(outputFile, functionTree.arguments[0], "min")
            return self.generateUnaryMinFunction(outputFile, functionTree, root)
        return self.generateNaryMinFunction(outputFile, functionTree, root)

    def generateMaxFunction(self, outputFile, functionTree, root):
        if len(functionTree.arguments) == 1:
            return self.generateUnaryMaxFunction(outputFile, functionTree, root)
            if functionTree.arguments[0].function == "take":
                return self.generateTakeFunction(outputFile, functionTree.arguments[0], "max")
            if functionTree.arguments[0].function == "tuple":
                return self.generateTupleFunction(outputFile, functionTree.arguments[0], "max")
        return self.generateNaryMaxFunction(outputFile, functionTree, root)

    def generateSquareFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("square")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1} * {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateSqrtFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        componentRegister.additionalConfig.add("algo")

        funName = self.getGeneratedFunctionName("sqrt")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = intSqrt({1});\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"


    def generateAvgFunction(self, outputFile, functionTree, root):
        if functionTree.arguments[0].function == "take":
            return self.generateTakeFunction(outputFile, functionTree.arguments[0], "avg")
        if functionTree.arguments[0].function == "tuple":
            return self.generateTupleFunction(outputFile, functionTree.arguments[0], "avg")
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

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

    # TODO: test this
    # TODO: allow alpha as optional
    def generateEWMAFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        alpha = functionTree.arguments[1].asConstant()
        if alpha is None:
            componentRegister.userError("2nd argument of EWMA() function is expected to be a constant!\n")
            return ""

        # TODO: improve this
        numerator = int(100 * alpha)
        denominator = 100

        funName = self.getGeneratedFunctionName("EWMA")
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    static {0} ewmaValue;\n".format(self.getDataType()))
        outputFile.write("    bool b = false, *isFilteredOut = &b;\n")
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        # S_t = Y_t * alpha + S_{t-1} * (1 - alpha)
        outputFile.write("    if (!*isFilteredOut) {\n")
        outputFile.write("        ewmaValue = value * {} / {} + ewmaValue * {} / {};\n".format(
                numerator, denominator, denominator - numerator, denominator))
        outputFile.write("    }\n")
        outputFile.write("    return ewmaValue;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    #TODO: variance function!

    def generateStdevFunction(self, outputFile, functionTree, root):
        if functionTree.arguments[0].function == "take":
            return self.generateTakeFunction(outputFile, functionTree.arguments[0], "stdev")
        if functionTree.arguments[0].function == "tuple":
            return self.generateTupleFunction(outputFile, functionTree.arguments[0], "stdev")
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

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

    # TODO: 
    # first arg: sensor
    # second arg: window size WinSize
    # third ard: strength
    def generateSmoothenFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        numSamples = None
        adjustmentWeight = None

        if len(functionTree.arguments) > 1:
            numSamples = functionTree.arguments[1].asConstant()
        if len(functionTree.arguments) > 2:
            adjustmentWeight = functionTree.arguments[2].asConstant()

        if numSamples == None: numSamples = 3
        if adjustmentWeight == None: adjustmentWeight = 1
        adjustmentWeight = int(adjustmentWeight * numSamples)  # scale it
        medianPos = numSamples / 2

        funName = self.getGeneratedFunctionName("smoothen")
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    enum {} ARRAY_SIZE = {} {};\n".format('{', numSamples, '}'))
        outputFile.write("    static {0} array[ARRAY_SIZE];\n".format(self.getDataType()))
        outputFile.write("    static uint16_t arrayCursor = {};\n".format(numSamples - 1))
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        # outputFile.write('    PRINTF("got %ld\\n", value);\n')
        outputFile.write("    array[arrayCursor] = value;\n")
        outputFile.write("    {0} median = array[(arrayCursor + ARRAY_SIZE - {1}) % ARRAY_SIZE];\n".format(
                self.getDataType(), medianPos))
        #outputFile.write('    PRINTF("median = %ld\\n", median);\n')
        outputFile.write("    int32_t adjustment = 0;\n")
        outputFile.write("    uint16_t i;\n")
        outputFile.write("    for (i = 1; i <= ARRAY_SIZE; ++i) {\n")
        outputFile.write("        adjustment += array[(arrayCursor + i) % ARRAY_SIZE] - median;\n")
        # outputFile.write('        PRINTF("array[%u] = %ld\\n", (arrayCursor + i) % ARRAY_SIZE, array[(arrayCursor + i) % ARRAY_SIZE]);\n')
        outputFile.write("    }\n")
        # outputFile.write('    PRINTF("adjustment = %ld\\n", adjustment);\n')
        outputFile.write("    arrayCursor = (arrayCursor + 1) % ARRAY_SIZE;\n")
        outputFile.write("    return median + adjustment * {} / {} / {};\n".format(
                adjustmentWeight, numSamples, numSamples))
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    # TODO: join this with smoothen
    def generateSharpenFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        numSamples = None
        adjustmentWeight = None

        if len(functionTree.arguments) > 1:
            numSamples = functionTree.arguments[1].asConstant()
        if len(functionTree.arguments) > 2:
            adjustmentWeight = functionTree.arguments[2].asConstant()

        if numSamples == None: numSamples = 3
        if adjustmentWeight == None: adjustmentWeight = 1
        adjustmentWeight = int(adjustmentWeight * numSamples)  # scale it
        medianPos = numSamples / 2

        funName = self.getGeneratedFunctionName("sharpen")
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    enum {} ARRAY_SIZE = {} {};\n".format('{', numSamples, '}'))
        outputFile.write("    static {0} array[ARRAY_SIZE];\n".format(self.getDataType()))
        outputFile.write("    static uint16_t arrayCursor = {};\n".format(numSamples - 1))
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        #outputFile.write('    PRINTF("got %ld\\n", value);\n')
        outputFile.write("    array[arrayCursor] = value;\n")
        outputFile.write("    {0} median = array[(arrayCursor + ARRAY_SIZE - {1}) % ARRAY_SIZE];\n".format(
                self.getDataType(), medianPos))
        # outputFile.write('    PRINTF("median = %ld\\n", median);\n')
        outputFile.write("    int32_t adjustment = 0;\n")
        outputFile.write("    uint16_t i;\n")
        outputFile.write("    for (i = 1; i <= ARRAY_SIZE; ++i) {\n")
        outputFile.write("        adjustment += array[(arrayCursor + i) % ARRAY_SIZE] - median;\n")
        #outputFile.write('        PRINTF("array[%u] = %ld\\n", (arrayCursor + i) % ARRAY_SIZE, array[(arrayCursor + i) % ARRAY_SIZE]);\n')
        outputFile.write("    }\n")
        #outputFile.write('    PRINTF("adjustment = %ld\\n", adjustment);\n')
        outputFile.write("    arrayCursor = (arrayCursor + 1) % ARRAY_SIZE;\n")
        outputFile.write("    return median - adjustment * {} / {} / {};\n".format(
                adjustmentWeight, numSamples, numSamples))
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    # TODO: allow arg as optional?
    def generateChangedFunction(self, outputFile, functionTree, root):
        # TODO: allow take()!

        milliseconds = functionTree.arguments[1].asConstant()
        if milliseconds is None:
            componentRegister.userError("Second argument of changed() function is expected to be a constant!\n")
            return ""

        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("changed")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    static {0} lastValue;\n".format(self.getDataType()))
        #outputFile.write("    static ticks_t changedIntervalBound = {};\n".format(milliseconds))
        outputFile.write("    static ticks_t changedIntervalBound;\n")
        outputFile.write("    {0} value = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    if (value == lastValue) {\n")
        outputFile.write("        // return true if changed during the interval\n")
        outputFile.write("        return timeAfter(changedIntervalBound, getJiffies());\n")
        outputFile.write("    }\n")
        outputFile.write("    lastValue = value;\n")
        outputFile.write("    changedIntervalBound = getJiffies() + {};\n".format(milliseconds))
        outputFile.write("    // return true even if the interval is zero\n")
        outputFile.write("    return true; \n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateSumFunction(self, outputFile, functionTree, root):
        if len(functionTree.arguments) == 1:
            if functionTree.arguments[0].function == "take":
                return self.generateTakeFunction(outputFile, functionTree.arguments[0], "sum")
            if functionTree.arguments[0].function == "tuple":
                return self.generateTupleFunction(outputFile, functionTree.arguments[0], "sum")
        subReadFunctions = []
        for a in functionTree.arguments:
            subReadFunctions.append(self.generateSubReadFunctions(outputFile, a, root))

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
        subReadFunction1 = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)
        subReadFunction2 = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[1], root)

        funName = self.getGeneratedFunctionName(functionTree.function)
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1} {2} {3};\n".format(
                self.getDataType(), subReadFunction1, op, subReadFunction2))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateSymmetricDiffFunction(self, outputFile, functionTree, root):
        subReadFunction1 = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)
        subReadFunction2 = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[1], root)

        funName = self.getGeneratedFunctionName(functionTree.function)
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value1 = {1};\n".format(
                self.getDataType(), subReadFunction1))
        outputFile.write("    {0} value2 = {1};\n".format(
                self.getDataType(), subReadFunction2))
        outputFile.write("    {0} value = value1 > value2 ? value1 - value2 : value2 - value1;\n".format(
                self.getDataType()))
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generatePowerFunction(self, outputFile, functionTree, root):
        power = functionTree.arguments[1].asConstant()
        if power is None:
            componentRegister.userError("Second argument of power() function is expected to be a constant!\n")
            return ""

        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

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

    def generateMatchFunction(self, outputFile, functionTree, root):
        patternName = functionTree.arguments[1].asString()
        pattern = componentRegister.patterns.get(patternName)
        if pattern is None:
            componentRegister.userError("Second argument of match() function must be a pattern name!\n")
            return ""

        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        numToTake = pattern.getSize()

        funName = self.getGeneratedFunctionName("match")
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
        outputFile.write("    uint16_t i;\n")
        outputFile.write("    for (i = 0; i < {}; ++i) {}\n".format(numToTake, '{'))
        outputFile.write("        if (values[i] != {}[i])\n".format(pattern.getVariableName()))
        outputFile.write("            return false;\n")
        outputFile.write("    }\n")
        outputFile.write("    return true;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateFilterRangeFunction(self, outputFile, functionTree, op, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

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
        kind = functionTree.function[6:]

        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)
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

    #
    # This is something like negation for sensor values
    #
    def generateInvertFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("invert")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1};\n".format(
                self.getDataType(), subReadFunction))
        # invert the value
        outputFile.write("    return !value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    #
    # This is not negation in the usual sense, but a negation of filter
    #
    def generateInvertFilterFunction(self, outputFile, functionTree, root):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)

        funName = self.getGeneratedFunctionName("InvertFilter")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} value = {1};\n".format(
                self.getDataType(), subReadFunction))
        # invert the filter
        outputFile.write("    *isFilteredOut = !*isFilteredOut;\n")
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    #
    # A logical function similar to Excel's IF()
    #
    def generateIfFunction(self, outputFile, functionTree, root):
        conditionFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], root)
        ifFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[1], root)
        elseFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[2], root)

        funName = self.getGeneratedFunctionName("If")
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} conditionValue = {1};\n".format(
                self.getDataType(), conditionFunction))
        outputFile.write("    if (conditionValue) return {0};\n".format(ifFunction))
        outputFile.write("    return {0};\n".format(elseFunction))
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateSyncFunction(self, outputFile, functionTree, root):
        assert self.syncOnlySensor

        funName = self.getGeneratedFunctionName(functionTree.function)
        outputFile.write("static inline {0} {1}(bool *isFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        for s in self.subsensors:
            outputFile.write("    {}SyncCallback();\n".format(s.getNameCC()))
        outputFile.write("    return 0;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateTakeFunction(self, outputFile, functionTree, aggregateFunction):
        subReadFunction = self.generateSubReadFunctions(
            outputFile, functionTree.arguments[0], None)

        numToTake = functionTree.arguments[1].asConstant()
        if numToTake is None:
            componentRegister.userError("Second argument of take() function is expected to be a constant!\n")
            return ""

        if len(functionTree.arguments) > 2:
            timeToTake = functionTree.arguments[2].asConstant()
            if timeToTake:
                # generate takeRecent function;
                # ignore "lazy" parameter in that case.
                return self.generateTakeRecentFunction(outputFile,
                                                       subReadFunction, aggregateFunction,
                                                       numToTake, timeToTake)

        lazy = getUseCaseParameterValue("lazy", self.sensorReadFunctionParams)

        staticIfLazy = "static " if lazy else ""

        funName = self.getGeneratedFunctionName("take" + toTitleCase(aggregateFunction))
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
        outputFile.write("    {}{} value;\n".format(staticIfLazy, self.getDataType()))

        if lazy:
            outputFile.write("    if (valuesCursor == 0) {\n\n")

        # MINIMUM
        if aggregateFunction == "min":
            outputFile.write("    value = {};\n".format(self.getMaxValue()))
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        if (values[i] < value) value = values[i];\n")
        # MAXIMUM
        elif aggregateFunction == "max":
            outputFile.write("    value = {};\n".format(self.getMinValue()))
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        if (values[i] > value) value = values[i];\n")
        # SUM
        elif aggregateFunction == "sum":
            outputFile.write("    value = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        value += values[i];\n")
        # AVERAGE
        elif aggregateFunction == "avg" or aggregateFunction == "average":
            outputFile.write("    value = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        value += values[i];\n")
            outputFile.write("    value /= {};\n".format(numToTake))
        # STANDARD DEVIATION
        elif aggregateFunction == "std" or aggregateFunction == "stdev":
            outputFile.write("    int32_t avg = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        avg += values[i];\n")
            outputFile.write("    avg /= {};\n".format(numToTake))
            outputFile.write("    value = 0;\n")
            outputFile.write("    for (i = 0; i < {}; ++i)\n".format(numToTake))
            outputFile.write("        value += abs(values[i] - avg);\n")
            outputFile.write("    value /= {};\n".format(numToTake))
        # OTHER
        else:
            componentRegister.userError("take(): unknown aggregate function {}()!\n".format(aggregateFunction));

        if lazy:
            outputFile.write("\n    }\n")
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateTakeRecentFunction(self, outputFile, subReadFunction, aggregateFunction, numToTake, timeToTake):
        funName = self.getGeneratedFunctionName("takeRecent" + toTitleCase(aggregateFunction))
        outputFile.write("static inline {0} {1}(bool *__unused)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    static struct {\n")
        outputFile.write("        {} value;\n".format(self.getDataType()))
        outputFile.write("        ticks_t timeRead;\n")
        outputFile.write("    {} values[{}];\n".format('}', numToTake))
        outputFile.write("    static uint16_t valuesCursor;\n")
        outputFile.write("    ticks_t time = getJiffies();\n")
        outputFile.write("    bool b = false, *isFilteredOut = &b;\n")
        outputFile.write("    {0} tmp = {1};\n".format(self.getDataType(), subReadFunction))
        outputFile.write("    if (!*isFilteredOut) {\n")
        outputFile.write("        values[valuesCursor].value = tmp;\n")
        outputFile.write("        values[valuesCursor].timeRead = time;\n")
        outputFile.write("        valuesCursor = (valuesCursor + 1) % {};\n".format(numToTake))
        outputFile.write("    }\n")
        outputFile.write("    {} value;\n".format(self.getDataType()))
        outputFile.write("    time -= {};\n".format(timeToTake))
        # MINIMUM
        if aggregateFunction == "min":
            outputFile.write("    value = {};\n".format(self.getMaxValue()))
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i) {}\n".format(numToTake, '{'))
            outputFile.write("        if (timeAfter(time, values[i].timeRead)) continue;\n")
            outputFile.write("        if (values[i].value < value) value = values[i].value;\n")
            outputFile.write("    }\n")
        # MAXIMUM
        elif aggregateFunction == "max":
            outputFile.write("    value = {};\n".format(self.getMinValue()))
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i) {}\n".format(numToTake, '{'))
            outputFile.write("        if (timeAfter(time, values[i].timeRead)) continue;\n")
            outputFile.write("        if (values[i] > value) value = values[i].value;\n")
            outputFile.write("    }\n")
        # SUM
        elif aggregateFunction == "sum":
            outputFile.write("    value = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i) {}\n".format(numToTake, '{'))
            outputFile.write("        if (timeAfter(time, values[i].timeRead)) continue;\n")
            outputFile.write("        value += values[i].value;\n")
            outputFile.write("    }\n")
        # AVERAGE
        elif aggregateFunction == "avg" or aggregateFunction == "average":
            outputFile.write("    value = 0;\n")
            outputFile.write("    uint16_t cnt = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i) {}\n".format(numToTake, '{'))
            outputFile.write("        if (timeAfter(time, values[i].timeRead)) continue;\n")
            outputFile.write("        cnt++;\n")
            outputFile.write("        value += values[i].value;\n")
            outputFile.write("    }\n")
            outputFile.write("    value /= cnt;\n")
        # STANDARD DEVIATION
        elif aggregateFunction == "std" or aggregateFunction == "stdev":
            outputFile.write("    int32_t avg = 0;\n")
            outputFile.write("    uint16_t cnt = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i) {}\n".format(numToTake, '{'))
            outputFile.write("        if (timeAfter(time, values[i].timeRead)) continue;\n")
            outputFile.write("        cnt++;\n")
            outputFile.write("        avg += values[i].value;\n")
            outputFile.write("    }\n")
            outputFile.write("    avg /= cnt;\n")
            outputFile.write("    value = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < {}; ++i) {}\n".format(numToTake, '{'))
            outputFile.write("        if (timeAfter(time, values[i].timeRead)) continue;\n")
            outputFile.write("        value += abs(values[i].value - avg);\n")
            outputFile.write("    }\n")
            outputFile.write("    value /= cnt;\n")
        # OTHER
        else:
            componentRegister.userError("take(): unknown aggregate function {}()!\n".format(aggregateFunction));
        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"

    def generateTupleFunction(self, outputFile, functionTree, aggregateFunction):
        subReadFunctions = []
        for a in functionTree.arguments:
            subReadFunctions.append(self.generateSubReadFunctions(outputFile, a, None))

        numToTake = len(subReadFunctions)

        funName = self.getGeneratedFunctionName("tuple" + toTitleCase(aggregateFunction))
        outputFile.write("static inline {0} {1}(bool *topLevelFilteredOut)\n".format(self.getDataType(), funName))
        outputFile.write("{\n")
        outputFile.write("    {0} values[{1}];\n".format(self.getDataType(), numToTake))
        outputFile.write("    uint16_t valuesCursor = 0;\n")
        outputFile.write("    bool b, *isFilteredOut = &b;\n")
        outputFile.write("    {} value;\n".format(self.getDataType()))
        for i in range(len(subReadFunctions)):
            outputFile.write("    b = false;\n")
            outputFile.write("    value = {};\n".format(subReadFunctions[i]))
            outputFile.write("    if (!*isFilteredOut) {\n")
            outputFile.write("        values[valuesCursor++] = value;\n".format(i, subReadFunctions[i]))
            outputFile.write("    }\n")

        # MINIMUM
        if aggregateFunction == "min":
            outputFile.write("    value = {};\n".format(self.getMaxValue()))
            outputFile.write("    uint16_t i; for (i = 0; i < valuesCursor; ++i)\n")
            outputFile.write("        if (values[i] < value) value = values[i];\n")
        # MAXIMUM
        elif aggregateFunction == "max":
            outputFile.write("    value = {};\n".format(self.getMinValue()))
            outputFile.write("    uint16_t i; for (i = 0; i < valuesCursor; ++i)\n")
            outputFile.write("        if (values[i] > value) value = values[i];\n")
        # SUM
        elif aggregateFunction == "sum":
            outputFile.write("    value = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < valuesCursor; ++i)\n")
            outputFile.write("        value += values[i];\n")
        # AVERAGE
        elif aggregateFunction == "avg" or aggregateFunction == "average":
            outputFile.write("    value = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < valuesCursor; ++i)\n")
            outputFile.write("        value += values[i];\n")
            outputFile.write("    value /= {};\n".format(numToTake))
        # STANDARD DEVIATION
        elif aggregateFunction == "std" or aggregateFunction == "stdev":
            outputFile.write("    int32_t avg = 0;\n")
            outputFile.write("    uint16_t i; for (i = 0; i < valuesCursor; ++i)\n")
            outputFile.write("        avg += values[i];\n")
            outputFile.write("    avg /= {};\n".format(numToTake))
            outputFile.write("    value = 0;\n")
            outputFile.write("    for (i = 0; i < valuesCursor; ++i)\n")
            outputFile.write("        value += abs(values[i] - avg);\n")
            outputFile.write("    value /= {};\n".format(numToTake))
        # OTHER
        else:
            componentRegister.userError("tuple(): unknown aggregate function {}()!\n".format(aggregateFunction))
        outputFile.write("    if (valuesCursor == 0) {\n")
        outputFile.write("        *topLevelFilteredOut = true;\n")
        outputFile.write("    }\n")

        outputFile.write("    return value;\n")
        outputFile.write("}\n\n")
        return funName + "(isFilteredOut)"


    def generateSubReadFunctions(self, outputFile, functionTree, root):
        if functionTree is None:
            # special case for remote sensors
            if self.isRemote() and self.specification._name == "Null":
                if len(self.remoteFields) > 1:
                    componentRegister.userError("Network packet '{}' with more than one field used: specify field name!\n".format(self.name))
                    return "0"
                baseSensorName = self.networkComponent.getPrefix() + self.remoteFields[0]
                baseSensor = componentRegister.sensors.get(baseSensorName)
                assert baseSensor
                return baseSensor.generateSubReadFunctions(outputFile, None, root)

            # a physical sensor; generate just raw read function
            if self.specification._readFunctionDependsOnParams:
                readFunctionSuffix = str(self.readFunctionNum)
                self.readFunctionNum += 1
            else:
                readFunctionSuffix = ""
                # avoid generating it twice
                if self.generatedDefaultRawReadFunction:
                    return self.getRawReadFunction(readFunctionSuffix, root)
                self.generatedDefaultRawReadFunction = True

            self.markAsUsed()

            outputFile.write("inline {0} {1}ReadRaw{2}(bool *__unused)\n".format(
                    self.getDataType(), self.getNameCC(), readFunctionSuffix))
            outputFile.write("{\n")

            if self.specification._readFunctionDependsOnParams:
                specifiedReadFunction = self.getDependentParameterValue(
                    "readFunction", self.sensorReadFunctionParams)
            else:
                specifiedReadFunction = self.getParameterValue("readFunction", None)

            if specifiedReadFunction is None:
                componentRegister.userError("Sensor '{}' has no valid read function!\n".format(self.name))
                specifiedReadFunction = "0"
            outputFile.write("    return {};\n".format(specifiedReadFunction))
            outputFile.write("}\n\n")

            # return either this, just generated function or cacheRead()
            return self.getRawReadFunction(readFunctionSuffix, root)

        # if this is a sensor name with no arguments, find the sensor and recurse
        if len(functionTree.arguments) == 0:
            # print "functionTree.function =", functionTree.function
            name = functionTree.function
            if isinstance(functionTree.function, Value):
                # special case for constants
                return functionTree.function.asString()
            if isinstance(functionTree.function, SealValue):
                name = functionTree.function.firstPart
                # print "name = ", name
                if name in componentRegister.systemStates:
                    # special case for variables ("states")
                    # TODO: should replace code here for X.Y type values
                    return "sealState_" + name
                if root and root.containingOutputComponent:
                    # yeah!
                    componentName = root.containingOutputComponent.componentUseCase.outputUseCase.getNameCC()
                    return componentName + "Packet." + name

            sensor = componentRegister.findComponentByName(name)
            assert sensor
            assert type(sensor) is Sensor
            # Use parameters from self as the base, but also
            # inherit (propagate) them from the root
            sensor.sensorReadFunctionParams = mergeParameters(
                sensor.parameters, self.sensorReadFunctionParams)
            #print "inherit params from", self.name, "to", sensor.name
            return sensor.generateSubReadFunctions(outputFile, sensor.functionTree, root)

        (validationOk, errorMessage) = validateFunction(functionTree)
        if not validationOk:
            componentRegister.userError(errorMessage)
            return "0"

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
        if functionTree.function == "avg" or functionTree.function == "average":
            return self.generateAvgFunction(outputFile, functionTree, root)
        if functionTree.function == "ewma":
            return self.generateEWMAFunction(outputFile, functionTree, root)
        if functionTree.function == "std" or functionTree.function == "stdev":
            return self.generateStdevFunction(outputFile, functionTree, root)
        if functionTree.function == "smoothen" or functionTree.function == "blur":
            return self.generateSmoothenFunction(outputFile, functionTree, root)
        if functionTree.function == "sharpen" or functionTree.function == "contrast":
            return self.generateSharpenFunction(outputFile, functionTree, root)
        if functionTree.function == "changed":
            return self.generateChangedFunction(outputFile, functionTree, root)
        if functionTree.function == "sum":
            return self.generateSumFunction(outputFile, functionTree, root)
        if functionTree.function == "plus" or functionTree.function == "add": # synonyms to one use of sum()
            return self.generateArithmeticFunction(outputFile, functionTree, '+', root)
        if functionTree.function == "minus" or functionTree.function == "subtract":
            return self.generateArithmeticFunction(outputFile, functionTree, '-', root)
        if functionTree.function == "multiply" or functionTree.function == "times":
            return self.generateArithmeticFunction(outputFile, functionTree, '*', root)
        if functionTree.function == "divide":
            return self.generateArithmeticFunction(outputFile, functionTree, '/', root)
        if functionTree.function == "modulo":
            return self.generateArithmeticFunction(outputFile, functionTree, '%', root)
        if functionTree.function == "difference": # same as abs(minus(x, y))
            return self.generateSymmetricDiffFunction(outputFile, functionTree, root)
        if functionTree.function == "square":
            return self.generateSquareFunction(outputFile, functionTree, root)
        if functionTree.function == "sqrt":
            return self.generateSqrtFunction(outputFile, functionTree, root)
        if functionTree.function == "power":
            return self.generatePowerFunction(outputFile, functionTree, root)
        if functionTree.function == "match":
            return self.generateMatchFunction(outputFile, functionTree, root)
        if functionTree.function == "filterrange":
            return self.generateFilterRangeFunction(outputFile, functionTree, root)
        if functionTree.function[:6] == "filter":
            return self.generateFilterFunction(outputFile, functionTree, root)
        if functionTree.function == "invert":
            return self.generateInvertFunction(outputFile, functionTree, root)
        if functionTree.function == "invertfilter":
            return self.generateInvertFilterFunction(outputFile, functionTree, root)
        if functionTree.function == "if":
            return self.generateIfFunction(outputFile, functionTree, root)
        if functionTree.function == "sync":
            if functionTree != self.functionTree:
                componentRegister.userError("sync() function only allowed at the top level!\n")
            else:
                return self.generateSyncFunction(outputFile, functionTree, root)
        if functionTree.function == "take":
            componentRegister.userError("take() function can be used only as an argument to one of the following:\n" +
                      "    min(), max(), sum(), avg(), stdev()!\n")
            return "0"
        if functionTree.function == "tuple":
            componentRegister.userError("tuple() function can be used only as an argument to one of the following:\n" +
                      "    min(), max(), sum(), avg(), stdev()!\n")
            return "0"
        componentRegister.userError("unhandled function {}()\n".format(functionTree.function))
        return "0"


    def generateReadFunctions(self, outputFile, useCase):
        if not self.isUsed(): return

        if self.isRemote(): return

        if useCase is not None:
            readFunctionSuffix = str(self.readFunctionNum)
            useCase.readFunctionSuffix = readFunctionSuffix
            self.readFunctionNum += 1
        else:
            readFunctionSuffix = ""

        if useCase: self.sensorReadFunctionParams = useCase.parameters
        else: self.sensorReadFunctionParams = self.parameters

        subReadFunction = self.generateSubReadFunctions(
            outputFile, self.functionTree, self)

        if self.cacheNeeded:
            outputFile.write("static inline {0} {1}CacheReadProcess{2}(bool *isFilteredOut)\n".format(
                    self.getDataType(), self.getNameCC(), readFunctionSuffix))
            outputFile.write("{\n")
            outputFile.write("    return {};\n".format(subReadFunction))
            outputFile.write("}\n\n")

        # generate reading and processing function
        outputFile.write("static inline {0} {1}ReadProcess{2}(bool *isFilteredOut)\n".format(
                self.getDataType(), self.getNameCC(), readFunctionSuffix))
        outputFile.write("{\n")

        # HERE: turn on/off? use associated componenent?

        if self.cacheNeeded:
            # TODO: in some cases this will lead to double read from cache (inefficient)
            dataFormat = str(self.getDataSize() * 8)
            outputFile.write("    return cacheReadSensor{0}({1}, &{2}CacheReadProcess{3}, {4}, isFilteredOut);\n".format(
                dataFormat, self.cacheNumber, self.getNameCC(),
                readFunctionSuffix, self.minUpdatePeriod))
        else:
            outputFile.write("    return {};\n".format(subReadFunction))

        outputFile.write("}\n\n")

######################################################
class PacketField(object):
    def __init__(self, sensorID, sensorName, dataSize, dataType, count = 1, defaultValue = None):
        self.sensorID = sensorID
        self.sensorName = sensorName
        # always use 4 bytes, because decoding otherwise is too messy
        # (could and should be optimized if the need arises...)
        self.dataSize = 4
        self.dataType = "int32_t"
        self.count = count   # how many of this field?
        self.defaultValue = defaultValue
        self.isRealSensor = sensorID >= PACKET_FIELD_ID_FIRST_FREE

######################################################
class OutputUseCase(object):
    def __init__(self, parent, useCase, number, fields):
        self.parent = parent
        self.useCase = useCase
        self.useCase.outputUseCase = self
        self.packetFields = []
        self.headerMasks = []
        self.useOnlyFields = dict()
        self.usedIds = set()
        self.numSensorFields = 0
        self.name = parent.name
        if number != 0:
            self.name += str(number)
        self.useOnlyFields = {}
        self.networkComponents = []
        for f in fields:
            nc = componentRegister.networkComponents.get(f[0])
            if nc:
                self.networkComponents.append(nc)
                for nf in nc.fields:
                    # if nf not in commonFields:
                    self.useOnlyFields[nc.getPrefix() + nf] = f[1]
            else:
                self.useOnlyFields[f[0]] = f[1]

        self.isAggregate = self.getParameterValue("aggregate", False)
        if not self.isAggregate and parent.name != "serial":
            componentRegister.userError("Parameter 'aggregate' must be set (true) for output '{}'\n".format(
                    toTitleCase(parent.name)))

    def getNameTC(self):
        return toTitleCase(self.name)

    def getNameCC(self):
        return toCamelCase(self.name)

    def getNameUC(self):
        return self.name.upper()

    def getParameterValue(self, parameter, defaultValue = None):
        result = self.useCase.getParameterValueValue(parameter, defaultValue)
        if type(result) is str:
            return result.strip('"')
        return result

    def generateVariables(self, outputFile):
        pass

    def addPacketTypes(self, s, count):
        # special case for a dummy sync-only "sensor"
        if s.syncOnlySensor:
            for subsensor in s.subsensors:
                self.addPacketTypes(subsensor, count)
            return
        # normal case
        # assert s.getSystemwideID() not in self.usedIds
        if s.getSystemwideID() in self.usedIds: return
        self.packetFields.append(PacketField(
                s.getSystemwideID(), s.getNameCC(), s.getDataSize(), s.getDataType(), count))
        self.usedIds.add(s.getSystemwideID())
        self.numSensorFields += 1 # yes, aggregate fields are counted as one

    def hasPacketField(self, fieldName):
        for f in self.packetFields:
            if f.sensorName == fieldName: return True
        return False

    def definePacketType(self):
        # TODO: handle the case of multiple use cases of the same sensor with different value params! (marginal)
        self.packetFields = []
        self.usedIds = set()
        self.numSensorFields = 0
        for s in componentRegister.sensors.values():
            if not s.isUsed() and s.networkComponent is None:
                continue

            # check if this field is used
            if self.useOnlyFields:
                count = self.useOnlyFields.get(s.name.lower())
            else:
                count = 1

            if count is not None:
                self.addPacketTypes(s, count)

        if self.getParameterValue("timestamp") or ("timestamp" in self.useOnlyFields):
            # 4-byte timestamp (seconds since system uptime)
            self.packetFields.append(PacketField(
                    PACKET_FIELD_ID_TIMESTAMP, "timestamp", 4, "uint32_t"))
            self.usedIds.add(PACKET_FIELD_ID_TIMESTAMP)

        if self.getParameterValue("address") or ("address" in self.useOnlyFields):
            # 2-byte address
            self.packetFields.append(PacketField(
                    PACKET_FIELD_ID_ADDRESS, "address", 2, "uint16_t"))
            self.usedIds.add(PACKET_FIELD_ID_ADDRESS)

        if self.getParameterValue("sequencenumber") or ("sequencenumber" in self.useOnlyFields):
            # 4-byte seqnum
            self.packetFields.append(PacketField(
                    PACKET_FIELD_ID_SEQNUM, "sequenceNumber", 4, "uint32_t"))
            self.usedIds.add(PACKET_FIELD_ID_SEQNUM)

        if self.getParameterValue("issent") or ("issent" in self.useOnlyFields):
            # boolean: sent/not
            self.packetFields.append(PacketField(
                    PACKET_FIELD_ID_IS_SENT, "isSent", 4, "uint32_t"))
            self.usedIds.add(PACKET_FIELD_ID_IS_SENT)

        # add default valued fields
        for f in self.useOnlyFields:
            if self.hasPacketField(f): continue
            # look at parameters for explicity specified value!
            size = 4
            type = "uint32_t"
            if f in commonFields:
                systemwideID = commonFields[f]
            elif f in componentRegister.sensors:
                systemwideID = componentRegister.sensors.get(f).getSystemwideID()
            else:
                continue
            paramValue = self.getParameterValue(f)
            if paramValue:
                self.packetFields.append(PacketField(
                        systemwideID, f, size, type, count = 1, defaultValue = paramValue))


        self.packetFields = sorted(self.packetFields, key = lambda f: f.sensorID)

    def generateConstants(self, outputFile):
        pass

    def generatePacketType(self, outputFile, indent):
        if len(self.packetFields) == 0: return

        if not self.isAggregate: return

        outputFile.write(getIndent(indent) + "struct {0}Packet_s {1}\n".format(self.getNameTC(), '{'))

        # --- generate header
        outputFile.write(getIndent(indent + 1) + "uint16_t magic;\n")  # 2-byte magic number
        outputFile.write(getIndent(indent + 1) + "uint16_t crc;\n")    # 2-byte crc (always)
        # type masks
        headerField = 0
        numField = 1
        for f in self.packetFields:
            if f.sensorID >= numField * 32:
                headerField |= 1 << 31
                outputFile.write("#define {}_TYPE_MASK {:#x}\n".format(self.getNameUC(), headerField))
                outputFile.write(getIndent(indent + 1) + "uint32_t typeMask{};\n".format(numField))
                self.headerMasks.append(headerField)
                headerField = 0
                numField += 1
            bit = f.sensorID
            bit -= 32 * (numField - 1)
            if (f.sensorID >= PACKET_FIELD_ID_FIRST_FREE):
                headerField |= (1 << bit)
        if headerField:
            outputFile.write("#define {}_TYPE_MASK {:#x}\n".format(self.getNameUC(), headerField))
            outputFile.write(getIndent(indent + 1) + "uint32_t typeMask{};\n".format(numField))
            self.headerMasks.append(headerField)

        # --- generate the body of the packet
        reservedNum = 0
        packetLen = 0
        for f in self.packetFields:
            paddingNeeded = packetLen % f.dataSize
            if paddingNeeded:
                outputFile.write(getIndent(indent + 1) + "uint{}_t __reserved{};\n".format(
                        paddingNeeded * 8, reservedNum))
                packetLen += paddingNeeded
                reservedNum += 1
            if f.count == 1:
                outputFile.write(getIndent(indent + 1) + "{0} {1};\n".format(f.dataType, f.sensorName))
            else:
                outputFile.write(getIndent(indent + 1) + "{0} {1}[{2}];\n".format(
                        f.dataType, f.sensorName, f.count))
            packetLen += f.dataSize * f.count

        # --- finish the packet
        outputFile.write(getIndent(indent) + "} PACKED;\n")
        # add a typedef
        outputFile.write(getIndent(indent) + "typedef struct {0}Packet_s {0}Packet_t;\n".format(
                self.getNameTC()))

        outputFile.write(getIndent(indent) + "{0}Packet_t {1}Packet;\n\n".format(
                self.getNameTC(), self.getNameCC()))

    def getPacketFields(self):
        s = ''
        for f in self.packetFields:
            s += '    "'
            s += f.sensorName
            s += '",\n'
        return s

    def generateSerialOutputCode(self, outputFile, sensorsUsed):
        usedSizes = set()
        if self.isAggregate:
            for f in self.packetFields:
                usedSizes.add((f.dataSize, f.dataType))
        else:
            for s in sensorsUsed:
                usedSizes.add((s.getDataSize(), s.getDataType()))
        generateSerialFunctions(usedSizes, outputFile)

        if not self.isAggregate:
            # done
            return

        crc = "Crc" if self.getParameterValue("crc") else ""

        outputFile.write("static inline void {}PacketPrint(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    PRINTF(\"======================\\n\");\n")
        for f in self.packetFields:
            if f.count == 1:
                outputFile.write("    serialPrint{3}_{0}(\"{1}\", serialPacket.{2});\n".format(
                        f.dataType, toTitleCase(f.sensorName), f.sensorName, crc))
            else:
                for i in range(f.count):
                    outputFile.write("    serialPrint{3}_{0}(\"{1}[{3}]\", serialPacket.{2}[{3}]);\n".format(
                            f.dataType, toTitleCase(f.sensorName), f.sensorName, i, crc))
        outputFile.write("}\n\n")


    def generateOutputCode(self, outputFile, sensorsUsed):
        if self.getNameCC() == "serial":
            self.generateSerialOutputCode(outputFile, sensorsUsed)
            if not self.isAggregate: return

        outputFile.write("static inline void {0}PacketInit(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    {0}Packet.typeMask1 = 0;\n".format(self.getNameCC()))
        outputFile.write("}\n\n")

        outputFile.write("static inline void {0}PacketSend(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    {0}Packet.magic = SEAL_MAGIC;\n".format(self.getNameCC()))
        if PACKET_FIELD_ID_SEQNUM in self.usedIds:
            outputFile.write("    static uint32_t seqnum;\n")
            outputFile.write("    if (!({0}Packet.typeMask1 & SEQNUM_TYPE_MASK))\n".format(self.getNameCC()))
            outputFile.write("        {0}Packet.sequenceNumber = ++seqnum;\n".format(self.getNameCC()))
        if PACKET_FIELD_ID_TIMESTAMP in self.usedIds:
            outputFile.write("    if (!({0}Packet.typeMask1 & TIMESTAMP_TYPE_MASK))\n".format(self.getNameCC()))
            outputFile.write("        {0}Packet.timestamp = getSyncTimeSec();\n".format(self.getNameCC()))
        if PACKET_FIELD_ID_ADDRESS in self.usedIds:
            outputFile.write("    if (!({0}Packet.typeMask1 & ADDRESS_TYPE_MASK))\n".format(self.getNameCC()))
            outputFile.write("        {0}Packet.address = localAddress;\n".format(self.getNameCC()))
            componentRegister.additionalConfig.add("addressing")
        if PACKET_FIELD_ID_IS_SENT in self.usedIds:
            outputFile.write("    if (!({0}Packet.typeMask1 & ISSENT_TYPE_MASK))\n".format(self.getNameCC()))
            outputFile.write("        {0}Packet.isSent = false;\n".format(self.getNameCC()))
        for f in self.packetFields:
            if f.defaultValue:
                outputFile.write("    if (!({0}Packet.typeMask1 & (1 << {1})))\n".format(self.getNameCC(), f.sensorID))
                outputFile.write("        {0}Packet.{1} = {2};\n".format(self.getNameCC(), f.sensorName, f.defaultValue))

        initialMask = 0
        for f in self.packetFields:
            if not f.isRealSensor:
                initialMask |= (1 << f.sensorID)
        outputFile.write("    {0}Packet.typeMask1 |= {1:#x};\n".format(
                self.getNameCC(), initialMask))

        if True:
            outputFile.write("    {0}Packet.crc = crc16((const uint8_t *) &{0}Packet + 4, sizeof({0}Packet) - 4);\n".format(
                    self.getNameCC()))

        if isinstance(self, FileOutputUseCase):
            useFunction = self.getNameCC() + "Print()"
        else:
            useFunction = self.getParameterValue("useFunction")
        if useFunction:
            outputFile.write("    {0};\n".format(useFunction))
        outputFile.write("    {0}PacketInit();\n".format(self.getNameCC()))
        outputFile.write("}\n\n")

        outputFile.write("static inline bool {0}PacketIsFull(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")
        outputFile.write("    return ({0}Packet.typeMask1 & {1}_TYPE_MASK) == {1}_TYPE_MASK;\n".format(
                self.getNameCC(), self.getNameUC()))
        outputFile.write("}\n\n")

    def generateCallbackCode(self, sensorName, outputFile, suffix):
        found = False
        for f in self.packetFields:
            if f.sensorName.lower() == sensorName.lower():
                found = True
                break
        if not found: return

        if not self.isAggregate:
            crc = "Crc" if self.getParameterValue("crc") else ""
            outputFile.write('        {0}Print{3}_{1}("{2}", {2}Value);\n'.format(
                    self.getNameCC(), f.dataType, toCamelCase(sensorName), crc))
            return

        if f.count == 1:
            outputFile.write("        {0}Packet.{1} = {1}Value;\n".format(
                    self.getNameCC(), toCamelCase(sensorName)))
        else:
            outputFile.write("        static uint16_t lastIdx;\n")
            outputFile.write("        {0}Packet.{1}[lastIdx] = {1}Value;\n".format(
                    self.getNameCC(), toCamelCase(sensor)))
            outputFile.write("        lastIdx = (lastIdx + 1) % {};\n".format(f.count))

        outputFile.write("        {0}Packet.typeMask1 |= {1}_TYPE_MASK;\n".format(
                self.getNameCC(), sensorName.upper()))

        outputFile.write("        if ({0}PacketIsFull()) {1}\n".format(self.getNameCC(), '{'))
        outputFile.write("            {0}PacketSend();\n".format(self.getNameCC()))
        outputFile.write("        }\n\n")

    def generateAppMainCode(self, outputFile):
        if self.isAggregate:
            outputFile.write("    {0}PacketInit();\n".format(self.getNameCC()))

    def generateBinaryOutputCode(self, outputFile, f, indent):
        if f.count == 1:
            outputFile.write(getIndent(indent) + "fwrite(&{0}Packet.{1}, sizeof({0}Packet.{1}), 1, out);\n".format(
                    self.getNameCC(), f.sensorName))
        else:
            for i in range(f.count):
                outputFile.write(getIndent(indent) + "fwrite(&{0}Packet.{1}[{2}], sizeof({0}Packet.{1}[{2}]), 1, out);\n".format(
                        self.getNameCC(), f.sensorName, i))

    def generateTextOutputCode(self, outputFile, f, indent):
        if f.count == 1:
            outputFile.write(getIndent(indent) + 'fprintf(out, "%lu ", {0}Packet.{1});\n'.format(
                    self.getNameCC(), f.sensorName))
        else:
            for i in range(f.count):
                outputFile.write(getIndent(indent) + 'fprintf(out, "%lu ", {0}Packet.{1}[{2}]);\n'.format(
                        self.getNameCC(), f.sensorName, i))

    def generateBinaryInputCode(self, outputFile, f, indent):
        if f.count == 1:
            outputFile.write(getIndent(indent) + "fread(&{0}Packet.{1}, sizeof({0}Packet.{1}), 1, in);\n".format(
                    self.getNameCC(), f.sensorName))
        else:
            for i in range(f.count):
                outputFile.write(getIndent(indent) + "fread(&{0}Packet.{1}[{2}], sizeof({0}Packet.{1}[{2}]), 1, in);\n".format(
                        self.getNameCC(), f.sensorName, i))

    def generateTextInputCode(self, outputFile, f, indent):
        if f.count == 1:
            outputFile.write(getIndent(indent) + 'fscanf(in, "%lu ", &{0}Packet.{1});\n'.format(
                    self.getNameCC(), f.sensorName))
            # XXX: for debugging
            # outputFile.write('        PRINTF("%lu ", {0}Packet.{1});\n'.format(
            #       self.getNameCC(), f.sensorName))
        else:
            for i in range(f.count):
                outputFile.write(getIndent(indent) + 'fscanf(in, "%lu ", &{0}Packet.{1}[{2}]);\n'.format(
                        self.getNameCC(), f.sensorName, i))

######################################################
class FileOutputUseCase(OutputUseCase):
    def __init__(self, parent, useCase, number, fields):
        super(FileOutputUseCase, self).__init__(parent, useCase, number, fields)

        self.isText = True
        if self.getParameterValue("binary", False): self.isText = False

        # file, name, filename are synonyms
        self.filename = self.getParameterValue("file")
        if not self.filename: self.filename = self.getParameterValue("name")
        if not self.filename: self.filename = self.getParameterValue("filename")
        if self.filename is None:
            # extension is determined by type
            if self.isText: suffix = "csv"
            else: suffix = "bin"
            self.filename = "file{0:03d}.{1}".format(number, suffix)
        elif self.getParameterValue("binary") is None and self.getParameterValue("text") is None:
            # type is determined by extension
            self.isText = (self.filename[-4:] == '.txt' or self.filename[-4:] == '.csv')

        if self.filename.find('.') == -1:
            # attach an extesion automatically
            if self.isText: self.filename += ".csv"
            else: self.filename += ".bin"

    def generateTextHeading(self, outputFile, f):
        if f.count == 1:
            outputFile.write('        fputs("{} ", out);\n'.format(f.sensorName))
        else:
            for i in range(f.count):
                outputFile.write('        fputs("{}[{}] ", out);\n'.format(f.sensorName, i))

    def generateOutputCode(self, outputFile, sensorsUsed):
        outputFile.write("static inline void {}Print(void)\n".format(self.getNameCC()))
        outputFile.write("{\n")

        if self.isText:
            outputFile.write('    bool accessOk = access("{}", F_OK) == 0;\n'.format(self.filename))
        outputFile.write('    FILE *out = fopen("{}", "ab");\n'.format(self.filename))
        outputFile.write('    if (!out) return;\n')

        if self.isText:
            outputFile.write('    if (!accessOk) { // new file created\n')

            for f in self.packetFields:
                self.generateTextHeading(outputFile, f)
            outputFile.write("        fputc('\\r', out);\n")
            outputFile.write("        fputc('\\n', out);\n")
            outputFile.write('    };\n')

        for f in self.packetFields:
            if self.isText:
                self.generateTextOutputCode(outputFile, f, indent = 1)
            else:
                self.generateBinaryOutputCode(outputFile, f, indent = 1)

        if self.isText:
            outputFile.write("    fputc('\\r', out);\n")
            outputFile.write("    fputc('\\n', out);\n")

        outputFile.write("    fclose(out);\n")
        outputFile.write("}\n\n")

        # call superclass function too
        super(FileOutputUseCase, self).generateOutputCode(outputFile, sensorsUsed)

######################################################
class FromFileOutputUseCase(OutputUseCase):
    def __init__(self, parent, useCase, number, fields):
        super(FromFileOutputUseCase, self).__init__(parent, useCase, number, fields)
        # file, name, filename are synonyms
        self.filename = self.getParameterValue("file")
        if not self.filename: self.filename = self.getParameterValue("filename")
        if not self.filename: self.filename = self.getParameterValue("name")
        # TODO: detect by contents?
        self.isText = (self.filename[-4:] == '.txt' or self.filename[-4:] == '.csv')

        self.condition = useCase.parameters.get("where")
        if not isinstance(self.condition, Expression): self.condition = None
        if self.condition: self.condition.dependentOnComponent = self

        self.isAggregate = True
        self.networkComponents = []

    def generateConstants(self, outputFile):
        pass

    def generateVariables(self, outputFile):
        pass

    def findFileType(self):
        self.associatedFileOutputUseCase = None
        fileComponent = componentRegister.outputs.get("file")
        if fileComponent is None: return
        for o in fileComponent.outputUseCases:
            if o.filename == self.filename:
                self.associatedFileOutputUseCase = o
                return True
        return False

    def prepareToGenerateCallbacks(self, outputFile):
        if not self.findFileType():
            componentRegister.userError(("Output '{0}' has file as parameter," \
               + "but the file is not used otherwise in the program and has unknown type\n").format(
                    self.getNameCC()))
            return

        self.isText = self.associatedFileOutputUseCase.isText
        self.packetFields = self.associatedFileOutputUseCase.packetFields
        self.usedIds = self.associatedFileOutputUseCase.usedIds
        self.numSensorFields = self.associatedFileOutputUseCase.numSensorFields
        self.networkComponents = self.associatedFileOutputUseCase.networkComponents

        # needed to mark the correspodning sensors as used
        if self.condition:
            self.conditionEvaluationCode = self.condition.getEvaluationCode(componentRegister).rstrip("\n\r;")

        # define packet type
        self.generatePacketType(outputFile, indent = 0)
        if self.getNameCC() == "serial":
            self.generateSerialOutputCode(outputFile, sensorsUsed = [])

    def generateCallbacks(self, outputFile):
        if self.associatedFileOutputUseCase is None:
            return # error already reported

        doRewriteFile = PACKET_FIELD_ID_IS_SENT in self.usedIds

        outputFile.write("void {0}{1}{2}Process(void)\n".format(
                self.getNameCC(), self.useCase.branchName, self.useCase.numInBranch))
        outputFile.write("{\n")
        outputFile.write("    bool isFilteredOut;\n")

        outputFile.write("    {}Packet.typeMask1 = {}_TYPE_MASK;\n\n".format(
                self.getNameCC(), self.getNameUC()))

        outputFile.write('    FILE *in = fopen("{}", "rb");\n'.format(self.filename))
        outputFile.write('    if (!in) return;\n')

        if doRewriteFile:
            # TODO XXX: copy the input file instead!
            outputFile.write('    FILE *out = fopen("{}.tmp", "wb");\n'.format(self.filename))
            outputFile.write('    if (!out) return;\n')

        if self.isText:
            # skip first line
            outputFile.write("\n    uint8_t c;\n")
            outputFile.write("    do {\n")
            outputFile.write("        c = fgetc(in);\n")
            if doRewriteFile:
                # copy first line to output file
                outputFile.write("        fputc(c, out);\n")
            outputFile.write("    } while (c != '\\n' && !feof(in));\n\n")

        # main loop: read the file and print its contents
        outputFile.write("    while (1) {\n")

        # outputFile.write('        PRINTF("enter loop\\n");\n')

        for f in self.packetFields:
            if self.isText:
                self.generateTextInputCode(outputFile, f, indent = 2)
            else:
                self.generateBinaryInputCode(outputFile, f, indent = 2)

        # if reading returned end of file, return here
        outputFile.write("        if (feof(in)) break;\n")

        # for debugging
        # outputFile.write('        PRINTF("\\n");\n')

        if self.condition:
            outputFile.write("\n")
            outputFile.write("        if (!{}) continue;\n\n".format(
                    self.conditionEvaluationCode))

        # for debugging
        # outputFile.write('        PRINTF("OK\\n");\n')

        # fill rest of packet fields
        outputFile.write("        {0}Packet.magic = SEAL_MAGIC;\n".format(self.getNameCC()))
        if PACKET_FIELD_ID_IS_SENT in self.usedIds:
            # mark the packet as sent
            outputFile.write("        {0}Packet.isSent = true;\n".format(self.getNameCC()))
        if True:
            outputFile.write("        {0}Packet.crc = crc16((const uint8_t *) &{0}Packet + 4, sizeof({0}Packet) - 4);\n".format(
                    self.getNameCC()))

        if isinstance(self, FileOutputUseCase):
            useFunction = self.getNameCC() + "Print()"
        else:
            useFunction = self.getParameterValue("useFunction")
        if useFunction:
            outputFile.write("        {0};\n".format(useFunction))

        if doRewriteFile:
            for f in self.packetFields:
                if self.isText:
                    self.generateTextOutputCode(outputFile, f, indent = 2)
                else:
                    self.generateBinaryOutputCode(outputFile, f, indent = 2)
            if self.isText:
                outputFile.write("        fputc('\\r', out);\n")
                outputFile.write("        fputc('\\n', out);\n")
            outputFile.write("\n")

        outputFile.write("        mdelay(10);\n")

        outputFile.write("    }\n")

        # outputFile.write('        PRINTF("close files\\n");\n')

        # close the file
        outputFile.write("    fclose(in);\n")

        if doRewriteFile:
            # replace input file with output file
            outputFile.write("    fclose(out);\n")
            outputFile.write('    rename("{0}.tmp", "{0}");\n'.format(self.filename))

        outputFile.write("}\n\n")

    # replace code for a condition
    def replaceCode(self, parameterName):

        # search for remote fields
        for f in self.packetFields:
            sensorName = f.sensorName.lower()
            if sensorName[:8] == '__remote':
                sensorName = sensorName[10:]
                if sensorName == parameterName:
                    return self.getNameCC() + "Packet." + f.sensorName

        # search for other fields
        for f in self.packetFields:
            if f.sensorName.lower() == parameterName:
                return self.getNameCC() + "Packet." + f.sensorName

        # Not an error! Continue lookup in the regular way.
        # componentRegister.userError(("Output '{0}': unknown parameter '{1}' in 'where' condition\n").format(
        #            self.getNameCC(), parameterName))
        return None

    def generateOutputCode(self, outputFile, sensorsUsed):
        pass

    def generateCallbackCode(self, sensorName, outputFile, suffix):
        pass

    def generateAppMainCode(self, outputFile):
        pass


######################################################
class Output(Component):
    def __init__(self, name, specification):
        super(Output, self).__init__(name, specification)
        self.outputUseCases = []

    def addOutputUseCase(self, useCase, fields):
        id = len(self.outputUseCases)
        if self.name == "file":
            ouc = FileOutputUseCase(self, useCase, id, fields)
        else:
            filename = useCase.getParameterValue("file")
            if not filename: filename = useCase.getParameterValue("filename")

            if filename:
                ouc = FromFileOutputUseCase(self, useCase, id, fields)
                useCase.generateAlarm = True
            else:
                ouc = OutputUseCase(self, useCase, id, fields)

        self.outputUseCases.append(ouc)

    def definePacketType(self):
        for ouc in self.outputUseCases:
            ouc.definePacketType()

    def generatePacketType(self, outputFile):
        for ouc in self.outputUseCases:
            if isinstance(ouc, FromFileOutputUseCase):
                pass
            else:
                ouc.generatePacketType(outputFile, 0)

    def generateVariables(self, outputFile):
        for ouc in self.outputUseCases:
            ouc.generateVariables(outputFile)

    def generateConstants(self, outputFile):
        super(Output, self).generateConstants(outputFile)
        for ouc in self.outputUseCases:
            ouc.generateConstants(outputFile)

    def generateOutputCode(self, outputFile, sensorsUsed):
        for ouc in self.outputUseCases:
            ouc.generateOutputCode(outputFile, sensorsUsed)

    def generateCallbackCode(self, sensorName, outputFile, suffix):
        for ouc in self.outputUseCases:
            ouc.generateCallbackCode(sensorName, outputFile, suffix)

    def generateAppMainCode(self, outputFile):
        for ouc in self.outputUseCases:
            ouc.generateAppMainCode(outputFile)


######################################################
class SetUseCase(object):
    def __init__(self, parent, expression, conditions, branchNumber, isNew):
        self.parent = parent
        self.expressionCode = expression.getEvaluationCode(componentRegister)
        self.conditions = list(conditions) # deep copy!
        self.branchNumber = branchNumber
        self.isNew = isNew
        componentRegister.branchCollection.addUseCase(branchNumber, self)

    def getType(self):
        return "int32_t"

    def generateVariables(self, outputFile):
        if self.isNew:
            outputFile.write("{0} {1};\n".format(self.getType(), self.parent.getVariableName()))

    def generateBranchEnterCode(self, outputFile):
        outputFile.write("    {\n")
        if self.expressionCode.find("isFilteredOut") != -1:
            # TODO: semantic problem - what to do when isFilteredOut happens to be true?
            outputFile.write("        bool isFilteredOut = false;\n")

        # set the value
        outputFile.write("        {0} = {1}".format(self.parent.getVariableName(), self.expressionCode))
        # re-evaluate all conditions that depend on it
        for c in self.parent.dependentConditions:
            outputFile.write("        condition{}Callback();\n".format(c.id))

        outputFile.write("    }\n")

    def generateBranchExitCode(self, outputFile):
        pass


######################################################
class SystemState(object):
    def __init__(self, name):
        self.name = name
        # list of use cases (i.e. when the state is changed)
        self.useCases = []
        # list of conditions that use this system state
        self.dependentConditions = set()

    def addUseCase(self, expression, conditions, branchNumber):
        isNew = len(self.useCases) == 0
        self.useCases.append(
            SetUseCase(self, expression, conditions, branchNumber, isNew))

    def generateVariables(self, outputFile):
        self.useCases[0].generateVariables(outputFile)

    def getVariableName(self):
        return "sealState_" + self.name

######################################################
class NetworkComponent(object):
    def __init__(self, name, fields, id):
        self.name = name
        self.fields = set(fields)
        self.fieldsUsedInConditions = set()
        self.sortedFields = []
        self.typemask = 0
        self.isError = False
        self.id = id

    def getNameCC(self):
        return self.name # XXX

    def getNameTC(self):
        return toTitleCase(self.name)

    def getPrefix(self):
        return "__remote{:02}".format(self.id)

    def replaceCode(self, fieldName, condition):
        fieldName = fieldName.lower()
        if fieldName == "value" \
                and fieldName not in self.fields \
                and len(self.fields) >= 1:
            fieldName = self.fields.pop()
            self.fields.add(fieldName) # re-add it, because pop() removes the element

        if fieldName not in self.fields:
            componentRegister.userError("Network component '{0}' has no field named '{1}'\n".format(
                    self.name, fieldName))
            return "0"
        self.fieldsUsedInConditions.add(fieldName)

        if len(self.fields) == 1:
            code = commonFields.get(fieldName)
            if code is None: code = componentRegister.sensors.get(fieldName).getSystemwideID()
            if condition: condition.dependentOnRemoteSensors.add(code)
            return "value"

        s = componentRegister.sensors.get(self.name)
        assert s
        if condition: condition.dependentOnPackets.add(s)

        # return the name of the field in the packet
        return self.getNameCC() + "PacketBuffer." + fieldName

    def sortFields(self):
        for f in self.fields:
            cf = commonFields.get(f)
            if cf:
                self.sortedFields.append((cf, f))
                continue

            s = componentRegister.sensors.get(f)
            if s is None:
                componentRegister.userError("Field '{0}' not known for network component '{1}'\n".format(
                        f, self.name))
                self.isError = True
                return
            s.markAsUsed()
            self.sortedFields.append((s.getSystemwideID(), f))
        # sort the fields by ID
        self.sortedFields.sort()

        for s in self.sortedFields:
            self.typemask |= (1 << s[0])

    def generatePacketType(self, outputFile):
        if self.isError: return

        outputFile.write("struct {0}PacketBuffer_s {1}\n".format(self.getNameTC(), '{'))
        for f in self.sortedFields:
            outputFile.write("    uint32_t {};\n".format(f[1]))
        outputFile.write("} PACKED;\n")
        outputFile.write("typedef struct {0}PacketBuffer_s {0}PacketBuffer_t;\n".format(
                self.getNameTC()))

        outputFile.write("{0}PacketBuffer_t {1}PacketBuffer;\n\n".format(
                self.getNameTC(), self.getNameCC()))

    def generateVariables(self, outputFile):
        pass

    def generateReadFunction(self, field, outputFile):
        pass

    def generateReadFunctions(self, outputFile):
        # generate a getter function for each fields used in conditions
        # (they can be used ONLY in conditions or where clauses, not in regular outputs)
        for f in self.fieldsUsedInConditions:
            self.generateReadFunction(f, outputFile)

    # app main code is generated in conditions

######################################################
class ComponentRegister(object):
    module = None

    def __init__(self):
        self.printFunction = self.dummyPrint

    def addComponent(self, name, spec):
        s = None
        if spec._typeCode == self.module.TYPE_ACTUATOR:
            if name in self.actuators:
                return None
            s = self.actuators[name] = Actuator(name, spec)
        elif spec._typeCode == self.module.TYPE_SENSOR:
            if name in self.sensors:
                return None
            s = self.sensors[name] = Sensor(name, spec)
        elif spec._typeCode == self.module.TYPE_OUTPUT:
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
        self.networkComponents = {}
        self.systemParams = []
        self.systemStates = {}
        self.systemConstants = {}
        self.parameterDefines = {}
        self.virtualComponents = {}
        self.patterns = {}
        self.numCachedSensors = 0
        self.additionalConfig = set()
        self.extraSourceFiles = []
        self.branchCollection = BranchCollection()
        self.allSensorNames = dict(commonFields)
        self.isError = False
        self.architecture = architecture
        # import the module (residing in "components" directory and named "<architecture>.py")
        self.module = __import__(architecture)
        self.nextFreeSensorID = PACKET_FIELD_ID_FIRST_FREE
        # construct empty components from descriptions
        for spec in self.module.components:
            c = self.addComponent(spec._name.lower(), spec)
            if not c:
                self.userError("Component '{0}' duplicated for platform '{1}', ignoring\n".format(
                        spec._name, architecture))

    def loadExtModule(self, filename):
        try:
            extModule = __import__(filename)
        except Exception as ex:
            self.userError("Failed to load " + filename)
            return

        for p in dir(extModule):
            spec = extModule.__getattribute__(p)
            if isinstance(spec, self.module.SealComponent):
                c = self.addComponent(spec._name.lower(), spec)

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

    def isComponentUsed(self, componentName):
        c = self.findComponentByKeyword("use", componentName.lower())
        if c is None: return False
        return c.isUsed()

    def allocateSensorID(self, sensorName):
        result = self.nextFreeSensorID
        self.allSensorNames[sensorName] = self.nextFreeSensorID
        self.nextFreeSensorID += 1
        # values 31, 63, 95, 127 are reserved
        if ((self.nextFreeSensorID + 1) & 31) == 0:
            self.nextFreeSensorID += 1
        return result

    def getPacketFields(self, componentName):
        c = self.outputs.get(componentName, None)
        if c is None: return ""
        for u in c.outputUseCases:
            if not isinstance(u, FromFileOutputUseCase):
                return u.getPacketFields()
        return ""

    #######################################################################
    def lookupVirtualBase(self, basename, branchNumber):
        copyName = "__copy" + str(branchNumber) + "_" + basename
        result = self.virtualComponents.get(copyName)
        if result: return result
        return self.virtualComponents.get(basename)

    def addVirtualComponent(self, c):
        if self.virtualComponents.get(c.name, None) is None:
            if self.findComponentByName(c.name) is not None:
                self.userError("Virtual component '{0}' duplicated with real one for platform '{1}', ignoring\n".format(
                        c.prettyName(), self.architecture))
                c.isError = True
                return
            self.virtualComponents[c.name] = c
            return

        # so, we already have this component.

        if c.isImplicit:
            # assert self.virtualComponents.get(c.name, None).isImplicit
            # for autogenerated components this is  perfectly normal
            # (but don't add it twice!)
            c.isError = True
            return

        other = self.virtualComponents.get(c.name, None)
        # implicit defines should generate unique names, no?
        # assert not other.isImplicit

        if other.branchNumber == c.branchNumber:
            self.userError("Virtual component '{0}' duplicated for platform '{1}', ignoring\n".format(
                    c.prettyName(), self.architecture))
            c.isError = True
            return

        # same name, different branch. allow this; will add two or more use cases.
        other.alsoInBranches.add(c.branchNumber)
        other.alsoInBranchesConditions[c.branchNumber] = c.conditions
        other.alsoInCodeBlocks[c.branchNumber] = c.containingCodeBlock
        self.virtualComponents[c.name].alsoInBranches.add(c.branchNumber)
        #print self.virtualComponents[c.name]
        # generate unique name
        c.name = "__copy" + str(c.branchNumber) + "_" + c.name
        # add it to components as usual
        self.virtualComponents[c.name] = c

    def continueAddingVirtualComponent(self, c):
        # print "continue adding", c.name, c
        if c.isError or c.added: return

        c.added = True
        c.base = None

        # this is required because copy component don't have bases[] set up
        basenames = c.getAllBasenames()
        for basename in basenames:
            if c.name == basename:
                self.userError("Virtual component '{0}' depends on self, ignoring\n".format(
                        c.prettyName()))
                c.isError = True
                return

        numericalValue = None
        for basename in basenames:
            # print "    process", basename
            # constant value
            if basename[:7] == '__const':
                numericalValue = getNumberFromString(basename[7:])
                continue
            if basename in self.patterns: continue
            # a real sensor?
            base = self.findComponentByName(basename)
            if base is None or base.isVirtual():
                # a virtual sensor?
                virtualBase = self.lookupVirtualBase(basename, c.branchNumber)
                if virtualBase is None:
                    self.userError("Virtual component '{0}' has unknown base component '{1}', ignoring\n".format(
                            c.prettyName(), basename))
                    c.isError = True
                    return
                if virtualBase.name != basename:
                    c.fixBasename(basename, virtualBase.name)
                # add the virtual base first (because of parameters)
                self.continueAddingVirtualComponent(virtualBase)

        immediateBaseName = c.getImmediateBasename()
        immediateBase = self.lookupVirtualBase(immediateBaseName, c.branchNumber)
        if immediateBase is not None:
            if immediateBase.isError:
                self.userError("Virtual component '{0}' has invalid virtual base component '{1}', ignoring\n".format(
                        c.prettyName(), immediateBaseName))
                c.isError = True
                return

            # print "  immed base is virtual, in branch", immediateBase.branchNumber
            # inherit parameters from parent virtual sensor
            c.parameterDictionary = immediateBase.parameterDictionary

            # copy the branch number from base (XXX: scoping semantics?)
            c.branchNumber = immediateBase.branchNumber
            # inherit the base too
            c.base = immediateBase.base
        else:
            # print "  immed base is real"
            c.base = self.findComponentByName(immediateBaseName)
            assert c.base

        # fill the parameter dictionary
        for p in c.parameterList:
            paramName = c.base.specification.resolveAlias(p[0])
            if p[1] is not None:
                c.parameterDictionary[paramName] = p[1]
            else:
                c.parameterDictionary[paramName] = Value(True)

        # used for pseudocomponents that are based on integer literals or constants
        if c.numericalValue is not None:
            c.parameterDictionary["value"] = Value(c.numericalValue)

    def finishAddingVirtualComponent(self, c):
        if c.isError: return
        assert c.base
        # print "+++ finish adding", c.name, "base", c.base.name

        # add a real component
        s = self.addComponent(c.name, c.base.specification)
        if not s:
            self.userError("Virtual component '{0}' duplicated, ignoring\n".format(
                    c.prettyName()))
            return
        s.containingOutputComponent = c.containingOutputComponent

        s.updateParameters(c.parameterDictionary)
        s.functionTree = c.functionTree
        if s.functionTree.function == "sync":
            s.syncOnlySensor = True

    #######################################################################
    def chainVirtualComponentBases(self, c):
        basenames = c.getAllBasenames()
        for basename in basenames:
            if c.name == basename:
                self.userError("Virtual component '{0}' depends on self, ignoring\n".format(
                        c.prettyName()))
                c.isError = True
                return

        c.numericalValue = None
        c.bases = set()

        for basename in basenames:
            if basename[:7] == '__const':
                c.numericalValue = getNumberFromString(basename[7:])
                continue
            if basename in self.patterns: continue
            base = self.findComponentByName(basename)
            if base is None or base.isVirtual():
                # a virtual sensor
                virtualBase = self.virtualComponents.get(basename, None)
                if virtualBase is None:
                    if c.containingOutputComponent is not None:
                        # normal
                        continue
                    self.userError("Virtual component '{0}' has unknown base component '{1}', ignoring\n".format(
                            c.prettyName(), basename))
                    c.isError = True
                    return
                c.bases.add(virtualBase)

    def addAndPropagateDerived(self, base, derived):
        oldDerived = base.derived
        base.derived.update(derived)
        if oldDerived == base.derived: return # to avoid loops
        for b in base.bases:
            newDerived = b.derived
            newDerived.add(b)
            self.addAndPropagateDerived(b, newDerived)

    def chainVirtualComponentDerived(self, c):
        for b in c.bases:
            newDerived = c.derived
            newDerived.add(c)
            self.addAndPropagateDerived(b, newDerived)

    def addVirtualComponentsToBaseBranches(self, c):
        if len(c.alsoInBranches) == 0: return
        for br in c.alsoInBranches:
            # XXX: OK, semantics here is totally unclear.
            # Is something like true scoping required?
            # The problem in that cae is that ATM we cannot know which branches
            # of a component are "native" and which inherited
            for b in c.derived:
                b.alsoInBranches.add(br)
                b.alsoInBranchesConditions[br] = c.alsoInBranchesConditions[br]
                b.alsoInCodeBlocks[br] = c.alsoInCodeBlocks[br]

    def addVirtualComponentsToCodeBlocks(self, c):
        result = {}
        if len(c.alsoInBranches) == 0: return result
        for (branch, block) in c.alsoInCodeBlocks.items():
            if c.name not in block.componentDefines:
                cp = copy.copy(c)
                cp.name = "__copy" + str(branch) + "_" + c.name
                cp.alsoInBranches = set()
                cp.alsoInBranchesConditions = {}
                cp.alsoInCodeBlocks = {}
                cp.bases = set()
                cp.derived = set()
                cp.branchNumber = branch
                cp.addedByPreprocessor = True
                block.componentDefines[c.name] = cp
                result[cp.name] = cp
        return result

    def chainVirtualComponents(self):
        for c in self.virtualComponents.values():
            if c.name[:6] == '__copy': continue
            self.chainVirtualComponentBases(c)
        for c in self.virtualComponents.values():
            if c.name[:6] == '__copy': continue
            self.chainVirtualComponentDerived(c)
        for c in self.virtualComponents.values():
            if c.name[:6] == '__copy': continue
            self.addVirtualComponentsToBaseBranches(c)
        newVC = {}
        for c in self.virtualComponents.values():
            if c.name[:6] == '__copy': continue
            newVC.update(self.addVirtualComponentsToCodeBlocks(c))
        self.virtualComponents.update(newVC)

    #######################################################################
    def addRemote(self, name, basename):
        # print "add new component with name", name, "basename", basename
        base = self.sensors.get(basename, None)
        if base is None:
            self.userError("Network component '{0}' has no base sensor component for architecture '{1}'\n".format(
                    name, self.architecture))
            return None
        o = self.addComponent(name, base.specification)
        componentRegister.additionalConfig.add("seal_net")
        return o

    def addNetworkComponent(self, name, fields, conditions, branchNumber):
        if name in self.networkComponents:
            self.userError("Network component '{0}' duplicated, ignoring\n".format(name))
            return
        # print "addNetworkComponent, name=", name, " fields=", fields
        componentRegister.additionalConfig.add("seal_net")
        id = len(self.networkComponents)
        nc = NetworkComponent(name, fields, id)
        self.networkComponents[name] = nc

        s = None

        # add remote sensor for each field
        for f in fields:
            basename = "null" if f in commonFields else f
            s = self.addRemote(nc.getPrefix() + f, basename)
            s.networkComponent = nc

        # add remote sensor for all fields
        s = self.addRemote(name, basename = "null")

        if s:
            s.remoteFields = fields
            s.networkComponent = nc
            # XXX: why? add an use case to the sensor as well (with empty parameters) 
            # s.addUseCase({}, conditions, branchNumber)

    def reallyUseComponent(self, keyword, name, parameters, fields, conditions, branchNumber):
        # find the component
        o = self.findComponentByKeyword(keyword, name)
        if o is None:
            self.userError("Component '{0}' not known or not supported for architecture '{1}'\n".format(
                    name, self.architecture))
            return None
        # add use case to the component
        uc = o.addUseCase(parameters, conditions, branchNumber)
        if isinstance(o, Output):
            o.addOutputUseCase(uc, fields)
        return uc

    def useComponent(self, keyword, name, parameters, fields, conditions, branchNumber):
        # print "useComponent", name
        # print "self.virtualComponents = ", self.virtualComponents
        virtual = self.virtualComponents.get(name)
        # print self.virtualComponents[name]
        # print virtual.alsoInBranches
        if virtual is None or len(virtual.alsoInBranches) == 0:
            return self.reallyUseComponent(keyword, name, parameters, fields, conditions, branchNumber)

        if branchNumber == 0:
            # use for all the branches specied by component)
            self.reallyUseComponent(keyword, name, parameters, fields,
                                    virtual.conditions,
                                    virtual.branchNumber)
            for b in virtual.alsoInBranches:
                vname = "__copy" + str(b) + "_" + name
                self.reallyUseComponent(keyword, vname, parameters, fields,
                                        virtual.alsoInBranchesConditions[b], b)
        else:
            # use for a specific branch
            if virtual.branchNumber == branchNumber:
                self.reallyUseComponent(keyword, name, parameters, fields, conditions, branchNumber)
            elif branchNumber in virtual.alsoInBranches:
                vname = "__copy" + str(branchNumber) + "_" + name
                self.reallyUseComponent(keyword, vname, parameters, fields,
                                        virtual.alsoInBranchesConditions[branchNumber], branchNumber)
            else:
                self.userError("Component '{0}' is not defined for use in this branch!\n".format(name))

    def prepareToGenerateConstants(self):
        # print "prepareToGenerateConstants"
        for s in self.sensors.values():
            # print "process", s.name
            name = s.name
            if name[:6] == '__copy':
                name = name[7:]
                # what if branch has two or multiple digits?
                while name[0] != '_':
                    name = name[1:]
                name = name[1:]
            virtual = self.virtualComponents.get(name)
            if virtual is None or len(virtual.alsoInBranches) == 0:
                # print "  no virtual"
                continue
            # print "  is virtual"
            for b in virtual.alsoInBranches:
                vname = "__copy" + str(b) + "_" + name
                s1 = self.sensors.get(vname)
                #assert s1
                if not s1: continue
                s.alsoSensorIds.add(s1.getSystemwideID())
            s1 = self.sensors.get(name)
            # assert s1
            if not s1: continue
            s.alsoSensorIds.add(s1.getSystemwideID())

    def setState(self, name):
        # add the state itself, if not already
        if name not in self.systemStates:
            self.systemStates[name] = SystemState(name)
            name = "sealState_" + name
            # also add a kind of sensor, to later allow these state values to be read
            if name in self.sensors:
                self.userError("State '{0}' name duplicates a real sensors for architecture '{1}'\n".format(
                        name, self.architecture))
                return
            # create a custom specification for this sensor
            spec = copy.copy(self.sensors.get("null").specification)
            spec.useFunction.value = name
            spec.readFunction.value = spec.useFunction.value
            # spec.dataType = componentRegister.module.SealParameter(value.getType())
            # add the sensor to array
            self.sensors[name] = Sensor(name, spec)

    def generateVariables(self, outputFile):
        for s in self.systemStates.values():
            s.generateVariables(outputFile)
        for p in self.patterns.values():
            p.generateVariables(outputFile)

    def getAllComponents(self):
        return set(self.actuators.values()).union(set(self.sensors.values())).union(set(self.outputs.values()))

    def markSyncSensors(self):
        for s in self.sensors.values():
            if s.syncOnlySensor:
                s.addSubsensors()

    def markCachedSensors(self):
        self.numCachedSensors = 0
        for s in self.sensors.values():
            if s.testIsCacheNeeded(self.numCachedSensors):
                self.numCachedSensors += 1

    # find the first base component that has interrupts enabled
    def getInterruptBase(self, comp):
        if comp.name not in self.virtualComponents: return comp
        virt = self.virtualComponents[comp.name]
        immediateBaseName = virt.getImmediateBasename()
        immediateBase = self.lookupVirtualBase(immediateBaseName, virt.branchNumber)
        if not immediateBase: return comp
        c = self.findComponentByName(immediateBase.name)
        assert c
        if not c.isInterruptBased(): return comp
        return self.getInterruptBase(c)

    # inParameter - the component use case which is processed at the moment;
    #               None if in "when" block condition is processed instead
    def replaceCode(self, componentName, parameterName, condition, inParameter):
        # print "replace code: " + componentName + "." + parameterName

        n = self.networkComponents.get(componentName)
        if n:
            return n.replaceCode(parameterName, condition)

        s = self.systemStates.get(componentName, None)
        if s != None:
            if parameterName.lower() != 'value':
                self.userError("System state '{}' has only 'value' parameter, not '{}'!\n".format(componentName, parameterName))
                return "false"
            if condition:
                condition.dependentOnStates.add(s)
                s.dependentConditions.add(condition)
            return "sealState_" + componentName

        c = self.findComponentByName(componentName)
        if c == None:
            if parameterName.lower() == 'ispresent':
                return "false"
            else:
                if inParameter: return componentName
                if componentName in commonFields: return componentName # XXX
                self.userError("Component '{0}' not known\n".format(componentName))
                return "false"

        if c.isInterruptBased() and condition:
            b = self.getInterruptBase(c)
            condition.dependentOnInterrupts.add(b)
            b.conditionsDependentOnInterrupt.append(condition)

        if c.isNetworkBased() and condition:
            condition.dependentOnPackets.add(c)

        # found
        if parameterName.lower() == 'ispresent':
            return "true"

        if parameterName.lower() == 'iserror':
            errorFunction = c.getParameterValue("errorFunction", None)
            if errorFunction:
                c.markAsUsed()
                return errorFunction
            else:
                self.userError("Parameter '{0}' for component '{1}' has no error attribute!'\n".format(parameterName, componentName))
                return "false"

        if parameterName.lower() == 'value':
            if type(c) is not Sensor:
                if inParameter: return componentName
                self.userError("Parameter '{0}' for component '{1}' is not readable!\n".format(parameterName, componentName))
                return "false"

            # new code:
            if not c.isUsed():
#                if inParameter:
#                    print "inParameter of component called '", inParameter.name, "'"
#                else:
                self.userError("Sensor '{0}' used in an expression, but never read!\n".format(componentName))
                return "false"
            if condition: condition.dependentOnPeriodicSensors.add(c.getNameCC())
            return c.getNameCC() + "Value"

            # old code:
            # otherwise unused component might become usable because of use in condition.
            # c.markAsUsed()
            # test if cache is needed and if yes, update the flag
            # c.testIsCacheNeededForCondition()
            # return the right read function
            # return c.getNameCC() + "ReadProcess(&isFilteredOut)"

        if componentName == 'variables' or componentName == 'constants':
            # a global C variable, return its name (the user is responsible for correctness)
            return parameterName

        self.userError("Unknown parameter '{0}' for component '{1}'\n".format(parameterName, componentName))
        return "false"

    def userError(self, msg):
        self.isError = True
        self.printFunction("Error: " + msg)

    def userWarning(self, msg):
        self.printFunction("Warning: " + msg)

    def dummyPrint(self, msg):
        sys.stderr.write(msg)
        
######################################################
# global variables
componentRegister = ComponentRegister()
conditionCollection = ConditionCollection()

def clearGlobals():
    global componentRegister
    global conditionCollection
    componentRegister = ComponentRegister()
    conditionCollection = ConditionCollection()
