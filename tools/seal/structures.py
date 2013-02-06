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

import string, sys
########################################################

INDENT_STRING = "    "  # indent new code level with two spaces
def getIndent(indent):
    # take indent string n times
    return INDENT_STRING * indent

def suffixTransform(value, suffix):
    if suffix is None or suffix == '':
        return value
    if suffix == 's':
        return value * 1000 # seconds to milliseconds
    if suffix == 'min':
        return value * 60 * 1000 # minutes to milliseconds
    if suffix == 'ms':
        return value        # milliseconds to milliseconds
    if suffix == 'us':
        return value / 1000.  # milliseconds to microeconds
    # default case: ignore the suffix
    print("Unknown suffix '{0}' for value {1}\n".format(suffix, value))
    # TODO: componentRegister.userError("Unknown suffix '{0}' for value {1}\n".format(suffix, value))
    return value

def toCamelCase(s):
    if s == '': return ''
    return s[0].lower() + s[1:]

def toTitleCase(s):
    if s == '': return ''
    return s[0].upper() + s[1:]

def isConstant(s):
    return s[0] >= '0' and s[0] <= '9'

def orderNumToString(i):
    lst = ["first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eight", "ninth"]
    i -= 1
    if i >= 0 and i < len(lst):
        return lst[i]
    return str(i)

# a decorator that allows to set up and use static variables
def static_var(varname, value = 0):
    def decorate(func):
        setattr(func, varname, value)
        return func
    return decorate

def getNumberFromString(s):
    try:
        try:
            return int(s, 0)
        except ValueError:
            return int(bool(s))
    except ValueError:
        return float(s)

def typeIsString(s):
    if isinstance(s, str):
        return True
    if sys.version[0] < '3':
        return isinstance(s, unicode)
    return False

######################################################
class FunctionTree(object):
    def __init__(self, function, arguments):
        if typeIsString(function) \
                or isinstance(function, Value) \
                or isinstance(function, SealValue):
            # for normal functions / arguments
            self.parameterName = None
            self.function = function.lower()
        else:
            # for named parameters; a pair is passed instead
            # assert len(arguments) == 0
            self.parameterName = function[0].lower()
            self.function = function[1].lower()
        self.arguments = arguments

    def asConstant(self):
        if len(self.arguments):
            return None
        if isinstance(self.function, SealValue):
            return None
        const = self.function.getRawValue()
        if isinstance(const, int) \
                or isinstance(const, long) \
                or isinstance(const, float):
            return const
        return None

    def asString(self):
        if len(self.arguments):
            return None
        if typeIsString(self.function):
            return self.function
        return self.function.asString()

    def generateSensorName(self):
        if type(self.function) is Value:
            # do not allow '.' to appear is sensor names
            return self.function.asString().replace(".", "point")
        if type(self.function) is SealValue:
            return self.function.firstPart # XXX
        result = self.function.lower()
        for a in self.arguments:
            result += "_" + a.generateSensorName()
        return result

    def getCode(self):
        if type(self.function) is Value:
            return self.function.asString()
#        if type(self.function) is SealValue:
#            return self.function.firstPart # XXX
        args = []
        for a in self.arguments:
            args.append(a.getCode())
        return self.function.lower() + "(" + ",".join(args) + ")"

    def collectSensors(self):
        result = []
        for a in self.arguments:
            result += a.collectSensors()

        if type(self.function) is SealValue:
            result.append(self.function.firstPart)

        if typeIsString(self.function):
            if self.function[:7] != '__const':
                result.append(self.function)

        return set(result)

    def collectImplicitDefines(self, containingComponent):
        return []

########################################################
class ConditionCollection(object):
    def __init__(self):
        self.reset()

    def reset(self):
        self.conditionStack = []
        self.conditionList = []
        self.branchNumber = 0
        self.branchChanged = False
        self.codeList = []

    def add(self, condition):
        self.conditionList.append(condition)
        currentConditionNumber = len(self.conditionList)
        self.conditionStack.append(currentConditionNumber)
#        self.branchChanged = True

    def invertLast(self):
        last = self.conditionStack.pop()
        # signal *logical* negation by negative *integer* value
        self.conditionStack.append(-last)
#        self.branchChanged = True

    def size(self):
        return len(self.conditionStack)

    def totalConditions(self):
        return len(self.conditionList)

    def pop(self, n):
        while n > 0:
            self.conditionStack.pop()
            n -= 1

    def generateCodeForCondition(self, i, componentRegister):
        # TODO: at the moment, all subconditions are always evaluated.
        # for optimization, should return as soon as isFilteredOut becomes true.
        condition = self.conditionList[i]
        condition.id = i + 1
        return "    int8_t result = (bool)" + condition.getEvaluationCode(componentRegister)

    def generateCode(self, componentRegister):
        for i in range(len(self.conditionList)):
            self.codeList.append(self.generateCodeForCondition(i, componentRegister))

    def writeOutCodeForEventBasedCondition(self, condition, outputFile, branchCollection):
        if condition.dependentOnPeriodicSensors or condition.dependentOnStates:
            outputFile.write("static void condition{0}Callback(void)\n".format(condition.id))
        elif condition.dependentOnRemoteSensors:
            outputFile.write("static void condition{0}Callback(uint16_t code, int32_t value)\n".format(condition.id))
        elif condition.dependentOnInterrupts:
            outputFile.write("static void condition{0}Callback(void)\n".format(condition.id))
        else:
            # dependentOnPackets != None
            outputFile.write("static void condition{0}Callback(int32_t *value)\n".format(condition.id))
        outputFile.write("{\n")
        ID = condition.id - 1
        outputFile.write("    bool isFilteredOut = false;\n")
        outputFile.write(self.codeList[ID])
        outputFile.write("    if (isFilteredOut) return;\n")
        outputFile.write("    if (result == conditionStatus[{}]) return;\n".format(ID))

        # OK, the status has changed; start or stop associated code branches
        outputFile.write("    conditionStatus[{}] = result;\n".format(ID))
        outputFile.write("\n")

        outputFile.write("    bool newBranchStatus;\n")
        branchesAssociated = branchCollection.getAssociatedBranches(condition.id)
        for br in branchesAssociated:
            outputFile.write("    newBranchStatus = branch{}Evaluate();\n".format(br))
            outputFile.write("    if (newBranchStatus != branchStatus[{}]) {}\n".format(br, '{'))
            outputFile.write("        branchStatus[{}] = newBranchStatus;\n".format(br))
            outputFile.write("        if (newBranchStatus) branch{}Start();\n".format(br))
            outputFile.write("        else branch{}Stop();\n".format(br))
            outputFile.write("    }\n")
        outputFile.write("}\n\n")

    def writeOutCodeForStaticCondition(self, condition, outputFile):
        outputFile.write("static inline bool condition{0}Check(void)\n".format(condition.id))
        outputFile.write("{\n")
        outputFile.write("    bool isFilteredOut = false;\n")
        outputFile.write(self.codeList[condition.id - 1])
        outputFile.write("    return isFilteredOut ? false : result;\n")
        outputFile.write("}\n\n")

    def writeOutCodeForCondition(self, condition, outputFile, branchCollection):
        if condition.isEventBased():
            self.writeOutCodeForEventBasedCondition(condition, outputFile, branchCollection)

        if not condition.isEventBased() or condition.dependentOnStates:
            self.writeOutCodeForStaticCondition(condition, outputFile)

    def writeOutCode(self, outputFile, branchCollection):
        for c in self.conditionList:
            self.writeOutCodeForCondition(c, outputFile, branchCollection)

    def generateLocalFunctionsForCondition(self, condition, outputFile):
        if condition.dependentOnPeriodicSensors or condition.dependentOnStates:
            outputFile.write("static void condition{0}Callback(void);\n".format(condition.id))
        elif condition.dependentOnRemoteSensors:
            outputFile.write("static void condition{0}Callback(uint16_t code, int32_t value);\n".format(condition.id))
        elif condition.dependentOnInterrupts:
            outputFile.write("static void condition{0}Callback(void);\n".format(condition.id))
        elif condition.dependentOnPackets:
            outputFile.write("static void condition{0}Callback(int32_t *value);\n".format(condition.id))
        else:
            outputFile.write("static inline bool condition{0}Check(void);\n".format(condition.id))

    def generateLocalFunctions(self, outputFile):
        for c in self.conditionList:
            self.generateLocalFunctionsForCondition(c, outputFile)

    def onSensorRead(self, outputFile, sensorName):
        for c in self.conditionList:
            if sensorName in c.dependentOnPeriodicSensors:
                outputFile.write("        condition{}Callback();\n".format(c.id))

    def generateAppMainCodeForCondition(self, condition, outputFile):
        for code in condition.dependentOnRemoteSensors:
            outputFile.write("    sealNetRegisterInterest({}, condition{}Callback);\n".format(
                    code, condition.id))

        for component in condition.dependentOnPackets:
            outputFile.write("    {\n")
            outputFile.write("        const uint32_t typeMask = 0")
            for f in component.remoteFields:
                outputFile.write("\n            | {}_TYPE_MASK".format(f.upper()))
            outputFile.write(";\n")
            outputFile.write("        sealNetPacketRegisterInterest(typeMask, condition{}Callback, (int32_t *)&{}PacketBuffer);\n".format(
                    condition.id, component.name))
            outputFile.write("    }\n")

        if not condition.dependentOnPeriodicSensors \
                and not condition.dependentOnRemoteSensors \
                and not condition.dependentOnInterrupts \
                and not condition.dependentOnPackets:
            # a static (constant or state-based) condition.
            # evaluate it and start / stop coresponding branches (after all static have been evaluated)
            outputFile.write("    conditionStatus[{}] = condition{}Check();\n".format(
                    condition.id - 1, condition.id))

    def generateAppMainCode(self, outputFile):
        if len(self.conditionList):
            outputFile.write("\n")
        for c in self.conditionList:
            self.generateAppMainCodeForCondition(c, outputFile)
        if len(self.conditionList):
            outputFile.write("\n")

    def ensureBranchIsPresent(self, componentRegister):
        componentRegister.branchCollection.addBranch(
            self.branchNumber,
            self.conditionStack)
        self.branchChanged = True

    def nextBranch(self):
        if self.branchChanged:
            self.branchNumber += 1
#            self.branchChanged = False

########################################################
class Value(object):
    def __init__(self, value = None, suffix = None):
        if type(value) is Value:
            # decapsulate
            self.value = value.value
            self.suffix = value.suffix
        else:
            self.value = value
            self.suffix = suffix

    def lower(self):
        return self # already lower()'ed

    def getRawValue(self):
        if not (isinstance(self.value, int) or isinstance(self.value, long)):
            return self.value
        return suffixTransform(self.value, self.suffix)

    def getCode(self):
#        print "getCode for", self.value, self.suffix
        if self.value is None:
            return None
        assert not isinstance(self.value, Value)
        if isinstance(self.value, SealValue):
            return self.value.getCode()
        if typeIsString(self.value):
            # XXX?
            # return '"' + self.value + '"'
            return self.value
        # integer, boolean or real
        s = str(self.value).lower()
        if not (self.suffix is None):
            s += self.suffix
        return s

    def getCodeForGenerator(self, componentRegister, condition, inParameter):
        # print " Value", self.value
        if type(self.value) is SealValue:
            return self.value.getCodeForGenerator(componentRegister, condition, inParameter)
        return self.getCode()

    def getType(self):
        if typeIsString(self.value):
            return "const char *"
        if isinstance(self.value, bool):
            return "bool"
        return "int32_t"

    def asString(self):
        return self.getCode()
#        if type(self.value) is bool:
#            return str(self.value).lower()
#        # TODO: convert time values to milliseconds?
#        return str(self.value)

    def collectImplicitDefines(self, containingComponent):
        return []

########################################################
class SealValue(object):
    def __init__(self, firstPart, secondPart = None):
        if type(firstPart) is SealValue:
            # XXX: decapsulate it
            self.firstPart = firstPart.firstPart
            self.secondPart = firstPart.secondPart
        else:
            self.firstPart = firstPart
            self.secondPart = secondPart
#        print "seal value: ", self.firstPart, self.secondPart
#        print "  (args: ", firstPart, secondPart, ")"

    def lower(self):
        return SealValue(self.firstPart.lower(), self.secondPart)

    def asString(self):
        return self.firstPart

    def getCode(self):
        # Value object can be inside SealValue
        if "getCode" in dir(self.firstPart):
            result = self.firstPart.getCode()
        else:
            result = self.firstPart
        if self.secondPart:
            result += '.'
            result += self.secondPart
        return result

    def getCodeForGenerator(self, componentRegister, condition, inParameter):
        if isinstance(self.firstPart, Value):
            return self.firstPart.getCode()

        # print "SealValue: getCodeForGenerator", self.firstPart

        if condition and condition.dependentOnComponent:
            r = condition.dependentOnComponent.replaceCode(self.firstPart)
            if r: return r

        sp = self.secondPart
        if sp is None:
            sp = "value"
        return componentRegister.replaceCode(self.firstPart, sp, condition, inParameter)

########################################################
class Expression(object):
    def __init__(self, left = None, op = None, right = None):
        #  print "Expression: ", left, " ", op, " ", right
        if op == '=': op = '==' # hehe
        elif op == '<>': op = '!='
        self.op = op
        self.funcExpressionLeft = None
        self.funcExpressionRight = None
        if type(left) is FunctionTree:
            if len(left.arguments) == 0:
                self.left = Value(SealValue(left.function))
            else:
                self.left = Value(SealValue(left.generateSensorName()))
                self.funcExpressionLeft = left
        else:
            self.left = left
        if type(right) is FunctionTree:
            if len(right.arguments) == 0:
                self.right = Value(SealValue(right.function))
            else:
                self.right = Value(SealValue(right.generateSensorName()))
                self.funcExpressionRight = right
        else:
            self.right = right

        # -- the rest are for conditions only (expression in some contexts is a condition)
        # dependent on these periodic sensors
        self.dependentOnPeriodicSensors = set()
        # dependent on these self-reading sensors (e.g. remote, GPS?, etc.)
        self.dependentOnRemoteSensors = set()
        # dependent on these interrupts sensors
        self.dependentOnInterrupts = set()
        # dependent on these states
        self.dependentOnStates = set()
        # dependent on these network data sources
        self.dependentOnPackets = set()
        # used for "where" conditions, contains the output use case that has this condition
        self.dependentOnComponent = None

    def isEventBased(self):
        return bool(len(self.dependentOnPeriodicSensors)) \
            or bool(len(self.dependentOnStates)) \
            or bool(len(self.dependentOnRemoteSensors)) \
            or bool(len(self.dependentOnInterrupts)) \
            or bool(len(self.dependentOnPackets))

    def collectImplicitDefines(self, containingComponent):
        result = []
        if self.funcExpressionLeft:
            result.append(ComponentDefineStatement(
                    self.left.value.firstPart, self.funcExpressionLeft, [], True, containingComponent))
        elif self.left:
            result += self.left.collectImplicitDefines(containingComponent)
        if self.funcExpressionRight:
            result.append(ComponentDefineStatement(
                    self.right.value.firstPart, self.funcExpressionRight, [], True, containingComponent))
        elif self.right:
            result += self.right.collectImplicitDefines(containingComponent)
        return result

    def getCode(self):
        #print self.left
        #print self.op
        #print self.right
        if self.left != None and self.right != None:
            return self.left.getCode() + " " + self.op + " " + self.right.getCode()
        if self.op != None:
            return self.op + " " + self.right.getCode()
        if type(self.right) is Expression:
            return "(" + self.right.getCode() + ")"
        if "getCode" in dir(self.right):
            return self.right.getCode()
        return self.right

    def getCodeForGenerator(self, componentRegister, condition, inParameter):
        # print "getCodeForGenerator", self.right, self.op, self.left
        if self.left != None and self.right != None:
            result = self.left.getCodeForGenerator(componentRegister, condition, inParameter)
            result += " " + self.op + " "
            result += self.right.getCodeForGenerator(componentRegister, condition, inParameter)
            return result
        if self.op != None:
            return self.op + " " + self.right.getCodeForGenerator(componentRegister, condition, inParameter)
        if type(self.right) is Expression:
            return "(" + self.right.getCodeForGenerator(componentRegister, condition, inParameter) + ")"
#        print "self.right=", self.right, "[", self.right.asString(), "]"
#        if type(self.right) is Value:
#            print "self.right.value", self.right.value
#            return self.right.asString()
        if type(self.right) is Value:
            return self.right.getCodeForGenerator(componentRegister, condition, inParameter)
        return self.right

    def getEvaluationCode(self, componentRegister):
#        print "\n\ngetEvaluationCode"
#        print "getEvaluationCode for", self.right.right.right.value.firstPart
#        print "getEvaluationCode for", self
        code = self.getCodeForGenerator(componentRegister, self, None)
        return code.replace(" and ", "\n        && ")\
            .replace(" or ", "\n        || ")\
            .replace("(not ", "(! ") + ";\n"

    def asString(self):
        temp = self.getCode().split(" ", 1)
        return "{}, {}".format(filterParam[temp[0]], temp[1])

########################################################
class SystemParameter(object):
    def __init__(self, value):
        # self.name = name.lower()
        self.value = value

    def addComponents(self, componentRegister, conditionCollection):
        #if self.name in componentRegister.systemParams:
        #    userError("Parameter '{0}' already specified, ignoring\n".format(self.name))
        #    return
        #componentRegister.systemParams[self.name] = self
        componentRegister.systemParams.append(self)

    def getCode(self, indent):
        return "config " + self.value.getCode() + ';'

    def getConfigLine(self):
        return self.value.getCode().strip('"')

########################################################
class PatternDeclaration(object):
    def __init__(self, name, values):
        self.name = name.lower()
        self.values = values
        self.isUsec = False
        for v in self.values:
            if v.suffix == "us":
                self.isUsec = True
                break

    def addComponents(self, componentRegister, conditionCollection):
        if self.name in componentRegister.patterns:
            componentRegister.userError("Pattern {0} already specified, ignoring\n".format(self.name))
            return
        componentRegister.patterns[self.name] = self

    def getCode(self, indent):
        result = "pattern " + self.name + " ("
        assert len(self.values)
        for v in self.values[:-1]:
            result += v.getCode() + ', '
        result += self.values[-1].getCode()
        result += ');'
        return result

    def getSize(self):
        return len(self.values)

    def getVariableName(self):
        return "pattern_" + self.name

    def generateVariables(self, outputFile):
        if self.isUsec: arrayType = "double"
        else: arrayType = "uint32_t"
        outputFile.write("{} {}[] = {}\n".format(arrayType, self.getVariableName(), '{'));
        # outputFile.write(string.join(map(lambda x: "    " + str(x.getRawValue()), self.values), ",\n"))
        values = ["    " + str(x.getRawValue()) for x in self.values]
        outputFile.write(",\n".join(values))
        outputFile.write("\n};\n");
        outputFile.write("uint_t pattern_{0}Cursor = 0;\n".format(self.name));

########################################################
class ConstStatement(object):
    def __init__(self, name, value):
        self.name = name.lower()
        self.value = value

    def addComponents(self, componentRegister, conditionCollection):
        pass

    def getCode(self, indent):
        return "const " + self.name + " " + self.value.getCode() + ';'

########################################################
class SetStatement(object):
    def __init__(self, name, expression):
        self.name = name.lower()
        self.expression = expression

    def addComponents(self, componentRegister, conditionCollection):
        self.conditionStack = list(conditionCollection.conditionStack)
        self.branchNumber = conditionCollection.branchNumber
        componentRegister.setState(self.name)

    def finishAdding(self, componentRegister):
        componentRegister.systemStates[self.name].addUseCase(
            self.expression,
            self.conditionStack,
            self.branchNumber)

    def getCode(self, indent):
        return "set " + self.name + " " + self.expression.getCode() + ';'

    def collectImplicitDefines(self):
        return self.expression.collectImplicitDefines(None)

########################################################
class NetworkReadStatement(object):
    def __init__(self, name, fields):
        self.name = name.lower()
        self.fields = []
        # f[0] - name, f[1] - count (integer)
        for f in fields:
            if f[1] != 1:
                componentRegister.userError("Field '{}' specified multiple times in NetworkRead '{}'\n".format(f[0], name))
            self.fields.append(f[0].lower())

    def addComponents(self, componentRegister, conditionCollection):
        componentRegister.addNetworkComponent(self.name, self.fields,
                                              conditionCollection.conditionStack,
                                              conditionCollection.branchNumber)

    def getCode(self, indent):
        result = "NetworkRead " + self.name
        if len(self.fields):
            result += " ("
            for f in self.fields:
                result += f[0] + ", "
            result = result[:-2] # remove last comma
            result += ")"
        result += ';'
        return result

########################################################
class ComponentDefineStatement(object):
    def __init__(self, name, functionTree, parameters, isImplicit, containingOutputComponent = None):
        # print "ComponentDefineStatement", name
        self.name = name.lower()
        self.functionTree = functionTree
        self.parameterList = parameters
        self.parameterDictionary = {}
        self.added = False
        self.isError = False
        self.base = None # underlying "real" component
        self.numericalValue = None # in case this a constant
        self.isImplicit = isImplicit
        self.containingOutputComponent = containingOutputComponent
        # to deal with multiple defines inside branches
        self.alsoInBranches = set()
        self.alsoInBranchesConditions = {}
        self.alsoInCodeBlocks = {}
        # underlying virtual components
        self.bases = set()
        # derived virtual components
        self.derived = set()
        self.addedByPreprocessor = True

    def prettyName(self):
        if self.name[:6] == '__copy':
            return self.name[8:]
        return self.name

    def getCode(self, indent):
        result = "define " + self.name
        # TODO
#        for p in self.parameterList:
#            result += " "
#            result += p[0]
#            if p[1] != None:
#                result += " "
#                result += p[1].getCode()
#            result += ","
#        if self.parameterList != []:
#            result = result[:-1] # remove last comma
#        result += ';'
#        return result

    def addComponents(self, componentRegister, conditionCollection):
        # print "ComponentDefineStatement", self.name, ": addComponents"
        self.conditions = list(conditionCollection.conditionStack)
        self.branchNumber = conditionCollection.branchNumber
        componentRegister.addVirtualComponent(self)

    def continueAdding(self, componentRegister):
        # print "ComponentDefineStatement", self.name, ": continueAdding"
        componentRegister.continueAddingVirtualComponent(self)

    def finishAdding(self, componentRegister):
        # print "ComponentDefineStatement", self.name, ": finishAdding"
        componentRegister.finishAddingVirtualComponent(self)

    def getAllBasenamesRecursively(self, functionTree):
        if len(functionTree.arguments) == 0:
            # in this (default) case of 0-ary function, function name = sensor or constant name
            # TODO: replace CONST defines before this!
#            print "  get basename functionTree.function:" ,functionTree.function
            if isinstance(functionTree.function, Value):
#                print "    ", functionTree.function.getRawValue()
                return ["__const" + str(functionTree.function.getRawValue())]
            if isinstance(functionTree.function, SealValue):
#                print "    seal value", functionTree.function.firstPart
                return [functionTree.function.firstPart]
            return [functionTree.function]
        basenames = []
        for a in functionTree.arguments:
            basenames += self.getAllBasenamesRecursively(a)
        return basenames

    def fixBasenameRecursively(self, functionTree, old, new):
        newTree = FunctionTree(functionTree.function, [])
        if isinstance(newTree.function, SealValue):
            if newTree.function.firstPart == old:
                newTree.function.firstPart = new
        elif typeIsString(newTree.function):
            if newTree.function == old:
                newTree.function = new
        for a in functionTree.arguments:
            newTree.arguments.append(self.fixBasenameRecursively(a, old, new))
        return newTree

    def fixBasename(self, old, new):
        self.functionTree = self.fixBasenameRecursively(self.functionTree, old, new)

    def getAllBasenames(self):
#        basenames = self.getAllBasenamesRecursively(self.functionTree)
#        return map(lambda x: self.correctBasename(x), basenames)
        return self.getAllBasenamesRecursively(self.functionTree)

    def getImmediateBasenameRecursively(self, functionTree):
        # no function
        if len(functionTree.arguments) == 0:
#            print "  get immed functionTree.function:" ,functionTree.function
            if isinstance(functionTree.function, Value):
#                print "    ", functionTree.function.value
                return "constant"
            if isinstance(functionTree.function, SealValue):
#                print "    seal value", functionTree.function.firstPart
                return functionTree.function.firstPart
            return functionTree.function
        # unary function
        if len(functionTree.arguments) == 1:
            return self.getImmediateBasenameRecursively(functionTree.arguments[0])
        # n-ary function; may return null sensor as base, if more than one are used
        # TODO: not all n-ary functions use multiple sensors!!!
        if functionTree.function == "match" \
                or functionTree.function == "neg" \
                or functionTree.function == "abs" \
                or functionTree.function == "map" \
                or functionTree.function == "power" \
                or functionTree.function == "square" \
                or functionTree.function == "sqrt" \
                or functionTree.function == "take" \
                or functionTree.function == "takeRecent" \
                or functionTree.function == "avg" \
                or functionTree.function == "stdev" \
                or functionTree.function[:6] == "filter" \
                or functionTree.function[:6] == "invert":
            return self.getImmediateBasenameRecursively(functionTree.arguments[0])
        return "null"

    def getImmediateBasename(self):
        return self.getImmediateBasenameRecursively(self.functionTree)


########################################################
class ParametersDefineStatement(object):
    def __init__(self, name, parameters):
        self.name = name.lower()
        self.parameters = parameters

    def getCode(self, indent):
        result = "parameters " + self.name
        for p in self.parameters:
            result += " "
            result += p[0]
            if p[1] != None:
                result += " "
                result += p[1].getCode()
            result += ","
        if self.parameters != []:
            result = result[:-1] # remove last comma
        result += ';'
        return result

    def addComponents(self, componentRegister, conditionCollection):
        if self.name in componentRegister.parameterDefines:
            componentRegister.userError("Parameter define {0} already specified, ignoring\n".format(self.name))
            return
        componentRegister.parameterDefines[self.name] = self

########################################################
class LoadStatement(object):
    def __init__(self, filename):
        self.filename = filename

    def getCode(self, indent):
        return 'load "' + self.filename + '"'

    def load(self, componentRegister):
        fixedFilename = self.filename.split(".")
        if len(fixedFilename) > 1:
            if fixedFilename[-1] == 'c':
                # it's a C file
                componentRegister.extraSourceFiles.append(self.filename)
                return
        # it should be a Python file
        componentRegister.loadExtModule(fixedFilename[0])

########################################################
class ComponentUseCase(object):
    def __init__(self, type_, name, parameters, fields):
        self.type = type_.lower()
        self.expression = None
        if isinstance(name, FunctionTree):
            if len(name.arguments) == 0:
                if isinstance(name.function, SealValue):
                    self.name = name.function.firstPart
                else:
                    self.name = name.function
                self.name = self.name.lower()
            else:
                self.name = name.generateSensorName()
                self.expression = name
        else:
            self.name = name.lower()
        self.parameters = dict(parameters)
        self.fields = []
        if fields:
            # f[0] - name, f[1] - count (integer)
            for f in fields: self.fields.append((f[0].lower(), f[1]))
        self.componentUseCase = None

    def getCode(self, indent):
        result = self.type + " " + self.name
        if len(self.fields):
            result += " ("
            for f in self.fields:
                result += f[0]
                if f[1] != 1: result += str(f[1])
                result += ", "
            result = result[:-2] # remove last comma
            result += ")"
        for p in self.parameters.items():
            result += ", "
            result += p[0]
            if p[1] != None:
                result += " "
                result += p[1].getCode()
        result += ';'
        return result

    def addUseCase(self, componentRegister, conditionCollection):
        self.componentUseCase = \
            componentRegister.useComponent(self.type, self.name,
                                       self.parameters, self.fields,
                                       conditionCollection.conditionStack,
                                       conditionCollection.branchNumber)

    def collectImplicitDefines(self):
        implicitDefines = []
        if self.expression: 
            implicitDefines.append(ComponentDefineStatement(self.name, self.expression, [], True))
        if self.type == "output" and "where" in self.parameters:
            implicitDefines += self.parameters["where"].collectImplicitDefines(self)
        return implicitDefines


########################################################
CODE_BLOCK_TYPE_PROGRAM = 0
CODE_BLOCK_TYPE_WHEN = 1
CODE_BLOCK_TYPE_ELSEWHEN = 2
CODE_BLOCK_TYPE_ELSE = 3

class CodeBlock(object):
    def __init__(self, blockType, condition, declarations, nextBlock):
        self.blockType = blockType
        self.condition = condition
        self.declarations = declarations
        self.componentDefines = {}
        self.nextBlock = nextBlock

    def getCode(self, indent):
        result = getIndent(indent)
        if self.blockType == CODE_BLOCK_TYPE_WHEN:
            result += "when"
            result += " " + self.condition.getCode()
            result += ":\n"
            indent += 1
        elif self.blockType == CODE_BLOCK_TYPE_ELSEWHEN:
            result += "elsewhen"
            result += " " + self.condition.getCode()
            result += ":\n"
            indent += 1
        elif self.blockType == CODE_BLOCK_TYPE_ELSE:
            result += "else:\n"
            indent += 1

        for d in self.declarations:
            # IDE -> unfinished condition '1<'
            if d == None:
                continue
            result += getIndent(indent)
            result += d.getCode(indent)
            result += "\n"
        # recursive call
        if self.nextBlock != None:
            assert indent > 0
            indent -= 1
            result += getIndent(indent)
            result += self.nextBlock.getCode(indent)
            result += "\n"
        elif self.blockType != CODE_BLOCK_TYPE_PROGRAM:
            result += "end\n"

        return result

    def enterCodeBlock(self, componentRegister, conditionCollection):
        stackStartSize = conditionCollection.size()
        if self.blockType == CODE_BLOCK_TYPE_PROGRAM:
            # reset all
            conditionCollection.reset()
        elif self.blockType == CODE_BLOCK_TYPE_WHEN:
            # append new
            conditionCollection.add(self.condition)
        elif self.blockType == CODE_BLOCK_TYPE_ELSEWHEN:
            # invert previous...
            conditionCollection.invertLast()
            # ..and append new
            conditionCollection.add(self.condition)
        elif self.blockType == CODE_BLOCK_TYPE_ELSE:
            # just invert previous
            conditionCollection.invertLast()

        # make sure this code branch gets a unique number
        conditionCollection.ensureBranchIsPresent(componentRegister)

        return stackStartSize

    def exitCodeBlock(self, componentRegister, conditionCollection, stackStartSize):
        if self.blockType == CODE_BLOCK_TYPE_WHEN:
            # pop all conditions from this code block
            conditionCollection.pop(conditionCollection.size() - stackStartSize)


    def addComponents(self, componentRegister, conditionCollection):
        ss = self.enterCodeBlock(componentRegister, conditionCollection)

        # add implicitly declared virtual sensors
        implicitDefines = []
        for d in self.declarations:
            if isinstance(d, ComponentUseCase) or isinstance(d, SetStatement):
                implicitDefines += d.collectImplicitDefines()
        self.declarations += implicitDefines
        # collect implicit defines from conditions too...
        if self.condition:
            self.declarations += self.condition.collectImplicitDefines(None)

        # add components - use cases always first, "when ..." blocks afterwards!
        for d in self.declarations:
            if type(d) is SystemParameter:
                if self.blockType != CODE_BLOCK_TYPE_PROGRAM:
                    componentRegister.userError("System configuration supported only in top level, ignoring '{0}'\n".format(
                            d.getConfigLine()))
                else:
                    d.addComponents(componentRegister, conditionCollection)
            if type(d) is PatternDeclaration:
                if self.blockType != CODE_BLOCK_TYPE_PROGRAM:
                    componentRegister.userError("Pattern declarations supported only in top level, ignoring pattern '{0}'\n".format(
                            d.name))
                else:
                    d.addComponents(componentRegister, conditionCollection)

            if type(d) is ComponentDefineStatement:
                # find unused name; this is needed because there can be
                # multiple sensor read with the same implicit define (want to add them all);
                # on the other hand, for defines-in-branch the branch code (componentDefines)
                # is searched for a specific name.
                key = d.name
                i = 1
                while key in self.componentDefines:
                    key = "__" + d.name + str(i)
                    i += 1
                self.componentDefines[key] = d
                d.containingCodeBlock = self
                d.addComponents(componentRegister, conditionCollection)

            if type(d) is ParametersDefineStatement \
                    or type(d) is NetworkReadStatement \
                    or type(d) is SetStatement:
                d.addComponents(componentRegister, conditionCollection)

            if type(d) is LoadStatement:
                if self.blockType != CODE_BLOCK_TYPE_PROGRAM:
                    componentRegister.userError("Load statements supported only in top level, ignoring '{0}'\n".format(
                            d.getConfigLine()))
                else:
                    d.load(componentRegister)

        # add set statements (may depend on virtual components)
#        for d in self.declarations:
#            if type(d) is SetStatement:
#                d.addComponents(componentRegister, conditionCollection)

        conditionCollection.nextBranch()

        # recursive call (for included "when" statements)
        for d in self.declarations:
            if type(d) is CodeBlock:
                d.addComponents(componentRegister, conditionCollection)

        # recursive call (for next "elsewhen" or "else" branch)
        if self.nextBlock != None:
            self.nextBlock.addComponents(componentRegister, conditionCollection)

        self.exitCodeBlock(componentRegister, conditionCollection, ss)


    def addVirtualComponents(self, componentRegister, conditionCollection):
        ss = self.enterCodeBlock(componentRegister, conditionCollection)

        for d in self.componentDefines.values():
            d.continueAdding(componentRegister)
        for d in self.componentDefines.values():
            d.finishAdding(componentRegister)

        # add set statements (may depend on virtual components)
        for d in self.declarations:
            if type(d) is SetStatement:
                d.finishAdding(componentRegister)

        conditionCollection.nextBranch()

        # recursive call (for included "when" statements)
        for d in self.declarations:
            if type(d) is CodeBlock:
                d.addVirtualComponents(componentRegister, conditionCollection)

        # recursive call (for next "elsewhen" or "else" branch)
        if self.nextBlock != None:
            self.nextBlock.addVirtualComponents(componentRegister, conditionCollection)

        self.exitCodeBlock(componentRegister, conditionCollection, ss)


    def addUseCases(self, componentRegister, conditionCollection):
        ss = self.enterCodeBlock(componentRegister, conditionCollection)

        # add use cases
        for d in self.declarations:
            if type(d) is ComponentUseCase:
                d.addUseCase(componentRegister, conditionCollection)

        conditionCollection.nextBranch()

        # recursive call (for included "when" statements)
        for d in self.declarations:
            if type(d) is CodeBlock:
                d.addUseCases(componentRegister, conditionCollection)

        # recursive call (for next "elsewhen" or "else" branch)
        if self.nextBlock != None:
            self.nextBlock.addUseCases(componentRegister, conditionCollection)

        self.exitCodeBlock(componentRegister, conditionCollection, ss)


    def add(self, componentRegister, conditionCollection):
        self.addComponents(componentRegister, conditionCollection)
        componentRegister.chainVirtualComponents()
        self.addVirtualComponents(componentRegister, conditionCollection)
        self.addUseCases(componentRegister, conditionCollection)
