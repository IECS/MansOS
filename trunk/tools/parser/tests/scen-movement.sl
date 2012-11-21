const THRESHOLD_LOW  50;
const THRESHOLD_HIGH 100;

set isDriving False; // vehicle mode

define AccelX AnalogIn, channel 1, period 10ms;
define AccelY AnalogIn, channel 2, period 10ms;
define AccelZ AnalogIn, channel 3, period 10ms;

define X_dev stdev(take(AccelX, 100));
define Y_dev stdev(take(AccelY, 100));
define Z_dev stdev(take(AccelZ, 100));

define SDev sum(X_dev, Y_dev, Z_dev), lazy;

read SDev;
when SDev > THRESHOLD_HIGH:
    set isDriving True;   // movement detected!
elsewhen SDev < THRESHOLD_LOW:
    set isDriving False;  // absence of movement detected!
end
