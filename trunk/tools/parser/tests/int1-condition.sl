pattern OnPattern (1, 1, 1, 1, 1, 1, 1, 1);
pattern OffPattern (0, 0, 0, 0, 0, 0, 0, 0);

define MyInput DigitalIn, port 2, pin 4, interrupt, risingEdge, interruptPort 2, interruptPin 0;
define MyOutput DigitalOut, port 2, pin 5;

use MyOutput, on; // by default on

when match(MyInput, OffPattern):
   // turn off
   use MyOutput, off;
end
when match(MyInput, OnPattern):
   // turn on
   use MyOutput, on;
end
