#
# Tmote-specific components
#

from msp430_comp import *

#actuators = msp430_comp.actuators
#sensors = msp430_comp.sensors
#outputs = msp430_comp.outputs
#
#actuators.append("Led")
#actuators.append("RedLed")
#actuators.append("BlueLed")
#actuators.append("GreenLed")

ledParameters = commonParameters
ledFunctions = {"use" : "toggle{0}"}

led = ComponentSpecification(TYPE_ACTUATOR, "Led", ledParameters)
redled = ComponentSpecification(TYPE_ACTUATOR, "RedLed", ledParameters)
blueled = ComponentSpecification(TYPE_ACTUATOR, "BlueLed", ledParameters)
greenled = ComponentSpecification(TYPE_ACTUATOR, "GreenLed", ledParameters)

sensorParameters = commonParameters + [("dataSize", 2)]

sensorFunctions = {"use" : "toggle{0}"}

lightsensor = ComponentSpecification(TYPE_SENSOR, "Light", sensorParameters)
humiditysensor = ComponentSpecification(TYPE_HUMIDITY, "Humidity", sensorParameters + [("config", "USE_HUMIDITY=y")])

includes = ("includeFiles", "#include \"foobar.h\"\n#inlclude \"baz.h\"")
cooltestcomponent = ComponentSpecification(TYPE_SENSOR, "Light", sensorParameters + [includes])
