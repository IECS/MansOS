from structures import *

# Arithmetic functions -- "sum", "plus" (add), "minus" (subtract), "times" (multiply), "divide", "modulo", "difference", "abs", "neg", "invert", "square", "sqrt", "power"
# Data aggregation -- "min", "max", "average" (avg), "stdev" (std), "ewma", "changed"
# Signal processing -- "sharpen" (contrast), "smoothen" (blur), "map"
# Filtering -- "match", "filterRange", "filterEqual", "filterNotEqual", "filterLess", "filterLessOrEqual", "filterMore", "filterMoreOrEqual", "invertfilter"
# Subset selection & special purpose -- "take", "tuple", "sync"

# this global dictionary holds all functions by name
functions = {}

class SealFunction(object):
    def __init__(self, name):
        global functions
        functions[name.lower()] = self
        self.name = name
        # parameters
        self.aggregate = False # can have subset-selection or "tuple" function as argument?
        self.special = False   # special-purpose?
        self.alias = None      # if set, can be referred by this name in SEAL code
        self.group = ""        # function class (documentation only)
        self.repeatedArguments = False # has this function variable number of arguments?
        self.arguments = []

    def getArgumentByName(self, name):
        for arg in self.arguments:
            if arg.name.lower() == name:
                return arg
        return None

    def getArgumentByPosition(self, pos):
        if pos < len(self.arguments):
            return self.arguments[pos]
        return None

    def declaration(self):
        result = self.name
        result += "("
        result += ", ".join(map(str, self.arguments))
        result += ")"
        return result

class SealArgument(object):
    def __init__(self, name, constantOnly = False, defaultValue = None, repeated = False):
        self.name = name
        # value must be known at compile time? i.e. not a sensor
        self.constantOnly = constantOnly
        # optional arguments can be skipped; they must have default values
        self.defaultValue = defaultValue
        # some functions (e.g. "sum(1, 2, 3)") can have arguments repatead zero to many times
        self.repeated = repeated

    def __str__(self):
        return self.name


# --------------------------------------------------

f = SealFunction("sum")
f.group = "arithmetic" # or aggregate
f.aggregate = True
f.repeatedArguments = True
f.arguments.append(SealArgument("value", repeated = True))

f = SealFunction("plus")
f.group = "arithmetic"
f.alias = "add"
f.arguments.append(SealArgument("arg1"))
f.arguments.append(SealArgument("arg2"))

f = SealFunction("minus")
f.group = "arithmetic"
f.alias = "subtract"
f.arguments.append(SealArgument("arg1"))
f.arguments.append(SealArgument("arg2"))

f = SealFunction("times")
f.group = "arithmetic"
f.alias = "multiply"
f.arguments.append(SealArgument("arg1"))
f.arguments.append(SealArgument("arg2"))

f = SealFunction("divide")
f.group = "arithmetic"
f.arguments.append(SealArgument("arg1"))
f.arguments.append(SealArgument("arg2"))

f = SealFunction("modulo")
f.group = "arithmetic"
f.arguments.append(SealArgument("arg1"))
f.arguments.append(SealArgument("arg2"))

f = SealFunction("difference")
f.group = "arithmetic"
f.arguments.append(SealArgument("arg1"))
f.arguments.append(SealArgument("arg2"))

f = SealFunction("abs")
f.group = "arithmetic"
f.arguments.append(SealArgument("value"))

f = SealFunction("neg")
f.group = "arithmetic"
f.arguments.append(SealArgument("value"))

f = SealFunction("invert")
f.group = "arithmetic"
f.arguments.append(SealArgument("value"))

f = SealFunction("square")
f.group = "arithmetic"
f.arguments.append(SealArgument("value"))

f = SealFunction("sqrt")
f.group = "arithmetic"
f.arguments.append(SealArgument("value"))

f = SealFunction("power")
f.group = "arithmetic"
f.arguments.append(SealArgument("base"))
f.arguments.append(SealArgument("exponent"))

# --------------------------------------------------

f = SealFunction("min")
f.group = "aggregation"
f.aggregate = True
f.repeatedArguments = True
f.arguments.append(SealArgument("value", repeated = True))

f = SealFunction("max")
f.group = "aggregation"
f.aggregate = True
f.repeatedArguments = True
f.arguments.append(SealArgument("value", repeated = True))

f = SealFunction("average")
f.group = "aggregation"
f.aggregate = True
f.alias = "avg"
f.arguments.append(SealArgument("value"))

f = SealFunction("stdev")
f.group = "aggregation"
f.aggregate = True
f.alias = "std"
f.arguments.append(SealArgument("value"))

f = SealFunction("variance")
f.group = "aggregation"
f.aggregate = True
f.arguments.append(SealArgument("value"))

f = SealFunction("ewma") # exponentially weighted moving average
f.group = "aggregation"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("alpha", constantOnly = True, defaultValue = Value(0.1)))

f = SealFunction("changed")
f.group = "aggregation"
f.aggregate = True
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("milliseconds", constantOnly = True, defaultValue = Value(10000)))

# --------------------------------------------------

f = SealFunction("map")
f.group = "signal processing"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("fromRangeLow", constantOnly = True))
f.arguments.append(SealArgument("fromRangeHigh", constantOnly = True))
f.arguments.append(SealArgument("toRangeLow", constantOnly = True))
f.arguments.append(SealArgument("toRangeHigh", constantOnly = True))

f = SealFunction("sharpen")
f.group = "signal processing"
f.alias = "contrast"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("numSamples", constantOnly = True, defaultValue = Value(3)))
f.arguments.append(SealArgument("weight", constantOnly = True, defaultValue = Value(1)))

f = SealFunction("smoothen")
f.group = "signal processing"
f.alias = "blur"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("numSamples", constantOnly = True, defaultValue = Value(3)))
f.arguments.append(SealArgument("weight", constantOnly = True, defaultValue = Value(1)))

# --------------------------------------------------

f = SealFunction("match")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
# XXX: only a string (pattern name) is allowed. this is not validated ATM
f.arguments.append(SealArgument("pattern"))

f = SealFunction("filterRange")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("thresholdMin", constantOnly = True))
f.arguments.append(SealArgument("thresholdMax", constantOnly = True))

f = SealFunction("filterEqual")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("threshold", constantOnly = True))

f = SealFunction("filterNotEqual")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("threshold", constantOnly = True))

f = SealFunction("filterLess")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("threshold", constantOnly = True))

f = SealFunction("filterLessOrEqual")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("threshold", constantOnly = True))

f = SealFunction("filterMore")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("threshold", constantOnly = True))

f = SealFunction("filterMoreOrEqual")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("threshold", constantOnly = True))

f = SealFunction("invertFilter")
f.group = "filtering"
f.arguments.append(SealArgument("filteredValue"))

# --------------------------------------------------

# Take a number of single sensor values
f = SealFunction("take")
f.group = "special"
f.special = True
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("numberToTake", constantOnly = True))
# time in milliseconds; if not set, time is not taken in account
f.arguments.append(SealArgument("timeToTake", constantOnly = True, defaultValue = Value(0)))

# Create a tuple with a number of different sensor values
f = SealFunction("tuple")
f.group = "special"
f.special = True
f.repeatedArguments = True
f.arguments.append(SealArgument("value", repeated = True))

# Synchronize sensor reading (must be at the top level)
f = SealFunction("sync")
f.group = "special"
f.special = True
f.repeatedArguments = True
f.arguments.append(SealArgument("value", repeated = True))

# --------------------------------------------------

def resolveAlias(funName):
    funName = funName.lower()
    if funName == "add": return "plus"
    if funName == "subtract": return "minus"
    if funName == "multiply": return "times"
    if funName == "avg": return "average"
    if funName == "std": return "stdev"
    if funName == "contrast": return "sharpen"
    if funName == "blur": return "smoothen"
    return funName

def validateFunction(functionTree):
    funName = functionTree.function

    fun = functions.get(resolveAlias(funName))
    if fun is None:
        return (False, "Unhandled function {}()\n".format(funName))

    givenArgs = {}
    for i in range(len(functionTree.arguments)):
        givenArg = functionTree.arguments[i]
        if givenArg.parameterName:
            formalArg = fun.getArgumentByName(givenArg.parameterName)
            if formalArg == None:
                return (False, "Unknown named argument {} for function {}\n".format(
                        givenArg.parameterName, fun.declaration()))
        else:
            # TODO? do not allow to mix the args (i.e. named args must always be at the end)
            formalArg = fun.getArgumentByPosition(i)
            if formalArg == None:
                if fun.repeatedArguments:
                    continue
                return (False, "Too many arguments for function {}\n".format(
                        fun.declaration()))

        if formalArg.name not in givenArgs:
            givenArgs[formalArg.name] = [givenArg]
        else:
            givenArgs[formalArg.name].append(givenArg)

    i = 0
    for f in fun.arguments:
        i += 1
        g = givenArgs.get(f.name)
        if g is None:
            if not f.defaultValue:
                return (False, "{} argument ('{}') of function {} is not optional\n".format(
                        toTitleCase(orderNumToString(i)), f.name, fun.declaration()))
            # append the default value to real arguments
            functionTree.arguments.append(FunctionTree(f.defaultValue, []))
            continue

        if len(g) > 1:
            if not f.repeated:
                return (False, "{} argument ('{}') of function {} repeated more than once\n".format(
                        toTitleCase(orderNumToString(i)), f.name, fun.declaration()))

        # OK, argument expected and found. check its value.
        if f.constantOnly:
            for v in g:
                if v.asConstant() is None:
                    return (False, "{} argument ('{}') of function {} is expected to be a constant!\n".format(
                        toTitleCase(orderNumToString(i)), f.name, fun.declaration()))

    return (True, None)
