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
        self.extraConfig = SealParameter(None)  # config file line(s)
        self.extraIncludes = SealParameter(None) # #include<> line(s)
        self.period = SealParameter(1000, ['100', '200', '500', '1000', '2000'])
        self.pattern = None
        self.once = SealParameter(False, [False, True])
        self.useFunction = SealParameter(None)  # each usable component must define this
        self.readFunction = SealParameter(None)  # each readable component must define this
        components.append(self)

#######################################################
class SealSensor(SealComponent):
    def __init__(self, name):
        super(SealSensor, self).__init__(TYPE_SENSOR, name)
        self.readFunction = SealParameter(None)  # each sensor must define this
        self.dataSize = 2 # in bytes
        self.filter = SealParameter('', ['> 100'])
        self.average = SealParameter('', ['10'])
        self.stdev = SealParameter('', ['10'])

class Light(SealSensor):
    def __init__(self):
        super(Light, self).__init__("Light")
        self.useFunction.value = "lightRead()"
        self.readFunction.value = "lightRead()"

class Constant(SealSensor):
    def __init__(self):
        super(Constant, self).__init__("Constant")
        self.useFunction.value = "5"
        self.readFunction.value = "5"

class Random(SealSensor):
    def __init__(self):
        super(Random, self).__init__("Random")
        self.useFunction.value = "randomRand()"
        self.readFunction.value = "randomRand()"
        self.extraConfig = SealParameter("USE_RANDOM=y")
        self.extraIncludes = SealParameter("#include <random.h>")


class Humidity(SealSensor):
    def __init__(self):
        super(Humidity, self).__init__("Humidity")
        self.useFunction.value = "humidityRead()"
        self.readFunction.value = "humidityRead()"
        self.errorFunction = SealParameter("humidityIsError()")
        self.extraConfig = SealParameter("USE_HUMIDITY=y")
        self.extraIncludes = SealParameter("#include <hil/humidity.h>")

#######################################################
class SealActuator(SealComponent):
    def __init__(self, name):
        super(SealActuator, self).__init__(TYPE_ACTUATOR, name)
        self.useFunction = SealParameter(None)  # each actuator must define this

class Led(SealActuator):
    def __init__(self):
        super(Led, self).__init__("Led")
        self.useFunction.value = "ledToggle()"
        self.readFunction.value = "ledGet()"

class RedLed(SealActuator):
    def __init__(self):
        super(RedLed, self).__init__("RedLed")
        self.useFunction.value = "redLedToggle()"
        self.readFunction.value = "redLedGet()"

class BlueLed(SealActuator):
    def __init__(self):
        super(BlueLed, self).__init__("BlueLed")
        self.useFunction.value = "blueLedToggle()"
        self.readFunction.value = "blueLedGet()"

class GreenLed(SealActuator):
    def __init__(self):
        super(GreenLed, self).__init__("GreenLed")
        self.useFunction.value = "greenLedToggle()"
        self.readFunction.value = "greenLedGet()"

class Print(SealActuator):
    def __init__(self):
        super(Print, self).__init__("Print")
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
        self.sendFunction = None # each output must define this
        self.aggregate = SealParameter(True, [False, True])
        self.crc = SealParameter(False, [False, True])

class Serial(SealOutput):
    def __init__(self):
        super(Serial, self).__init__("Serial")
        self.useFunction.value = "serialPacketPrint()"
        self.baudrate = SealParameter(38400, ['9600', '38400', '57600', '115200'])
        self.aggregate.value = False # false by default

class Radio(SealOutput):
    def __init__(self):
        super(Radio, self).__init__("Radio")
        self.useFunction.value = "radioSend(&radioPacket, sizeof(radioPacket))"
        self.crc.value = True # true by default

class InternalFlash(SealOutput):
    def __init__(self):
        super(InternalFlash, self).__init__("InternalFlash")
        self.useFunction.value = "flashWrite(&internalFlashPacket, sizeof(internalFlashPacket))"

class ExternalFlash(SealOutput):
    def __init__(self):
        super(ExternalFlash, self).__init__("ExternalFlash")
        self.useFunction.value = "extFlashWrite(&externalFlashPacket, sizeof(externalFlashPacket))"
