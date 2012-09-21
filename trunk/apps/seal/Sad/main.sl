// define red led to blink
define HelperLed RedLed, blink;

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
//output Serial;
