#
# Test platform components
#

from msp430 import *

foobar = SealActuator('Foobar')
foobar1 = SealActuator('Foobar1')
foobar2 = SealActuator('Foobar2')
foobar3 = SealActuator('Foobar3')
foo = SealActuator('Foo')
bar = SealActuator('Bar')

#sfoobar = SealSensor('Foobar')
#sfoobar1 = SealSensor('Foobar1')
#sfoobar2 = SealSensor('Foobar2')
#sfoobar3 = SealSensor('Foobar3')

ofoobar = SealOutput('Foobar')
ofoobar.useFunction.value = "sendFoobar()"
ofoobar1 = SealOutput('Foobar1')
ofoobar2 = SealOutput('Foobar2')
ofoobar3 = SealOutput('Foobar3')

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


class SQ100LightSensor(SealSensor):
    def __init__(self):
        super(SQ100LightSensor, self).__init__("SQ100Light")
        self.useFunction.value = "sq100LightRead()"
        self.readFunction.value = "sq100LightRead()"

sq100 = SQ100LightSensor()
