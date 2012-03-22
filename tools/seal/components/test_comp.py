#
# Tmote-specific components
#

from component_def import *

commonParameters = [("param1", None), ("param2", None), ("param3", None)]

foobar = ComponentSpecification(TYPE_ACTUATOR, 'Foobar', commonParameters)
foobar1 = ComponentSpecification(TYPE_ACTUATOR, 'Foobar1', commonParameters)
foobar2 = ComponentSpecification(TYPE_ACTUATOR, 'Foobar2', commonParameters)
foobar3 = ComponentSpecification(TYPE_ACTUATOR, 'Foobar3', commonParameters)

sfoobar = ComponentSpecification(TYPE_SENSOR, 'Foobar', commonParameters)
sfoobar1 = ComponentSpecification(TYPE_SENSOR, 'Foobar1', commonParameters)
sfoobar2 = ComponentSpecification(TYPE_SENSOR, 'Foobar2', commonParameters)
sfoobar3 = ComponentSpecification(TYPE_SENSOR, 'Foobar3', commonParameters)
