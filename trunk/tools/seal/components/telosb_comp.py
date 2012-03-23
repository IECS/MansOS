#
# Tmote-specific components
#

from msp430_comp import *

ledParameters = commonParameters + [("useFunction", "{0}Toggle()")]

led = ComponentSpecification(TYPE_ACTUATOR, "Led", ledParameters)
redled = ComponentSpecification(TYPE_ACTUATOR, "RedLed", ledParameters)
blueled = ComponentSpecification(TYPE_ACTUATOR, "BlueLed", ledParameters)
greenled = ComponentSpecification(TYPE_ACTUATOR, "GreenLed", ledParameters)

sensorParameters = commonParameters + [("dataSize", 2), ("useFunction", "{0}Read()")]

lightsensor = ComponentSpecification(TYPE_SENSOR, "Light", sensorParameters)
humiditysensor = ComponentSpecification(TYPE_SENSOR, "Humidity",
   sensorParameters + [("config", "USE_HUMIDITY=y"), ("includeFiles", "#include <hil/humidity.h>")])
