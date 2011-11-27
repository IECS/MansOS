// define system parameters
parameter battery 2700mAh;
parameter routingProtocol GPSR;

// blink red led periodically
use RedLed, period 1000ms;

// turn green led on once the program has started
use GreenLed, on_at 2000ms;

// blink blue led; faster at the start
when System.time < 5s:
     use BlueLed, period 100ms;
else:
     use BlueLed, period 1000ms;
end

// read onboard light sensor once every 10 seconds
read Light, period 10s;

// read a specific sensor once every 10 seconds
read APDS9300, period 10s;

// define a constant time value
const PERIOD 10s;

read Humidity, period PERIOD;

// by default, output all data read to serial port; the baudrate is 38400 by default, but specify it explicitly
sink Serial, baudrate 38400;

// also output to radio (aggregate=yes means put all data in one packet; by default on for radio, off for serial port)
sink Radio, aggregate yes;

// also output to MAC protocol, but only when a base station is detected nearby
sink MAC, protocol CSMA, when MAC.baseStationIsReachable;

// also output to higher level network-stack
sink NetworkSocket, port 100;

// save light sensor values (but not humidity sensor!) to flash in case battery voltage is above 2.7V
sink Flash {Light, APDS9300}, when System.voltage > 2.7V;
