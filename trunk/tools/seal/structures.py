import string, sys
########################################################

INDENT_STRING = "    "  # indent new code level with two spaces
def getIndent(indent):
    # take indent string n times
    return INDENT_STRING * indent

#def toMilliseconds(x, componentRegister):
#    if type(x) is int: return x
#    value = x.value
#    if x.suffix is None or x.suffix == '' or x.suffix == "ms":
#        pass
#    elif x.suffix == "s" or x.suffix == "sec":
#        value *= 1000
#    else:
#        componentRegister.userError("Unknown suffix '{0}' for time value\n".format(x.suffix))
#    return value

def suffixTransform(value, suffix):
    if suffix is None or suffix == '':
        return value
    if suffix == 's':
        return value * 1000 # seconds to milliseconds
    if suffix == 'ms':
        return value        # milliseconds to milliseconds
    if suffix == 'us':
        return value / 1000.  # milliseconds to microeconds
    # default case: ignore the suffix
    print "Unknown suffix '{0}' for value {1}\n".format(suffix, value)
    # TODO: componentRegister.userError("Unknown suffix '{0}' for value {1}\n".format(suffix, value))
    return value

def toCamelCase(s):
    if s == '': return ''
    return string.lower(s[0]) + s[1:]

def toTitleCase(s):
    if s == '': return ''
    return string.upper(s[0]) + s[1:]

def isConstant(s):
    return s[0] >= '0' and s[0] <= '9'

# a decorator that allows to set up and use static variables
def static_var(varname, value = 0):
    def decorate(func):
        setattr(func, varname, value)
        return func
    return decorate

######################################################
class FunctionTree(object):
    def __init__(self, function, arguments):
#        if isinstance(function, SealValue):
#            print "  FunctionTree from SealValue!"
#            self.function = function.firstPart
#        else:
#            self.function = function
        self.function = function.lower()
        self.arguments = arguments

    def checkArgs(self, argCount, componentRegister):
        if len(self.arguments) != argCount:
            componentRegister.userError("Function {}(): {} arguments expected, {} given!\n".format(
                    self.function, argCount, len(self.arguments)))
            return False
        return True

    def asConstant(self):
        if len(self.arguments):
            return None
        const = self.function.getRawValue()
        if isinstance(const, int):
            return const
        return None
#        try:
#            return int(self.function.value)
#        except Exception:
#            return None

    def generateSensorName(self):
        if type(self.function) is Value:
            return self.function.asString()
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

#    def getEvaluationCode(self, componentRegister):
#        return self.getCode()

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
        self.branchChanged = True

    def invertLast(self):
        last = self.conditionStack.pop()
        # signal *logical* negation by negative *integer* value
        self.conditionStack.append(-last)
        self.branchChanged = True

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
        return "    bool result = " + condition.getEvaluationCode(componentRegister)

    def generateCode(self, componentRegister):
        for i in range(len(self.conditionList)):
            self.codeList.append(self.generateCodeForCondition(i, componentRegister))

    def writeOutCodeForEventBasedCondition(self, condition, outputFile):
        if condition.dependentOnSensors:
            outputFile.write("static void condition{0}Callback(uint16_t code, int32_t values)\n".format(condition.id))
        else:
            outputFile.write("static void condition{0}Callback(int32_t *values)\n".format(condition.id))
        outputFile.write("{\n")
        outputFile.write("    bool isFilteredOut = false;\n")
        outputFile.write(self.codeList[condition.id - 1])
        outputFile.write("    if (isFilteredOut) return;\n")
        outputFile.write("    if (result == oldConditionStatus[{}]) return;\n".format(condition.id))
        # OK, the status has changed; start or stop the code branch
        outputFile.write("\n")
        outputFile.write("    oldConditionStatus[{}] = result;\n".format(condition.id))
        outputFile.write("    if (result) branch{0}Start();\n".format(condition.id))
        outputFile.write("    else branch{0}Stop();\n".format(condition.id))
        outputFile.write("}\n\n")

        # this function does nothing
        outputFile.write("static inline bool condition{0}Check(bool oldValue)\n".format(condition.id))
        outputFile.write("{\n")
        outputFile.write("    return oldValue;\n")
        outputFile.write("}\n\n")

    def writeOutCodeForPeriodicCondition(self, condition, outputFile):
        outputFile.write("static inline bool condition{0}Check(bool oldValue)\n".format(condition.id))
        outputFile.write("{\n")
        outputFile.write("    bool isFilteredOut = false;\n")
        outputFile.write(self.codeList[condition.id - 1])
        outputFile.write("    return isFilteredOut ? oldValue : result;\n")
        outputFile.write("}\n\n")

    def writeOutCodeForCondition(self, condition, outputFile):
        if condition.isEventBased():
            self.writeOutCodeForEventBasedCondition(condition, outputFile)
        else:
            self.writeOutCodeForPeriodicCondition(condition, outputFile)

    def writeOutCode(self, outputFile):
        for c in self.conditionList:
            self.writeOutCodeForCondition(c, outputFile)

    def generateAppMainCodeForCondition(self, condition, outputFile):
        for code in condition.dependentOnSensors:
            outputFile.write("    sealCommRegisterInterest({}, condition{}Callback);\n".format(
                    code, condition.id))

        for component in condition.dependentOnPackets:
            outputFile.write("    {\n")
            outputFile.write("        static int32_t buffer[{}];\n".format(len(component.remoteFields)))
            outputFile.write("        const uint32_t typeMask = 0")
            for f in component.remoteFields:
                outputFile.write("\n            | {}_TYPE_MASK".format(f.upper()))
            outputFile.write(";\n")
            outputFile.write("        sealCommPacketRegisterInterest(typeMask, condition{}Callback, buffer);\n".format(
                    condition.id))
            outputFile.write("    }\n")

    def generateAppMainCode(self, outputFile):
        for c in self.conditionList:
            self.generateAppMainCodeForCondition(c, outputFile)


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
        if not isinstance(self.value, int):
            return self.value
        return suffixTransform(self.value, self.suffix, )

    def getCode(self):
#        print "getCode for", self.value, self.suffix
        if self.value is None:
            return None
        assert not isinstance(self.value, Value)
        if isinstance(self.value, SealValue):
            return self.value.getCode()
        if isinstance(self.value, str):
            return '"' + self.value + '"'
        # integer or boolean
        s = string.lower(str(self.value))
        if not (self.suffix is None):
            s += self.suffix
        return s

    def getCodeForGenerator(self, componentRegister, condition):
#        print "getCodeForGenerator for ", self.value
        if type(self.value) is SealValue:
            return self.value.getCodeForGenerator(componentRegister, condition)
        return self.getCode()

    def getType(self):
        if type(self.value) is str:
            return "const char *"
        if type(self.value) is bool:
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

    def getCodeForGenerator(self, componentRegister, condition):
        if isinstance(self.firstPart, Value):
            return self.firstPart.getCode()

#        print "SealValue: getCodeForGenerator", self.firstPart

        if condition.dependentOnComponent:
            r = condition.dependentOnComponent.replaceCode(self.firstPart)
            if r: return r

        sp = self.secondPart
        if sp is None:
            sp = "value"
        return componentRegister.replaceCode(self.firstPart, sp, condition)

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

        # -- the rest are for conditiions opnly (expression in some contexts is a condition)
        # dependent on these self-reading sensors
        self.dependentOnSensors = set()
        # dependent on these network data sources
        self.dependentOnPackets = set()
        # used for "where" conditions, contains the output use case that has this condition
        self.dependentOnComponent = None

    def isEventBased(self):
        return bool(len(self.dependentOnSensors)) \
            or bool(len(self.dependentOnPackets))

    def collectImplicitDefines(self, containingComponent):
        result = []
        if self.funcExpressionLeft:
            result.append(ComponentDefineStatement(
                    self.left.value.firstPart, self.funcExpressionLeft, [], containingComponent))
        elif self.left:
            result += self.left.collectImplicitDefines(containingComponent)
        if self.funcExpressionRight:
            result.append(ComponentDefineStatement(
                    self.right.value.firstPart, self.funcExpressionRight, [], containingComponent))
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

    def getCodeForGenerator(self, componentRegister, condition):
        #print "getCodeForGenerator", self
        if self.left != None and self.right != None:
            result = self.left.getCodeForGenerator(componentRegister, condition)
            result += " " + self.op + " "
            result += self.right.getCodeForGenerator(componentRegister, condition)
            return result
        if self.op != None:
            return self.op + " " + self.right.getCodeForGenerator(componentRegister, condition)
        if type(self.right) is Expression:
            return "(" + self.right.getCodeForGenerator(componentRegister, condition) + ")"
#        print "self.right=", self.right, "[", self.right.asString(), "]"
#        if type(self.right) is Value:
#            print "self.right.value", self.right.value
#            return self.right.asString()
        if type(self.right) is Value:
            return self.right.getCodeForGenerator(componentRegister, condition)
        return self.right

    def getEvaluationCode(self, componentRegister):
#        print "getEvaluationCode for", self.right, self.left
#        print "getEvaluationCode for", self.right.right.right.value.firstPart
#        print "getEvaluationCode for", self
        code = self.getCodeForGenerator(componentRegister, self)
        return string.replace(string.replace(string.replace(
                    code,
                    " and ", "\n        && "),
                    " or ", "\n        || "),
                    " not ", " ! ") + ";\n"

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

    def generateVariables(self, outputFile):
        if self.isUsec: arrayType = "double"
        else: arrayType = "uint32_t"
        outputFile.write("{} __pattern_{}[] = {}\n".format(arrayType, self.name, '{'));
        outputFile.write(string.join(map(lambda x: "    " + str(x.getRawValue()), self.values), ",\n"))
        outputFile.write("\n};\n");
        outputFile.write("uint_t __pattern_{0}Cursor = 0;\n".format(self.name));

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
        componentRegister.setState(self.name, self.expression,
                                   conditionCollection.conditionStack,
                                   conditionCollection.branchNumber)

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
    def __init__(self, name, functionTree, parameters, containingOutputComponent = None):
        self.name = name.lower()
#        self.basename = basename.lower()
        self.functionTree = functionTree
        self.parameterList = parameters
        self.parameterDictionary = {}
        self.added = False
        self.isError = False
        self.base = None # underlying "real" component
        self.containingOutputComponent = containingOutputComponent
#        self.bases = []

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
        componentRegister.addVirtualComponent(self)

    def continueAdding(self, componentRegister):
        componentRegister.continueAddingVirtualComponent(self)

    def finishAdding(self, componentRegister):
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

    def getAllBasenames(self):
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
        fixedFilename = string.split(self.filename, ".")
        if len(fixedFilename) > 1:
            if fixedFilename[-1] == 'c':
                # it's a C file
                componentRegister.extraSourceFiles.append(self.filename)
                return
        # it should be a Python file
        componentRegister.loadExtModule(fixedFilename[0])

########################################################
#class ProcessStatement(object):
#    def __init__(self, name, target, parameters):
#        self.name = name.lower()
#        self.target = target.lower()
#        self.parameters = parameters

#    def getCode(self, indent):
#        result = "process {} {},".format(self.name, self.target)
#        for p in self.parameters:
#            result += " "
#            result += p[0]
#            if p[1] != None:
#                result += " "
#                result += p[1].getCode()
#            result += ","
#        if self.parameters != []:
#            result = result[:-1] # remove last comma
#        result += ';'
#        return reslt

#    def generateVariables(self, processStructInits, outputFile):
#        outputFile.write ("uint16_t {} = 0;\n".format(self.name));
#        for x in self.parameters:
#            if x[0] in ['average', 'filter', 'stdev']:
#                outputFile.write ("{}{}_t {}{}{};\n".format(
#                    x[0][0].upper(), x[0][1:], x[0], self.name[0].upper(),
#                    self.name[1:]));
#                processStructInits.append("    {}{}{} = {}Init({});\n".format(
#                    x[0], self.name[0].upper(), self.name[1:], x[0], x[1].asString()))

#    def generateCallbackCode(self, outputFile, inputValue, indent, recursiveCall):
#        lastVal = inputValue
#        myIndent = indent
#        haveFilter = False
#        for x in self.parameters:
#            name = x[0] + self.name[0].upper() + self.name[1:]
#            if x[0] == 'filter':
#                haveFilter = True
#                outputFile.write(getIndent(myIndent))
#                outputFile.write("if (addFilter(&{}, &{})) ".format(name, lastVal));
#                outputFile.write("{\n");
#                myIndent += 1
#            elif x[0] in ['average', 'stdev']:
#                outputFile.write(getIndent(myIndent))
#                outputFile.write("add{}{}(&{}, &{});\n".format(x[0][0].upper(),
#                                                x[0][1:], name, lastVal));
#            else:
#                # TODO: disallow this
#                print "Unsuported parameter for process: {}".format(x[0])
#                continue
#            lastVal = "get{}{}Value(&{})".format(x[0][0].upper(), x[0][1:], name)
#
#        outputFile.write(getIndent(myIndent));
#        outputFile.write("{} = {};\n".format(self.name, lastVal));
#
#        # Calls function who checks for dependencies against this process, 
#        # can't be done here because scope is too small
#        recursiveCall(self.name, myIndent, outputFile)
#
#        if haveFilter:
#            outputFile.write(getIndent(myIndent - 1));
#            outputFile.write("}\n");
#
#    def addComponents(self, componentRegister, conditionCollection):
#        if self.name in componentRegister.process:
#            userError("Process {0} already specified, ignoring\n".format(self.name))
#            return
#        componentRegister.process[self.name] = self

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
        for p in self.parameters.iteritems():
            result += ", "
            result += p[0]
            if p[1] != None:
                result += " "
                result += p[1].getCode()
        result += ';'
        return result

    def addComponents(self, componentRegister, conditionCollection):
        self.componentUseCase = \
            componentRegister.useComponent(self.type, self.name,
                                       self.parameters, self.fields,
                                       conditionCollection.conditionStack,
                                       conditionCollection.branchNumber)

    def collectImplicitDefines(self):
        implicitDefines = []
        if self.expression: 
            implicitDefines.append(ComponentDefineStatement(self.name, self.expression, []))
        if self.type == "output" and "where" in self.parameters:
            implicitDefines += self.parameters["where"].collectImplicitDefines(self)
        return implicitDefines


########################################################
CODE_BLOCK_TYPE_PROGRAM = 0
CODE_BLOCK_TYPE_WHEN = 1
CODE_BLOCK_TYPE_ELSEWHEN = 2
CODE_BLOCK_TYPE_ELSE = 3

class CodeBlock(object):
    def __init__(self, blockType, condition, declarations, next):
        self.blockType = blockType
        self.condition = condition
        self.declarations = declarations
        self.next = next

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
        if self.next != None:
            assert indent > 0
            indent -= 1
            result += getIndent(indent)
            result += self.next.getCode(indent)
            result += "\n"
        elif self.blockType != CODE_BLOCK_TYPE_PROGRAM:
            result += "end\n"

        return result

    def addComponents(self, componentRegister, conditionCollection):
        if self.blockType == CODE_BLOCK_TYPE_PROGRAM:
            # reset all
            conditionCollection.reset()
            # TODO: system parameters!
        elif self.blockType == CODE_BLOCK_TYPE_WHEN:
            stackStartSize = conditionCollection.size()
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

        # if there are some declarations present for this code branch,
        # make sure it gets unique number
        if conditionCollection.branchChanged and len(self.declarations):
            conditionCollection.branchNumber += 1
            conditionCollection.branchChanged = False

        # add implicitly declared virtual senosrs
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

#            if type(d) is ComponentDefineStatement:
#                if self.blockType != CODE_BLOCK_TYPE_PROGRAM:
#                    componentRegister.userError("Define supported only in top level, ignoring define '{0}'\n".format(
#                            d.name))
#                else:
#                    d.addComponents(componentRegister, conditionCollection)
            if type(d) is ParametersDefineStatement \
                    or type(d) is ComponentDefineStatement \
                    or type(d) is NetworkReadStatement:
                d.addComponents(componentRegister, conditionCollection)

            if type(d) is LoadStatement:
                if self.blockType != CODE_BLOCK_TYPE_PROGRAM:
                    componentRegister.userError("Load statements supported only in top level, ignoring '{0}'\n".format(
                            d.getConfigLine()))
                else:
                    d.load(componentRegister)

        # finish adding all virtual components
        for d in self.declarations:
            if type(d) is ComponentDefineStatement:
                d.continueAdding(componentRegister)
        for d in self.declarations:
            if type(d) is ComponentDefineStatement:
                d.finishAdding(componentRegister)

        # add set statements (may depend on virtual components)
        for d in self.declarations:
            if type(d) is SetStatement:
                d.addComponents(componentRegister, conditionCollection)

        # add use cases
        for d in self.declarations:
            if type(d) is ComponentUseCase:
                d.addComponents(componentRegister, conditionCollection)

        # recursive call (for included "when" statements)
        for d in self.declarations:
            if type(d) is CodeBlock:
                d.addComponents(componentRegister, conditionCollection)

        # recursive call (for next "elsewhen" or "else" branch)
        if self.next != None:
            self.next.addComponents(componentRegister, conditionCollection)

        if self.blockType == CODE_BLOCK_TYPE_WHEN:
            # pop all conditions from this code block
            conditionCollection.pop(conditionCollection.size() - stackStartSize)
