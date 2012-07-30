// define HumRedLed RedLed, blink;

// use Humidity, associate HumRedLed, turnOnOff;

// pattern FooPattern (1, 1, 1, 0);
// define MyInput DigitalIn, port 2, pin 0;

// when match(invert(MyInput), FooPattern):
//     use Print, format "matched!\n";
// else:
//     use Print, format "not matched!\n";
// end

// when invertFilter(filterEqual(MyInput, 1)):
//    use Print, format "is zero!\n";
// else:
//    use Print, format "is one!\n";
// end


//define HumRedLed RedLed, blink;

define MyInput DigitalIn, port 1, pin 2;
define MyOutput DigitalOut, port 5, pin 4;
define MyLed BlueLed;

read MyInput, out MyOutput;
read MyInput, out MyLed;
