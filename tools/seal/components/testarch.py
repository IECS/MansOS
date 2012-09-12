#
# Test platform components
#

from msp430 import *

class TestActuator(SealActuator):
    def __init__(self, name):
        super(TestActuator, self).__init__(name)
        self.param = SealParameter(None)
        self.param1 = SealParameter(None)
        self.param2 = SealParameter(None)
        self.param3 = SealParameter(None)

class TestSensor(SealSensor):
    def __init__(self, name):
        super(TestSensor, self).__init__(name)
        self.param = SealParameter(None)
        self.param1 = SealParameter(None)
        self.param2 = SealParameter(None)
        self.param3 = SealParameter(None)

class TestOutput(SealOutput):
    def __init__(self, name):
        super(TestOutput, self).__init__(name)
        self.param = SealParameter(None)
        self.param1 = SealParameter(None)
        self.param2 = SealParameter(None)
        self.param3 = SealParameter(None)

foobar = TestActuator('Foobar')
foobar1 = TestActuator('Foobar1')
foobar2 = TestActuator('Foobar2')
foobar3 = TestActuator('Foobar3')
foo = TestActuator('Foo')
bar = TestActuator('Bar')

sfoobar = TestSensor('SFoobar')
sfoobar.readFunction.value = ""

ofoobar = TestOutput('OFoobar')
ofoobar.useFunction.value = "sendFoobar()"

#includes = ("includeFiles", "#include \"foobar.h\"\n#include \"baz.h\"")
#cooltestcomponent = ComponentSpecification(TYPE_SENSOR, "CoolTestComponent", sensorParameters + [includes])

#temperature = TemperatureSensor()
#internalTemperature = InternalTemperatureSensor()
soilHumidity = SoilHumiditySensor()
soilTemperature = SoilTemperatureSensor()

watering = WateringAct()


class SeismicSensor(SealSensor):
    def __init__(self):
        super(SeismicSensor, self).__init__("SeismicSensor")
        self.useFunction.value = "0"
        self.readFunction.value = "0"

class AcousticSensor(SealSensor):
    def __init__(self):
        super(AcousticSensor, self).__init__("AcousticSensor")
        self.useFunction.value = "0"
        self.readFunction.value = "0"

fileo = FileOutput()
seismic = SeismicSensor()
acoustic = AcousticSensor()


class LightWithIdSensor(SealSensor):
    def __init__(self):
        super(LightWithIdSensor, self).__init__("LightWithId")
        self.useFunction.value = "lightRead()"
        self.readFunction.value = "lightRead()"
        self._readFunctionDependsOnParams = True

    def calculateParameterValue(self, parameter, useCaseParameters):
        if parameter != "readFunction" and parameter != "useFunction":
            return SealSensor.calculateParameterValue(self, parameter, useCaseParameters)
        id = int(self.getParameterValue("id", useCaseParameters))
        if id is None: id = 0
        return "lightWithIdRead(" + str(id) + ")"

lightWithId = LightWithIdSensor()
