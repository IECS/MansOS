// general examples
use Print, period 1000, format "arg1=%#04x arg2=%d\n", arg1 0x1234, arg2 17;

// print with a specific and default format string
use Print, format "light: %d", arg1 Light;
use Print, arg1 Light;
use Print, arg1 Light, arg2 Humidity;

// print to specific mediums
use Print, format "hello world!";
use Print, format "hello world!", out Serial;
use Print, format "hello world!", out Radio;

// print to specific mediums - without "Print"!
read Light, out Radio;
read Light, out Serial;
