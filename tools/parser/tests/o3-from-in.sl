define MyInput DigitalIn, port 1, pin 2;
define MyOutput DigitalOut, port 5, pin 4;
define MyLed BlueLed;

read MyInput, out MyOutput;
read MyInput, out MyLed;
