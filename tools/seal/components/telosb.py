#
# Tmote-specific components
#

from msp430 import *

led = LedAct()
redLed = RedLedAct()
blueled = BlueLedAct()
greenled = GreenLedAct()

light = LightSensor()
humidity = HumiditySensor()

externalFlash = ExternalFlashOutput()

# use ext flash as local storage
localStorage.useFunction.value = externalFlash.useFunction.value
