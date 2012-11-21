// user button on Tmote Sky
define MyPin DigitalIn, port 2, pin 7;
read MyPin;

// if status changed during last second, print a message
when changed(MyPin, 1s):
   use print, format "user button status changed!\n";
end
