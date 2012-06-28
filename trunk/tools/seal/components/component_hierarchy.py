TYPE_ACTUATOR = 1
TYPE_SENSOR = 2
TYPE_OUTPUT = 3

components = []

#######################################################
class SealParameter(object):
    def __init__(self, value, valueList = []):
        self.value = value
        self.valueList = valueList

#######################################################
class SealComponent(object):
    def __init__(self, typeCode, name):
        self.typeCode = typeCode
        self.name = name
        # parameters
        self.id = SealParameter(0, ["0", "1"])
        self.extraConfig = SealParameter(None)  # config file line(s)
        self.extraIncludes = SealParameter(None) # #include<> line(s)
        self.period = SealParameter(1000, ['100', '200', '500', '1000', '2000'])
        self.pattern = None
        self.once = SealParameter(False, [False, True])
        self.useFunction = SealParameter(None)  # each usable component must define this
        self.readFunction = SealParameter(None)  # each readable component must define this
        components.append(self)

    def calculateParameterValue(self, parameter, useCaseParameters):
        # default implementation
        return self.__getattribute__(parameter).value

    def getParameterValue(self, parameter, useCaseParameters):
        param = useCaseParameters.get(parameter, None)
        if param is not None:
            # XXX ugly hack!
            try:
                return param.value
            except AttributeError:
                return param
        return self.__getattribute__(parameter).value

######################################################
class SealSensor(SealComponent):
    def __init__(self, name):
        super(SealSensor, self).__init__(TYPE_SENSOR, name)
        self.cacheable = False # whether can be kept in cache
        self.dataSize = 2 # in bytes
#        self.filter = SealParameter('', ['> 100'])
#        self.average = SealParameter('', ['10'])
#        self.stdev = SealParameter('', ['10'])
        self.prereadFunction = SealParameter(None)
        self.minUpdatePeriod = 1000 # milliseconds
        self.readTime = 0 # read instanttly
        self.readFunctionDependsOnParams = False

class ConstantSensor(SealSensor):
    def __init__(self):
        super(ConstantSensor, self).__init__("Constant")
        self.useFunction.value = "0"
        self.readFunction.value = self.useFunction.value
        # the value used in read function
        self.value = SealParameter(5, ["0", "1", "2", "5", "10", "100", "1000"])
        self.readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        return self.getParameterValue("value", useCaseParameters)

class RandomSensor(SealSensor):
    def __init__(self):
        super(RandomSensor, self).__init__("Random")
        self.useFunction.value = "randomNumber()"
        self.readFunction.value = "randomNumber()"
        self.extraConfig = SealParameter("USE_RANDOM=y")
        self.extraIncludes = SealParameter("#include <random.h>")
        # parameters
        self.min = SealParameter(0, ["0", "1", "2", "5", "10", "100", "1000"])
        self.max = SealParameter(0xffff, ["1", "2", "5", "10", "100", "1000", "65535"])
        self.readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        #if parameter != "readFunction" and parameter != "useFunction":
        #    return SealSensor.calculateParameterValue(parameter, useCaseParameters)
        min = int(self.getParameterValue("min", useCaseParameters))
        max = int(self.getParameterValue("max", useCaseParameters))
        modulo = max - min + 1
        if modulo == 0x10000:
            return self.useFunction.value
        else:
            # TODO: cast the result to uint16_t, but only in case of signed int overflow
            return "{} % {} + {}".format(self.useFunction.value, modulo, min)

#
# A time-based counter value.
# Be careful about wraparound: use 64-bit jiffies if possible!
#
class CounterSensor(SealSensor):
    def __init__(self):
        super(CounterSensor, self).__init__("Counter")
        self.useFunction.value = "0"
        self.readFunction.value = "0"
        self.counterperiod = SealParameter(1000, ["100", "200", "500", "1000", "2000"])
        self.readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        counterPeriod = self.getParameterValue("counterperiod", useCaseParameters)
        return "getJiffies() / {0}".format(counterPeriod)

#
# Timestamp sensor.
# At the moment, timestamp is the same as system uptime seconds.
#
class TimestampSensor(SealSensor):
    def __init__(self):
        super(TimestampSensor, self).__init__("Timestamp")
        self.useFunction.value = "getUptime()"
        self.readFunction.value = "getUptime()"

#
# Generic sensor with a predefined waveform.
#
class WaveSensor(SealSensor):
    def __init__(self, name):
        super(WaveSensor, self).__init__(name)
        self.useFunction.value = "0"
        self.readFunction.value = "0"
        # parameters (all names must be lowercase because of search algorithm limitations)
        self.low = SealParameter(0, ["0", "1", "2", "5", "10", "100", "1000"])
        self.high = SealParameter(100, ["0", "1", "2", "5", "10", "100", "1000"])
        self.waveperiod = SealParameter(1000, ["100", "200", "500", "1000", "2000"])
        self.readFunctionDependsOnParams = True

#
# Predefined waveform sensor: 
#    +----+    +----+    +----+
#  --+    +----+    +----+    +--
#
class SquareWaveSensor(WaveSensor):
    def __init__(self):
        super(SquareWaveSensor, self).__init__("SquareWave")

    def calculateParameterValue(self, parameter, useCaseParameters):
        wavePeriod = self.getParameterValue("waveperiod", useCaseParameters)
        low = self.getParameterValue("low", useCaseParameters)
        high = self.getParameterValue("high", useCaseParameters)
        return "(getJiffies() % {0} < ({0} / 2)) ? {1} : {2}".format(wavePeriod, low, high)

#
# Predefined waveform sensor:
#   ^   ^   ^
#  / \ / \ / \
# /   v   v   v
#
class TriangleWaveSensor(WaveSensor):
    def __init__(self):
        super(TriangleWaveSensor, self).__init__("TriangleWave")
        self.extraIncludes = SealParameter("#include <lib/algo.h>")

    def calculateParameterValue(self, parameter, useCaseParameters):
        wavePeriod = self.getParameterValue("waveperiod", useCaseParameters)
        low = self.getParameterValue("low", useCaseParameters)
        high = self.getParameterValue("high", useCaseParameters)
        return "triangleWaveValue({}, {}, {})".format(wavePeriod, low, high)

#
# Predefined waveform sensor:
#   /  /  /
#  /| /| /|
# / |/ |/ |
#
class SawtoothWaveSensor(WaveSensor):
    def __init__(self):
        super(SawtoothWaveSensor, self).__init__("SawtoothWave")
        self.extraIncludes = SealParameter("#include <lib/algo.h>")

    def calculateParameterValue(self, parameter, useCaseParameters):
        wavePeriod = self.getParameterValue("waveperiod", useCaseParameters)
        low = self.getParameterValue("low", useCaseParameters)
        high = self.getParameterValue("high", useCaseParameters)
        return "sawtoothWaveValue({}, {}, {})".format(wavePeriod, low, high)

#
# Predefined waveform sensor: sine wave
#
class SineWaveSensor(WaveSensor):
    def __init__(self):
        super(SineWaveSensor, self).__init__("SineWave")

    def calculateParameterValue(self, parameter, useCaseParameters):
        wavePeriod = self.getParameterValue("waveperiod", useCaseParameters)
        low = self.getParameterValue("low", useCaseParameters)
        high = self.getParameterValue("high", useCaseParameters)
        return "sineWaveValue({}, {}, {})".format(wavePeriod, low, high)

class LightSensor(SealSensor):
    def __init__(self):
        super(LightSensor, self).__init__("Light")
        self.useFunction.value = "lightRead()"
        self.readFunction.value = "lightRead()"

class HumiditySensor(SealSensor):
    def __init__(self):
        super(HumiditySensor, self).__init__("Humidity")
        self.useFunction.value = "humidityRead()"
        self.readFunction.value = "humidityRead()"
        self.errorFunction = SealParameter("humidityIsError()")
        self.extraConfig = SealParameter("USE_HUMIDITY=y")
        self.extraIncludes = SealParameter("#include <hil/humidity.h>")

#######################################################
class SealActuator(SealComponent):
    def __init__(self, name):
        super(SealActuator, self).__init__(TYPE_ACTUATOR, name)

class LedAct(SealActuator):
    def __init__(self):
        super(LedAct, self).__init__("Led")
        self.useFunction.value = "ledToggle()"
        self.readFunction.value = "ledGet()"

class RedLedAct(SealActuator):
    def __init__(self):
        super(RedLedAct, self).__init__("RedLed")
        self.useFunction.value = "redLedToggle()"
        self.readFunction.value = "redLedGet()"

class BlueLedAct(SealActuator):
    def __init__(self):
        super(BlueLedAct, self).__init__("BlueLed")
        self.useFunction.value = "blueLedToggle()"
        self.readFunction.value = "blueLedGet()"

class GreenLedAct(SealActuator):
    def __init__(self):
        super(GreenLedAct, self).__init__("GreenLed")
        self.useFunction.value = "greenLedToggle()"
        self.readFunction.value = "greenLedGet()"

class PrintAct(SealActuator):
    def __init__(self):
        super(PrintAct, self).__init__("Print")
        self.format = SealParameter(None)
        self.arg0 = SealParameter(None)
        self.arg1 = SealParameter(None)
        self.arg2 = SealParameter(None)
        self.arg3 = SealParameter(None)
        self.arg4 = SealParameter(None)
        self.arg5 = SealParameter(None)
        self.arg6 = SealParameter(None)
        self.arg7 = SealParameter(None)
        self.arg8 = SealParameter(None)
        self.arg9 = SealParameter(None)

#######################################################
class SealOutput(SealComponent):
    def __init__(self, name):
        super(SealOutput, self).__init__(TYPE_OUTPUT, name)
        self.aggregate = SealParameter(True, [False, True])
        self.crc = SealParameter(False, [False, True])
        self.address = SealParameter(False, [False, True])
        self.timestamp = SealParameter(True, [False, True])

class SerialOutput(SealOutput):
    def __init__(self):
        super(SerialOutput, self).__init__("Serial")
        self.useFunction.value = "serialPacketPrint()"
        self.baudrate = SealParameter(38400, ['9600', '38400', '57600', '115200'])
        self.aggregate.value = False # false by default

class RadioOutput(SealOutput):
    def __init__(self):
        super(RadioOutput, self).__init__("Radio")
        self.useFunction.value = "radioSend(&radioPacket, sizeof(radioPacket))"
        self.crc.value = True # true by default
        self.address.value = True # true by default

class InternalFlashOutput(SealOutput):
    def __init__(self):
        super(InternalFlashOutput, self).__init__("InternalFlash")
        self.useFunction.value = "flashWrite(&internalFlashPacket, sizeof(internalFlashPacket))"

class ExternalFlashOutput(SealOutput):
    def __init__(self):
        super(ExternalFlashOutput, self).__init__("ExternalFlash")
        self.useFunction.value = "extFlashWrite(&externalFlashPacket, sizeof(externalFlashPacket))"

# TODO: SD card

# TODO: "local storage" (i.e. [external] flash or SD card, depending on platform
