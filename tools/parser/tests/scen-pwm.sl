// define input signal channel
define InputPin AnalogIn, channel 6;
// define output signal channel
define OutputPin AnalogOut, pin 3, port 3;
// define input->output mapping: input range is 120..255, output: 0..100.
define MySignal map(InputPin, 120, 255, 0, 100), out OutputPin;
// define the action
use MySignal;
