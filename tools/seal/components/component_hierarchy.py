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

TYPE_ACTUATOR = 1
TYPE_SENSOR = 2
TYPE_OUTPUT = 3

components = []

#######################################################
class SealParameter(object):
    def __init__(self, value, valueList = []):
        self.value = value
        self.valueList = valueList
        self.isAdvanced = False

class SealAdvancedParameter(SealParameter):
    def __init__(self, value, valueList = []):
        super(SealAdvancedParameter, self).__init__(value, valueList)
        self.isAdvanced = True

#######################################################
class SealComponent(object):
    def __init__(self, typeCode, name):
        self._typeCode = typeCode
        self._name = name
        # parameters
        self.aliases = dict() # alias name -> parameter name
        self.id = SealParameter(0, ["0", "1"])
        self.extraConfig = SealAdvancedParameter(None)   # config file line(s)
        self.extraIncludes = SealAdvancedParameter(None) # #include<> line(s)
        # read/use period
        self.period = SealParameter(1000, ['100', '200', '500', '1000', '2000'])
        # read/use using specific time pattern?
        self.pattern = SealParameter(None)
        # read/use just once?
        self.once = SealParameter(None, [False, True])
        # read/use just a few times? ("times 1" is the same as "once")
        self.times = SealParameter(None, ['1', '2', '3', '4', '5', '10', '20', '50', '100'])
        # read/use just for a time period?
        self.duration = SealParameter(None, ['100', '200', '500', '1000', '2000'])
        self.useFunction = SealAdvancedParameter(None)   # each usable component must define this
        self.readFunction = SealAdvancedParameter(None)  # each readable component must define this
        self.aliases["time"] = "duration"
        components.append(self)

    def resolveAlias(self, possibleAliasName):
        if possibleAliasName in self.aliases:
            return self.aliases[possibleAliasName]
        return possibleAliasName

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
        self.cache = SealParameter(None, [False, True])
        self._cacheable = True # whether can be kept in cache
        self._dataSize = 4 # in bytes
        self._dataType = "int32_t"
        self.preReadFunction = SealAdvancedParameter(None)
        self._minUpdatePeriod = 1000 # milliseconds
        self._readTime = 0 # read instanttly
        self._readFunctionDependsOnParams = False
        # call on and off functions before/after reading?
        self.turnonoff = SealParameter(None, [False, True])
        self.onFunction = SealAdvancedParameter(None)
        self.offFunction = SealAdvancedParameter(None)
        # output reading to somewhere? (actuator or output; same syntax, different semantics)
        self.out = SealParameter(None)
        # evaluate function(s) lazily? (for example, useful for averaged sensors)
        self.lazy = SealParameter(None, [False, True])

# for remote use only
class CommandSensor(SealSensor):
    def __init__(self):
        super(CommandSensor, self).__init__("Command")
        self.useFunction.value = "0"
        self.readFunction.value = self.useFunction.value

# to read C variables (pseudo sensor)
class VariableSensor(SealSensor):
    def __init__(self):
        super(VariableSensor, self).__init__("Variables")
        self.useFunction.value = "0"
        self.readFunction.value = self.useFunction.value
        self._cacheable = False
        # the name of C variable
        self.name = SealParameter(None, [])
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        return self.getParameterValue("name", useCaseParameters)

# to read C constants (pseudo sensor)
class ConstantsSensor(SealSensor):
    def __init__(self):
        super(ConstantsSensor, self).__init__("Constants")
        self.useFunction.value = "0"
        self.readFunction.value = self.useFunction.value
        self._cacheable = False
        # the name of C constant
        self.name = SealParameter(None, [])
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        return self.getParameterValue("name", useCaseParameters)


class ConstantSensor(SealSensor):
    def __init__(self):
        super(ConstantSensor, self).__init__("Constant")
        self.useFunction.value = "0"
        self.readFunction.value = self.useFunction.value
        # the value used in read function
        self.value = SealParameter(5, ["0", "1", "2", "5", "10", "100", "1000"])
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        return self.getParameterValue("value", useCaseParameters)

class CounterSensor(SealSensor):
    def __init__(self):
        super(CounterSensor, self).__init__("Counter")
        self.useFunction.value = "0"
        self.readFunction.value = self.useFunction.value
        # the values used in read function
        self.start = SealParameter(0, ["0", "1", "2", "5", "10", "100", "1000"])
        self.max = SealParameter(0xffffffff, ["1", "2", "5", "10", "100", "1000", "0xffffffff"])
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        start = self.getParameterValue("start", useCaseParameters)
        max = self.getParameterValue("max", useCaseParameters)
        if isinstance(max, int) or isinstance(max, long):
            suffix = "ul" if max >= 0x80000000 else "l"
        else:
            suffix = ""
        # use GCC statement-as-expression extension
        return "({" + "static uint32_t counter = {}; counter = (counter + 1) % {}{};".format(
            start, max, suffix) + "})"

class RandomSensor(SealSensor):
    def __init__(self):
        super(RandomSensor, self).__init__("Random")
        self.useFunction.value = "randomNumber()"
        self.readFunction.value = "randomNumber()"
        self.extraConfig.value = "USE_RANDOM=y"
        self.extraIncludes.value = "#include <random.h>"
        # parameters
        self.min = SealParameter(0, ["0", "1", "2", "5", "10", "100", "1000"])
        self.max = SealParameter(0xffff, ["1", "2", "5", "10", "100", "1000", "65535"])
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        min = self.getParameterValue("min", useCaseParameters)
        max = self.getParameterValue("max", useCaseParameters)
        if (isinstance(min, int) or isinstance(min, long)) \
                and (isinstance(max, int) or isinstance(max, long)):
            modulo = max - min + 1
            if modulo == 0x10000:
                return self.useFunction.value
            # TODO: cast the result to uint16_t, but only in case of signed int overflow
            return "{} % {} + {}".format(self.useFunction.value, modulo, min)
        return "{} % ({} - {} + 1) + {}".format(self.useFunction.value, max, min, min)

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
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        counterPeriod = self.getParameterValue("counterperiod", useCaseParameters)
        return "getJiffies() / {0}".format(counterPeriod)

#
# System time sensor.
# At the moment, timestamp is the same as system uptime seconds.
#
class SystemTimeSensor(SealSensor):
    def __init__(self):
        super(SystemTimeSensor, self).__init__("SystemTime")
        self.useFunction.value = "getSyncTimeSec()"
        self.readFunction.value = "getSyncTimeSec()"

class TimestampSensor(SealSensor):
    def __init__(self):
        super(TimestampSensor, self).__init__("Timestamp")
        self.useFunction.value = "getSyncTimeSec()"
        self.readFunction.value = "getSyncTimeSec()"

# TODO: allow aliases!
class UptimeSensor(SealSensor):
    def __init__(self):
        super(UptimeSensor, self).__init__("Uptime")
        self.useFunction.value = "getTimeSec()"
        self.readFunction.value = "getTimeSec()"

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
        self._readFunctionDependsOnParams = True

#
# Predefined waveform sensor: 
#    +----+    +----+    +----+
#  --+    +----+    +----+    +--
#
class SquareWaveSensor(WaveSensor):
    def __init__(self):
        super(SquareWaveSensor, self).__init__("SquareWave")

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        wavePeriod = self.getParameterValue("waveperiod", useCaseParameters)
        low = self.getParameterValue("low", useCaseParameters)
        high = self.getParameterValue("high", useCaseParameters)
        return "(getTimeMs() % {0} < ({0} / 2)) ? {1} : {2}".format(wavePeriod, low, high)

#
# Predefined waveform sensor:
#   ^   ^   ^
#  / \ / \ / \
# /   v   v   v
#
class TriangleWaveSensor(WaveSensor):
    def __init__(self):
        super(TriangleWaveSensor, self).__init__("TriangleWave")
        self.extraIncludes.value = "#include <lib/algo.h>"

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        wavePeriod = self.getParameterValue("waveperiod", useCaseParameters)
        low = self.getParameterValue("low", useCaseParameters)
        high = self.getParameterValue("high", useCaseParameters)
        return "signalTriangleWave({}, {}, {})".format(wavePeriod, low, high)

#
# Predefined waveform sensor:
#   /  /  /
#  /| /| /|
# / |/ |/ |
#
class SawtoothWaveSensor(WaveSensor):
    def __init__(self):
        super(SawtoothWaveSensor, self).__init__("SawtoothWave")
        self.extraIncludes.value = "#include <lib/algo.h>"

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        wavePeriod = self.getParameterValue("waveperiod", useCaseParameters)
        low = self.getParameterValue("low", useCaseParameters)
        high = self.getParameterValue("high", useCaseParameters)
        return "signalSawtoothWave({}, {}, {})".format(wavePeriod, low, high)

#
# Predefined waveform sensor: sine wave
#
class SineWaveSensor(WaveSensor):
    def __init__(self):
        super(SineWaveSensor, self).__init__("SineWave")

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        wavePeriod = self.getParameterValue("waveperiod", useCaseParameters)
        low = self.getParameterValue("low", useCaseParameters)
        high = self.getParameterValue("high", useCaseParameters)
        return "signalSineWave({}, {}, {})".format(wavePeriod, low, high)

class LightSensor(SealSensor):
    def __init__(self):
        super(LightSensor, self).__init__("Light")
        self.useFunction.value = "lightRead()"
        self.readFunction.value = "lightRead()"

class TotalSolarRadiationSensor(SealSensor):
    def __init__(self):
        super(TotalSolarRadiationSensor, self).__init__("TotalSolarRadiation")
        self.useFunction.value = "totalSolarRadiationRead()"
        self.readFunction.value = "totalSolarRadiationRead()"

class PhotosyntheticRadiationSensor(SealSensor):
    def __init__(self):
        super(PhotosyntheticRadiationSensor, self).__init__("PhotosyntheticRadiation")
        self.useFunction.value = "photosyntheticRadiationRead()"
        self.readFunction.value = "photosyntheticRadiationRead()"

class HumiditySensor(SealSensor):
    def __init__(self):
        super(HumiditySensor, self).__init__("Humidity")
        self.useFunction.value = "humidityRead()"
        self.readFunction.value = "humidityRead()"
        self.onFunction.value = "humidityOn()"
        self.offFunction.value = "humidityOff()"
        self.errorFunction = SealAdvancedParameter("humidityIsError()")
        self.extraConfig.value = "USE_HUMIDITY=y"

# the following for are not implemented
class TemperatureSensor(SealSensor):
    def __init__(self):
        super(TemperatureSensor, self).__init__("Temperature")
        self.useFunction.value = "temperatureRead()"
        self.readFunction.value = "temperatureRead()"

class InternalTemperatureSensor(SealSensor):
    def __init__(self):
        super(InternalTemperatureSensor, self).__init__("InternalTemperature")
        self.useFunction.value = "internalTemperatureRead()"
        self.readFunction.value = "internalTemperatureRead()"

class SoilHumiditySensor(SealSensor):
    def __init__(self):
        super(SoilHumiditySensor, self).__init__("SoilHumidity")
        self.useFunction.value = "soilHumidityRead()"
        self.readFunction.value = "soilHumidityRead()"

class SoilTemperatureSensor(SealSensor):
    def __init__(self):
        super(SoilTemperatureSensor, self).__init__("SoilTemperature")
        self.useFunction.value = "soilTemperatureRead()"
        self.readFunction.value = "soilTemperatureRead()"

class AnalogInputSensor(SealSensor):
    def __init__(self):
        super(AnalogInputSensor, self).__init__("AnalogIn")
        self.useFunction.value = "adcRead(1)"
        self.readFunction.value = "adcRead(1)"
        self.channel = SealParameter(0, ["0", "1", "2", "3", "4", "5", "6", "7"])
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        # print "calculateParameterValue: ", parameter
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        channel = self.getParameterValue("channel", useCaseParameters)
        if channel is None: channel = 1
        return "adcRead({})".format(channel)

class DigitalInputSensor(SealSensor):
    def __init__(self):
        super(DigitalInputSensor, self).__init__("DigitalIn")
        self.useFunction.value = "pinRead(1, 0)"
        self.readFunction.value = "pinRead(1, 0)"
        self.pin = SealParameter(0, ["0", "1", "2", "3", "4", "5", "6", "7"])
        self.port = SealParameter(1, ["1", "2", "3", "4", "5", "6"])
        self._readFunctionDependsOnParams = True
        # interrupt related configuration
        self.interrupt = SealParameter(None, [False, True])
        self.rising = SealParameter(None, [False, True])
        self.falling = SealParameter(None, [False, True]) # inverse of rising edge
        self.aliases["rising"] = "risingedge"
        self.aliases["falling"] = "fallingedge"
        self.interruptPin = SealParameter(0, ["0", "1", "2", "3", "4", "5", "6", "7"])
        self.interruptPort = SealParameter(1, ["1", "2", "3", "4", "5", "6"])

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        port = self.getParameterValue("port", useCaseParameters)
        pin = self.getParameterValue("pin", useCaseParameters)
        if port is None: port = 1
        if pin is None: pin = 0
        return "pinRead({}, {})".format(port, pin)

#######################################################
class SealActuator(SealComponent):
    def __init__(self, name):
        super(SealActuator, self).__init__(TYPE_ACTUATOR, name)
        self.onFunction = SealAdvancedParameter(None)
        self.offFunction = SealAdvancedParameter(None)
        self.writeFunction = SealAdvancedParameter(None)
        self.on = SealParameter(None, [False, True])
        self.off = SealParameter(None, [False, True])
        # "blink X" is shortcut syntax for "times 2, period X"
        self.blink = SealParameter(None, ['50', '100', '200'])

class LedAct(SealActuator):
    def __init__(self):
        super(LedAct, self).__init__("Led")
        self.useFunction.value = "ledToggle()"
        self.readFunction.value = "ledGet()"
        self.onFunction.value = "ledOn()"
        self.offFunction.value = "ledOff()"
        self.writeFunction.value = "if (value) ledOn(); else ledOff()"

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" \
                and parameter != "useFunction" \
                and parameter != "onFunction" \
                and parameter != "offFunction" \
                and parameter != "writeFunction":
            return SealActuator.calculateParameterValue(self, parameter, useCaseParameters)
        myid = self.getParameterValue("id", useCaseParameters)
        ledName = ("led" + str(myid)) if myid else "led"
        if parameter == "useFunction":
            return ledName + "Toggle()"
        if parameter == "readFunction":
            return ledName + "Get()"
        if parameter == "onFunction":
            return ledName + "On()"
        if parameter == "offFunction":
            return ledName +  "Off()"
        if parameter == "writeFunction":
            return "if (value) " +ledName + "On(); else " + ledName + "Off()"
        return None

class RedLedAct(SealActuator):
    def __init__(self):
        super(RedLedAct, self).__init__("RedLed")
        self.useFunction.value = "redLedToggle()"
        self.readFunction.value = "redLedGet()"
        self.onFunction.value = "redLedOn()"
        self.offFunction.value = "redLedOff()"
        self.writeFunction.value = "if (value) redLedOn(); else redLedOff()"

class BlueLedAct(SealActuator):
    def __init__(self):
        super(BlueLedAct, self).__init__("BlueLed")
        self.useFunction.value = "blueLedToggle()"
        self.readFunction.value = "blueLedGet()"
        self.onFunction.value = "blueLedOn()"
        self.offFunction.value = "blueLedOff()"
        self.writeFunction.value = "if (value) blueLedOn(); else blueLedOff()"

class GreenLedAct(SealActuator):
    def __init__(self):
        super(GreenLedAct, self).__init__("GreenLed")
        self.useFunction.value = "greenLedToggle()"
        self.readFunction.value = "greenLedGet()"
        self.onFunction.value = "greenLedOn()"
        self.offFunction.value = "greenLedOff()"
        self.writeFunction.value = "if (value) greenLedOn(); else greenLedOff()"

class YellowLedAct(SealActuator):
    def __init__(self):
        super(YellowLedAct, self).__init__("YellowLed")
        self.useFunction.value = "yellowLedToggle()"
        self.readFunction.value = "yellowLedGet()"
        self.onFunction.value = "yellowLedOn()"
        self.offFunction.value = "yellowLedOff()"
        self.writeFunction.value = "if (value) yellowLedOn(); else yellowLedOff()"

class DigitalOutputAct(SealActuator):
    def __init__(self):
        super(DigitalOutputAct, self).__init__("DigitalOut")
        self.useFunction.value = "pinToggle(1, 0)"
        self.readFunction.value = "pinRead(1, 0)"
        self.onFunction.value = "pinSet(1, 0)"
        self.offFunction.value = "pinClear(1, 0)"
        self.writeFunction.value = "if (value) pinSet(1, 0); else pinClear(1, 0)"
        self.pin = SealParameter(0, ["0", "1", "2", "3", "4", "5", "6", "7"])
        self.port = SealParameter(1, ["1", "2", "3", "4", "5", "6"])

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" \
                and parameter != "useFunction" \
                and parameter != "onFunction" \
                and parameter != "offFunction" \
                and parameter != "writeFunction":
            return SealActuator.calculateParameterValue(self, parameter, useCaseParameters)
        port = self.getParameterValue("port", useCaseParameters)
        pin = self.getParameterValue("pin", useCaseParameters)
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
        if parameter == "writeFunction":
            return "if (value) pinSet" + args + "; else pinClear" + args
        return None

class AnalogOutputAct(SealActuator):
    def __init__(self):
        super(AnalogOutputAct, self).__init__("AnalogOut")
        self.useFunction.value = "pwmWrite(1, 0, 128)"
        self.readFunction.value = "0"
        self.onFunction.value = "pwmWrite(1, 0, 255)"
        self.offFunction.value = "pwmWrite(1, 0, 0)"
        #self.writeFunction.value = "pwmWrite(1, 0, value ? 255 : 0)"
        self.writeFunction.value = "pwmWrite(1, 0, value)"
        self.pin = SealParameter(0, ["0", "1", "2", "3", "4", "5", "6", "7"])
        self.port = SealParameter(1, ["1", "2", "3", "4", "5", "6"])
        self.value = SealParameter(128, ["0", "64", "128", "196", "255"])

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" \
                and parameter != "onFunction" \
                and parameter != "offFunction" \
                and parameter != "writeFunction":
            return SealActuator.calculateParameterValue(self, parameter, useCaseParameters)
        port = self.getParameterValue("port", useCaseParameters)
        pin = self.getParameterValue("pin", useCaseParameters)
        value = self.getParameterValue("value", useCaseParameters)
        if port is None: port = 1
        if pin is None: pin = 0
        if value is None: value = 128
        args = "(" + str(port) + ", " + str(pin) + ", "
        if parameter == "useFunction":
            return "pwmWrite" + args  + str(value) + ")"
        if parameter == "onFunction":
            return "pinSet" + args + "255)"
        if parameter == "offFunction":
            return "pinClear" + args + "0)"
        if parameter == "writeFunction":
            # return "pwmWrite" + args + "value ? 255 : 0)"
            return "pwmWrite" + args + "value)"
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

        self.extraConfig.value = "USE_BEEPER=y"
        self.extraIncludes.value = "#include <beeper.h>"

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
        self.arg4 = SealAdvancedParameter(None)
        self.arg5 = SealAdvancedParameter(None)
        self.arg6 = SealAdvancedParameter(None)
        self.arg7 = SealAdvancedParameter(None)
        self.arg8 = SealAdvancedParameter(None)
        self.arg9 = SealAdvancedParameter(None)
        # output reading to some SEAL output?
        self.out = SealParameter(None)

# not implemented
class WateringAct(SealActuator):
    def __init__(self):
        super(WateringAct, self).__init__("Watering")
        self.useFunction.value = "0"

#######################################################
class SealOutput(SealComponent):
    def __init__(self, name):
        super(SealOutput, self).__init__(TYPE_OUTPUT, name)
        # create packets? (if aggregate=true)
        self.aggregate = SealAdvancedParameter(True, [False, True])
        # packet fields (crc is always included...)
        # self.crc = SealParameter(False, [False, True])
        self.address = SealParameter(False, [False, True])
        self.timestamp = SealParameter(True, [False, True])
        self.sequencenumber = SealAdvancedParameter(False, [False, True])
        self.issent = SealAdvancedParameter(False, [False, True])
        # The name of the file, FROM which to output
        # but "File" outputs has "filename" parameter TO which to output; do not confuse!
        # Automatically generated if None.
        self.filename = SealAdvancedParameter(None) # TODO: make not advanced!
        self.aliases["file"] = "filename" # synonym Nr.1 
        self.aliases["name"] = "filename" # synonym Nr.2
        # for FromFile outputs
        self.where = SealAdvancedParameter(None)

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
        # self.crc.value = True # true by default
        self.address.value = True # true by default

class ActiveMessageOutput(SealOutput):
    def __init__(self):
        super(ActiveMessageOutput, self).__init__("ActiveMessage")
        self.useFunction.value = "activeMessageSend(&activemessagePacket, sizeof(activemessagePacket))"
        self.extraIncludes.value = "#include <lib/activemsg.h>"
        self.extraConfig.value = "USE_ACTIVE_MSG_RADIO=y"

class InternalFlashOutput(SealOutput):
    def __init__(self):
        super(InternalFlashOutput, self).__init__("InternalFlash")
        self.useFunction.value = "flashWrite(&internalflashPacket, sizeof(internalflashPacket))"

class ExternalFlashOutput(SealOutput):
    def __init__(self):
        super(ExternalFlashOutput, self).__init__("ExternalFlash")
        # XXX: radio should be re-initialized only on those platforms where it conficts with flash
        self.useFunction.value = """
    flashStreamWriteRecord(&externalflashPacket, sizeof(externalflashPacket), true);
    radioReinit()"""
        self.extraIncludes.value = "#include <flash_stream.h>"
        self.extraConfig.value = "USE_FLASH_STREAM=y"

class SdCardOutput(SealOutput):
    def __init__(self):
        super(SdCardOutput, self).__init__("SdCard")
        # XXX: radio should be re-initialized only on those platforms where it conficts with SD card
        self.useFunction.value = """
    sdStreamWriteRecord(&sdcardPacket, sizeof(sdcardPacket), true);
    radioReinit()"""
        self.extraIncludes.value = "#include <sdstream.h>"
        self.extraConfig.value = "USE_SDCARD_STREAM=y"
        self.address.value = True # true by default

# "local storage" (i.e. [external] flash or SD card is defined depending on platform
# on telosb, local storage is synonym for external flash
class LocalStorageOutput(SealOutput):
    def __init__(self):
        super(LocalStorageOutput, self).__init__("LocalStorage")
        # ext flash by default
        self.useFunction.value = "extFlashWrite(0, &localstoragePacket, sizeof(localstoragePacket))"
        self.extraIncludes.value = "#include <extflash.h>"
        self.extraConfig.value = "USE_EXT_FLASH=y"

class FileOutput(SealOutput):
    def __init__(self):
        super(FileOutput, self).__init__("File")
        self.useFunction.value = "filePrint()"
        # a file can be text or binary - allow both parameter names wih inverse meaning
        # XXX: make text files by default - more intuitive
        self.text = SealParameter(None, [False, True])
        self.binary = SealParameter(None, [False, True])
        self.extraIncludes.value = "#include <fs.h>"
        self.extraConfig.value = "USE_FS=y"

class NetworkOutput(SealOutput):
    def __init__(self):
        super(NetworkOutput, self).__init__("Network")
        self.address.value = True # true by default
        # create a socket, if not alreay, and use it to send data to the root
        self.useFunction.value = """
    static Socket_t socket;
    if (socket.port == 0) {
        socketOpen(&socket, NULL);
        socketBind(&socket, SEAL_DATA_PORT);
        socketSetDstAddress(&socket, MOS_ADDR_ROOT);
    }
    socketSend(&socket, &networkPacket, sizeof(networkPacket))"""
        self.protocol = SealParameter("NULL", ["NULL", "CSMA", "CSMA_ACK", "SAD"])
        self.routing = SealParameter("DV", ["DV", "SAD"])
        self.extraIncludes.value = "#include <net/mac.h>"
        self.extraConfig.value = "USE_NET=y"

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "extraConfig":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        protocol = self.getParameterValue("protocol", useCaseParameters)
        if protocol is None: protocol = "NULL"
        else:
            if "asString" in dir(protocol): protocol = protocol.asString()
            protocol = protocol.upper()
        macProto = "USE_NET=y\n" + "CONST_MAC_PROTOCOL=MAC_PROTOCOL_" + protocol
        protocol = self.getParameterValue("routing", useCaseParameters)
        if protocol is None: protocol = "DV"
        else:
            if "asString" in dir(protocol): protocol = protocol.asString()
            protocol = protocol.upper()
        routingProto = "\nCONST_ROUTING_PROTOCOL=ROUTING_PROTOCOL_" + protocol
        return macProto + routingProto
