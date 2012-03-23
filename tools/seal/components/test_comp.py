#
# Tmote-specific components
#

from telosb_comp import *

commonParameters = [("param1", None), ("param2", None), ("param3", None)]

foobar = ComponentSpecification(TYPE_ACTUATOR, 'Foobar', commonParameters)
foobar1 = ComponentSpecification(TYPE_ACTUATOR, 'Foobar1', commonParameters)
foobar2 = ComponentSpecification(TYPE_ACTUATOR, 'Foobar2', commonParameters)
foobar3 = ComponentSpecification(TYPE_ACTUATOR, 'Foobar3', commonParameters)
foo = ComponentSpecification(TYPE_ACTUATOR, 'Foo', commonParameters)
bar = ComponentSpecification(TYPE_ACTUATOR, 'Bar', commonParameters)

sfoobar = ComponentSpecification(TYPE_SENSOR, 'Foobar', commonParameters)
sfoobar1 = ComponentSpecification(TYPE_SENSOR, 'Foobar1', commonParameters)
sfoobar2 = ComponentSpecification(TYPE_SENSOR, 'Foobar2', commonParameters)
sfoobar3 = ComponentSpecification(TYPE_SENSOR, 'Foobar3', commonParameters)

ofoobar = ComponentSpecification(TYPE_OUTPUT, 'Foobar', commonParameters)
ofoobar1 = ComponentSpecification(TYPE_OUTPUT, 'Foobar1', commonParameters)
ofoobar2 = ComponentSpecification(TYPE_OUTPUT, 'Foobar2', commonParameters)
ofoobar3 = ComponentSpecification(TYPE_OUTPUT, 'Foobar3', commonParameters)

includes = ("includeFiles", "#include \"foobar.h\"\n#include \"baz.h\"")
cooltestcomponent = ComponentSpecification(TYPE_SENSOR, "CoolTestComponent", sensorParameters + [includes])
