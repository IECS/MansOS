# Arithmetic functions -- "sum", "plus" (add), "minus" (subtract), "times" (multiply), "divide", "modulo", "difference", "abs", "neg", "invert", "square", "sqrt", "power"
# Data aggregation -- "min", "max", "average" (avg), "stdev" (std), "ewma", "changed"
# Signal processing -- "sharpen" (contrast), "smoothen" (blur), "map"
# Filtering -- "match", "filterRange", "filterEqual", "filterNotEqual", "filterLess", "filterLessOrEqual", "filterMore", "filterMoreOrEqual", "invertfilter"
# Subset selection & special purpose -- "take", "tuple", "sync"

functions = {}

class SealFunction(object):
    def __init__(self, name):
        global functions
        functions[name] = self
        # parameters
        self.aggregate = False # can have subset-selection or "tuple" function as argument?
        self.special = False   # special-purpose?
        self.synonym = None    # if set, can be referred by this name in SEAL code
        self.group = ""        # function class (documentation only)
        self.arguments = []

class SealArgument(object):
    def __init__(self, name, constantOnly = False, defaultValue = None, repeated = False):
        self.name = name
        # value must be known at compile time? i.e. not a sensor
        self.constantOnly = constantOnly
        # optional arguments can be skipped; they must have default values
        self.defaultValue = defaultValue
        # some functions (e.g. "sum(1, 2, 3)") can have arguments repatead zero to many times
        self.repeated = repeated

# --------------------------------------------------

f = SealFunction("sum")
f.group = "arithmetic" # or aggregate
f.aggregate = True
f.arguments.append(SealArgument("value", repeated = True))

f = SealFunction("plus")
f.group = "arithmetic"
f.synonym = "add"
f.arguments.append(SealArgument("arg1"))
f.arguments.append(SealArgument("arg2"))

f = SealFunction("minus")
f.group = "arithmetic"
f.synonym = "subtract"
f.arguments.append(SealArgument("arg1"))
f.arguments.append(SealArgument("arg2"))

f = SealFunction("times")
f.group = "arithmetic"
f.synonym = "multiply"
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
f.arguments.append(SealArgument("arg1"))

f = SealFunction("neg")
f.group = "arithmetic"
f.arguments.append(SealArgument("arg1"))

f = SealFunction("invert")
f.group = "arithmetic"
f.arguments.append(SealArgument("arg1"))

f = SealFunction("square")
f.group = "arithmetic"
f.arguments.append(SealArgument("arg1"))

f = SealFunction("sqrt")
f.group = "arithmetic"
f.arguments.append(SealArgument("arg1"))

f = SealFunction("power")
f.group = "arithmetic"
f.arguments.append(SealArgument("base"))
f.arguments.append(SealArgument("exponent"))

# --------------------------------------------------

f = SealFunction("min")
f.group = "aggregation"
f.aggregate = True
f.arguments.append(SealArgument("value", repeated = True))

f = SealFunction("max")
f.group = "aggregation"
f.aggregate = True
f.arguments.append(SealArgument("value", repeated = True))

f = SealFunction("average")
f.group = "aggregation"
f.aggregate = True
f.synonym = "avg"
f.arguments.append(SealArgument("value"))

f = SealFunction("stdev")
f.group = "aggregation"
f.aggregate = True
f.synonym = "std"
f.arguments.append(SealArgument("value"))

f = SealFunction("ewma") # exponentially weighted moving average
f.group = "aggregation"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("alpha", constantOnly = True, defaultValue = "0.1"))

f = SealFunction("changed")
f.group = "aggregation"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("milliseconds", constantOnly = True, defaultValue = "10s"))

# --------------------------------------------------

f = SealFunction("sharpen")
f.group = "signal processing"
f.synonym = "contrast"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("numSamples", constantOnly = True, defaultValue = "3"))
f.arguments.append(SealArgument("weight", constantOnly = True, defaultValue = "1"))

f = SealFunction("smoothen")
f.group = "signal processing"
f.synonym = "blur"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("numSamples", constantOnly = True, defaultValue = "3"))
f.arguments.append(SealArgument("weight", constantOnly = True, defaultValue = "1"))

f = SealFunction("map")
f.group = "signal processing"
f.arguments.append(SealArgument("value"))
f.arguments.append(SealArgument("fromRangeLow", constantOnly = True))
f.arguments.append(SealArgument("fromRangeHigh", constantOnly = True))
f.arguments.append(SealArgument("toRangeLow", constantOnly = True))
f.arguments.append(SealArgument("toRangeHigh", constantOnly = True))

# --------------------------------------------------

f = SealFunction("match")
f.group = "filtering"
f.arguments.append(SealArgument("value"))
# only a string (pattern name) is allowed - set "constantOnly" to True
f.arguments.append(SealArgument("pattern", constantOnly = True))

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
f.arguments.append(SealArgument("timeToTake", constantOnly = True, defaultValue = "0"))

# Create a tuple with a number of different sensor values
f = SealFunction("tuple")
f.group = "special"
f.special = True
f.arguments.append(SealArgument("value", repeated = True))

# Synchronize sensor reading (must be at the top level)
f = SealFunction("sync")
f.group = "special"
f.special = True
f.arguments.append(SealArgument("value", repeated = True))
