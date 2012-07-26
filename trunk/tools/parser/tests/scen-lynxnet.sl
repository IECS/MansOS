//
// LynxNet emergency controller
//

const RESET_PORT 1; const RESET_PIN  1;
const POWER_PORT 1; const POWER_PIN  2;
const BATTERY_LOW_PORT 1; const BATTERY_LOW_PIN 3;
const WATCHDOG_PORT 1; const WATCHDOG_PIN 4;

// The logic is simple
define ResetOut DigitalOut, port RESET_PORT, pin RESET_PIN;
define PowerOut DigitalOut, port POWER_PORT, pin POWER_PIN;
define BatteryLow DigitalIn, port BATTERY_LOW_PORT, pin BATTERY_LOW_PIN;
define Watchdog DigitalIn, port WATCHDOG_PORT, pin WATCHDOG_PIN, interrupt, risingEdge;

set IsMainControllerOff false;

// when battery of the main controller is low, turn it off,
// but turn it back on after a while (however, lit the number of tries)
set NextRetryTime 0;
set NumBatteryRetry 0;
const MAX_RETRY 5;
const RETRY_INTERVAL 36000; // 10 hours in seconds
when BatteryLow:
    use PowerOut, off;
    set NextRetryTime add(Uptime, RETRY_INTERVAL);
    set IsMainControllerOff true;
    set NumBatteryRetry plus(NumBatteryRetry, 1);
end
when IsMainControllerOff and NumBatteryRetry < MAX_RETRY and NextRetryTime < Uptime:
    use PowerOut, on;
    set IsMainControllerOff false;
end

// check for watchdog signal, and if none received
const WATCHDOG_MAX_TIME 20; // seconds
set NextWatchdogTime WATCHDOG_MAX_TIME;
set NumWatchdogRetry 0;
when Watchdog:
    // watchdog interrupt fired
    set NextWatchdogTime add(Uptime, WATCHDOG_MAX_TIME);
    set NumWatchdogRetry 0;
    set IsMainControllerOff false;
end
when NextWatchdogTime < Uptime:
    // watchdog interrupt missed; try to reset the main MCU
    use ResetOut, once;
    set NextWatchdogTime add(Uptime, WATCHDOG_MAX_TIME);
    set NumWatchdogRetry plus(NumWatchdogRetry, 1);
end
when NumWatchdogRetry >= MAX_RETRY:
    // too bad; give up and turn the main MCU off
    set IsMainControllerOff true;
    use PowerOut, off;
end

// send out the emergency signal (more often if main MCU is dead)
const SHORT_SIGNAL_PERIOD 800ms;
const NORMAL_SIGNAL_PERIOD 1600ms;
when IsMainControllerOff:
    use CarrierSignalGenerator, period SHORT_SIGNAL_PERIOD;
else:
    use CarrierSignalGenerator, period NORMAL_SIGNAL_PERIOD;
end
