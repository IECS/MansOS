config "CONST_CONDITION_EVALUATION_INTERVAL=100";

// 0g is ~2080 accel value.
const THRESHOLD_LOW 1930;
const THRESHOLD_HIGH 2230;

// channels
define AccelX AnalogIn, channel 0;
define AccelY AnalogIn, channel 1;
define AccelZ AnalogIn, channel 2;

when AccelX > THRESHOLD_LOW and AccelX < THRESHOLD_HIGH
        and AccelY > THRESHOLD_LOW and AccelY < THRESHOLD_HIGH
        and AccelZ > THRESHOLD_LOW and AccelZ < THRESHOLD_HIGH:
   use RedLed, on;
   use Beeper, on, duration 200, frequency 1000;
else:
   use RedLed, off;
end
