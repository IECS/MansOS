#
# Tmote-specific components
#

import msp430_comp

actuators = msp430_comp.actuators
sensors = msp430_comp.sensors
outputs = msp430_comp.outputs

actuators.append("Led")
actuators.append("RedLed")
actuators.append("BlueLed")
actuators.append("GreenLed")

