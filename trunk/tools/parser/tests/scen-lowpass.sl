// constants
const NUM_SAMPLES 5;
const ADJUSTMENT_WEIGHT 5;
const ACCEL_CHANNEL 2;
// define out accelerometer sensor (make sure cache is disabled)
define Accelerometer AnalogIn, channel ACCEL_CHANNEL, cache False;
// define accelerometer sensor with low pass filter applied
define SmoothedAccelerometer smoothen(Accelerometer, NUM_SAMPLES, ADJUSTMENT_WEIGHT);
// read the noise-removed sensor
read SmoothedAccelerometer, period 10ms;
// output to serial port
output Serial, aggregate;
