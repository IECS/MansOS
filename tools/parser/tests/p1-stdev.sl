const ACCEL_Z 2; // channel number

const THRESHOLD 100;

define AccelZ AnalogIn, channel ACCEL_Z;
define Deviation stdev(take(AccelZ, 10));

read Deviation;
when Deviation > THRESHOLD:
   use RedLed, on;
   use Beeper, on, duration 200, frequency 1000;
else:
   use RedLed, off;
end

