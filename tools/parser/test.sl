// do something number of times,
// in this case, toggle a LED 10 times
// use Led, times 10;

// define Foo Abs(Random);
// when Foo:
//     use print, format "pin status changed!\n";
// end;


// // user button on Tmote Sky
// define MyPin DigitalIn, port 2, pin 7;

// // if status changed during last second, print a message
// when changed(MyPin, 1s):
//    use print, format "user button status changed!\n";
// end


define Accel1 AnalogIn, channel 1;
define Accel2 AnalogIn, channel 2;

define Blurred smoothen(Accel1);
define Contrasted sharpen(Accel2, 5, 5);

read Blurred; read Contrasted;
