#
# Test platform components
#

from telosb import *

foobar = SealActuator('Foobar')
foobar1 = SealActuator('Foobar1')
foobar2 = SealActuator('Foobar2')
foobar3 = SealActuator('Foobar3')
foo = SealActuator('Foo')
bar = SealActuator('Bar')

sfoobar = SealSensor('Foobar')
sfoobar1 = SealSensor('Foobar1')
sfoobar2 = SealSensor('Foobar2')
sfoobar3 = SealSensor('Foobar3')

ofoobar = SealOutput('Foobar')
ofoobar.useFunction.value = "sendFoobar()"
ofoobar1 = SealOutput('Foobar1')
ofoobar2 = SealOutput('Foobar2')
ofoobar3 = SealOutput('Foobar3')

#includes = ("includeFiles", "#include \"foobar.h\"\n#include \"baz.h\"")
#cooltestcomponent = ComponentSpecification(TYPE_SENSOR, "CoolTestComponent", sensorParameters + [includes])

