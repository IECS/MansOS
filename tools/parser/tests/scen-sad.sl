// simple version:
// read Light, period 10min;
// read Humidity, period 10min;
// read Temperature, period 10min;
// output SdCard;
// output Network, protocol CSMA;

// real life version:
config "USE_LONG_LIFETIME=y"; // long 64-bit counter
define HelperLed RedLed, blink;
// define battery voltage sensor
define Battery AnalogIn, channel constants.ADC_INTERNAL_VOLTAGE;
// define light sensor
when Variables.localAddress != 0x0796:
    define MyLight Light;
else:
    define MyLight SQ100Light;
end
// define all sensors to read
define AllSensors sync(MyLight, Humidity, Temperature, Battery);
// define our action
read AllSensors, period 10min, associate HelperLed, turnOnOff;
// define our outputs
output SdCard;
output Network, protocol SAD, routing SAD;
