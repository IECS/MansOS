// read Uptime, period 2;
// output Serial (Uptime, SequenceNumber);
// output Serial, SequenceNumber;
// output Serial, SequenceNumber True;


// read Counter;
// read Counter, start 5, max 10;
// output Serial;

//load "UncommonExtension.py";
//read UncommonSensor;
//output Serial;

load "CLangExtension"; // python file
load "ext.c"; // C file
read CLangExtSensor;
output Serial;
