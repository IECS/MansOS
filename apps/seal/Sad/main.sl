// use 64-bit counter
config "USE_LONG_LIFETIME=y";
config "USE_SOFT_SERIAL=y";
config "USE_SW_SERIAL_INTERRUPTS=y";
config "BAUDRATE=9600";
config "CONST_SINGLE_HOP=1";
config "USE_ADDRESSING=y";

// define red led to blink
define HelperLed RedLed, blink;

define Battery AnalogIn, channel constants.ADC_INTERNAL_VOLTAGE;

// define light sensor
// when Variables.localAddress != 0x0796:
//     define MyLight Light;
// else:
//     define MyLight SQ100Light;
// end
define MyLight Light;

// define all sensors to read
define AllSensors sync(MyLight, Humidity, Temperature, Battery);

// define our action
read AllSensors, period 10min, associate HelperLed, turnOnOff, sync;

// define our outputs
output SdCard;
//output Network, protocol SAD, routing SAD;
output Serial;

pattern P (100, 10000);       // miliseconds
use redLed, pattern P;
