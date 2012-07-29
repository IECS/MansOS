const ACCEL_Z 2; // channel number

const THRESHOLD 100;

define AccelZ AnalogIn, channel ACCEL_Z;
define Difference multiply(stdev(take(AccelZ, 2)), 2)

when Difference > THRESHOLD:
   use redLed, on;
   use Beeper, on, duration 200, frequency 1000;
else:
   use redLed, off;
end
