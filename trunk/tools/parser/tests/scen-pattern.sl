// define a signal using pattern declaration with microsecond intervals
pattern P (300us, 100us, 300us, 400us, 300us, 200us);
// then output this pattern to a specific pin
use DigitalOut, port 1, pin 6, pattern P;
