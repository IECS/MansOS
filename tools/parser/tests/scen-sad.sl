// simple version:
// read Light, period 10min;
// read Humidity, period 10min;
// read Temperature, period 10min;
// output SdCard;
// output Network, protocol CSMA;

// real life version:
config "USE_LONG_LIFETIME=y";
define HelperLed RedLed, blink;
when Variables.localAddress == 0x0796:
 	define TheLight SQ100Light;
else:
  	define TheLight Light;
end
define AllSensors sync(TheLight, Humidity, Temperature);
read AllSensors, period 10min, associate HelperLed, turnOnOff;
output SdCard;
output Network, protocol CSMA;
