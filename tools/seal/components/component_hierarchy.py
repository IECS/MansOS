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
        self.pattern = SealParameter(None)
        self.once = SealParameter(False, [False, True])
        self.useFunction = SealParameter(None)   # each usable component must define this
        self.readFunction = SealParameter(None)  # each readable component must define this
        components.append(self)

    def calculateParameterValue(self, parameter, useCaseParameters):
        # default implementation
        return self.__getattribute__(parameter).value

    def getParameterValue(self, parameter, useCaseParameters):
        param = useCaseParameters.get(parameter, None)
        if param is not None:
            # XXX ugly hack!
            # print "got param = ", param
            try:
                # print "   value = ", param.value
                return param.value
            except AttributeError:
                return param
        return self.__getattribute__(parameter).value

######################################################
class SealSensor(SealComponent):
    def __init__(self, name):
        super(SealSensor, self).__init__(TYPE_SENSOR, name)
        self.cache = SealParameter(False, [False, True])
        self.cacheable = True # whether can be kept in cache
        self.dataSize = 2 # in bytes
#        self.filter = SealParameter('', ['> 100'])
#        self.average = SealParameter('', ['10'])
#        self.stdev = SealParameter('', ['10'])
        self.prereadFunction = SealParameter(None)
        self.minUpdatePeriod = 1000 # milliseconds
        self.readTime = 0 # read instanttly
        self.readFunctionDependsOnParams = False

# for remote use only
class CommandSensor(SealSensor):
    def __init__(self):
        super(CommandSensor, self).__init__("Command")
        self.useFunction.value = "0"
        self.readFunction.value = self.useFunction.value

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

class CounterSensor(SealSensor):
    def __init__(self):
        super(CounterSensor, self).__init__("Counter")
        self.useFunction.value = "0"
        self.readFunction.value = self.useFunction.value
        # the values used in read function
        self.start = SealParameter(0, ["0", "1", "2", "5", "10", "100", "1000"])
        self.max = SealParameter(0xffffffff, ["1", "2", "5", "10", "100", "1000", "0xffffffff"])
        self.readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        start = int(self.getParameterValue("start", useCaseParameters))
        max = int(self.getParameterValue("max", useCaseParameters))
        if max >= 0x80000000: suffix = "u"
        else: suffix = ""

        # use GCC statement-as-expression extension
        return "({" + "static uint32_t counter = {}; counter = (counter + 1) % {}{};".format(
            start, max, suffix) + "})"

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
class TimeCounterSensor(SealSensor):
    def __init__(self):
        super(TimeCounterSensor, self).__init__("TimeCounter")
        self.useFunction.value = "0"
        self.readFunction.value = "0"
        self.counterperiod = SealParameter(1000, ["100", "200", "500", "1000", "2000"])
        self.readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        counterPeriod = self.getParameterValue("counterperiod", useCaseParameters)
        return "getJiffies() / {0}".format(counterPeriod)

#
# System time sensor.
# At the moment, timestamp is the same as system uptime seconds.
#
class SystemTimeSensor(SealSensor):
    def __init__(self):
        super(SystemTimeSensor, self).__init__("SystemTime")
        self.useFunction.value = "getUptime()"
        self.readFunction.value = "getUptime()"

# TODO: allow aliases!
class UptimeSensor(SealSensor):
    def __init__(self):
        super(UptimeSensor, self).__init__("Uptime")
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

class AnalogInputSensor(SealSensor):
    def __init__(self):
        super(AnalogInputSensor, self).__init__("AnalogIn")
        self.useFunction.value = "adcRead(1)"
        self.readFunction.value = "adcRead(1)"
        self.channel = SealParameter(0, ["0", "1", "2", "3", "4", "5", "6", "7"])
        self.readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        # print "calculateParameterValue: ", parameter
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        channel = int(self.getParameterValue("channel", useCaseParameters))
        if channel is None: channel = 1
        return "adcRead({})".format(channel)

class DigitalInputSensor(SealSensor):
    def __init__(self):
        super(DigitalInputSensor, self).__init__("DigitalIn")
        self.useFunction.value = "pinRead(1, 0)"
        self.readFunction.value = "pinRead(1, 0)"
        self.readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        port = int(self.getParameterValue("port", useCaseParameters))
        pin = int(self.getParameterValue("pin", useCaseParameters))
        if port is None: port = 1
        if pin is None: pin = 0
        return "pinRead({}, {})".format(port, pin)

#######################################################
class SealActuator(SealComponent):
    def __init__(self, name):
        super(SealActuator, self).__init__(TYPE_ACTUATOR, name)
        self.onFunction = SealParameter(None)
        self.offFunction = SealParameter(None)
        self.on = SealParameter(False, [False, True])
        self.off = SealParameter(False, [False, True])

class LedAct(SealActuator):
    def __init__(self):
        super(LedAct, self).__init__("Led")
        self.useFunction.value = "ledToggle()"
        self.readFunction.value = "ledGet()"
        self.onFunction.value = "ledOn()"
        self.offFunction.value = "ledOff()"

class RedLedAct(SealActuator):
    def __init__(self):
        super(RedLedAct, self).__init__("RedLed")
        self.useFunction.value = "redLedToggle()"
        self.readFunction.value = "redLedGet()"
        self.onFunction.value = "redLedOn()"
        self.offFunction.value = "redLedOff()"

class BlueLedAct(SealActuator):
    def __init__(self):
        super(BlueLedAct, self).__init__("BlueLed")
        self.useFunction.value = "blueLedToggle()"
        self.readFunction.value = "blueLedGet()"
        self.onFunction.value = "blueLedOn()"
        self.offFunction.value = "blueLedOff()"

class GreenLedAct(SealActuator):
    def __init__(self):
        super(GreenLedAct, self).__init__("GreenLed")
        self.useFunction.value = "greenLedToggle()"
        self.readFunction.value = "greenLedGet()"
        self.onFunction.value = "greenLedOn()"
        self.offFunction.value = "greenLedOff()"

class DigitalOutputAct(SealActuator):
    def __init__(self):
        super(DigitalOutputAct, self).__init__("DigitalOut")
        self.useFunction.value = "pinToggle(1, 0)"
        self.readFunction.value = "pinRead(1, 0)"
        self.onFunction.value = "pinSet(1, 0)"
        self.offFunction.value = "pinClear(1, 0)"

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" \
                and parameter != "useFunction" \
                and parameter != "onFunction" \
                and parameter != "offFunction":
            return SealActuator.calculateParameterValue(self, parameter, useCaseParameters)
        port = int(self.getParameterValue("port", useCaseParameters))
        pin = int(self.getParameterValue("pin", useCaseParameters))
        if port is None: port = 1
        if pin is None: pin = 0
        args = "(" + str(port) + ", " + str(pin) + ")"
        if parameter == "readFunction":
            return "pinRead" + args
        if parameter == "useFunction":
            return "pinToggle" + args
        if parameter == "onFunction":
            return "pinSet" + args
        if parameter == "offFunction":
            return "pinClear" + args
        return None

class BeeperAct(SealActuator):
    def __init__(self):
        super(BeeperAct, self).__init__("Beeper")
        self.useFunction.value = "beeperBeep(100)"
        self.readFunction.value = "false"
        self.onFunction.value = "beeperBeep(100)"
        self.offFunction.value = "/* nothing */"

        self.frequency = SealParameter(None, ["100", "200", "500", "1000", "2000", "5000", "10000"])
        self.duration = SealParameter(None, ["10", "20", "50", "100", "200", "500", "1000", "2000"])

        self.extraConfig = SealParameter("USE_BEEPER=y")
        self.extraIncludes = SealParameter("#include <hil/beeper.h>")

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "useFunction" and parameter != "onFunction":
            return SealActuator.calculateParameterValue(self, parameter, useCaseParameters)
        frequency = self.getParameterValue("frequency", useCaseParameters)
        duration = self.getParameterValue("duration", useCaseParameters)
        if duration is None: duration = "100"
        if parameter == "useFunction" or parameter == "onFunction":
            if frequency:
                return "beeperBeepEx({}, {})".format(duration, frequency)
            return "beeperBeep({})".format(duration)


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
        self.sequencenumber = SealParameter(False, [False, True])

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

class SdCardOutput(SealOutput):
    def __init__(self):
        super(SdCardOutput, self).__init__("SdCard")
        self.useFunction.value = "sdcardWrite(&sdCardPacket, sizeof(sdCardPacket))"

# "local storage" (i.e. [external] flash or SD card is defined depending on platform
# on telosb, local storage is synonym for external flash
class LocalStorageOutput(SealOutput):
    def __init__(self):
        super(LocalStorageOutput, self).__init__("LocalStorage")
        # ext flash by default
        self.useFunction.value = "extFlashWrite(&externalFlashPacket, sizeof(externalFlashPacket))"

class FileOutput(SealOutput):
    def __init__(self):
        super(FileOutput, self).__init__("File")
        self.useFunction.value = "filePrint()"
        self.filename = SealParameter(None, [])
        # a file can be text or binary - allow both parameter names wih inverse meaning
        self.text = SealParameter(False, [False, True])
        self.binary = SealParameter(True, [False, True])
