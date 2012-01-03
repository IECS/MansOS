// do nothing, just declare that red LED will be used in the program
use RedLed;

// red led is on when system starts
use RedLed, turn_on;

// red led is turned off when a condition is fullfilled
// (conditions are checked in the main loop at least once per minute)
use RedLed, turn_off, when 1 < 2;

// blue led is always on in daytime
when System.isDaytime: use BlueLed, turn_on;

// blue led blinks periodically in night
when System.isDaytime = false: use BlueLed, period 1000;

// alternatively:
when System.isDaytime:
     use BlueLed, turn_on;
else:
     use BlueLed, period 1000;
end

// compilation error
// when System.isDaytime = false use BlueLed, period 2000;

// blink one time on radio rx
use GreenLed, blinkOnce, when System.radioRx;

// blink two times on radio rx error
use GreenLed, blinkTwice, when System.radioRxError;

// blink three times on radio tx
use GreenLed, blinkTimes 3, when System.radioTx;

// during daytime also read sensors
when System.isDaytime
     read Light, period 2s;
     read Humidity, period 2s;
end

// send data to serial
sendto Serial;

// also send data to radio
sendto Radio;

// always when the condition is checked print a message (parameters?)
// when 1
//     sink Serial, print "In main loop...";
//     sink Radio, print "param param...";
//end
