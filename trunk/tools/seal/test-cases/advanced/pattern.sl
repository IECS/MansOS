
// SOS signal in morse code (...---...). Pause time is 300 ms, dot: 300ms, line: 900ms
pattern SOS [300,300,300,300,300,300, // . . .
             900,300,900,300,900,300, // - - -
             300,300,300,300,300,300, // . . .
             3000] ms; // longer pause at the end

use RedLed, pattern SOS;
