// use 64-bit counter
config "USE_LONG_LIFETIME=y";
config "CONDITION_EVALUATION_INTERVAL=10000";

// define red led to blink
define HelperLed RedLed, blink;

// define light sensor
when Variables.localAddress == 0x0796:
    define TheLight SQ100Light;
else:
    define TheLight Light;
end

// alternative: define light sensor
// define TheLight Light;

// define all sensors to read
define AllSensors sync(TheLight, Humidity, Temperature);

// define our action
//read AllSensors, period 10min, associate HelperLed, turnOnOff;
read AllSensors, period 30s, associate HelperLed, turnOnOff;

// define our outputs
output SdCard;
output Network, protocol SAD, routing SAD;
output Serial;
