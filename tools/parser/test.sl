config "CONST_CONDITION_EVALUATION_INTERVAL=100";

const ACCEL_Z 2; // channel number

define AccelZ AnalogIn, channel ACCEL_Z;
define Deviation stdev(take(AccelZ, 10));

when Deviation > 100:
   use redLed, on;
   use Beeper, on, duration 200, frequency 1000;
else:
   use redLed, off;
end
