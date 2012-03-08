define StartupSequence, period 1s, on_time 1500ms, off_time 4500ms;
use RedLed, parameters StartupSequence;
use GreenLed, parameters StartupSequence;

define SerialParams, baudrate 38400, aggregate;
sendto Serial, parameters SerialParams;
