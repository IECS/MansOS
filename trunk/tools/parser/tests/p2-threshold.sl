// Z measurements:
// 1680 - norm       (1g)
// 2080 - weightless (0g)
// 1280 (assumed) -  (2g)
const THRESHOLD_LOW 1360;
const THRESHOLD_HIGH 2000;

// channel number
const ACCEL_Z 2;

define AccelZ AnalogIn, channel ACCEL_Z;

when AccelZ < THRESHOLD_LOW or AccelZ > THRESHOLD_HIGH:
   use RedLed, on;
   use Beeper, on, duration 200, frequency 1000;
else:
   use RedLed, off;
end
