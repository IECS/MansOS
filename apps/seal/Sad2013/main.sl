// use 64-bit counter
config "USE_LONG_LIFETIME=y";

// define red led to blink
define HelperLed RedLed, blink;

define Battery AnalogIn, channel constants.ADC_INTERNAL_VOLTAGE;

// define all sensors to read
define AllSensors sync(SQ100Light, Light, Humidity, Temperature, Battery);

// define our action
read AllSensors, period 15min, associate HelperLed, turnOnOff;

pattern P (100,10000);       // miliseconds

use redLed, pattern P;

// define our outputs
output SdCard, timestamp, address;
//output ExternalFlash, timestamp, address;
//output Network;
output Serial;
