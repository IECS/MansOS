// blink red led periodically
use RedLed, period 1000ms;
// blink green led; faster at the start
when System.time < 5s:
    use GreenLed, period 100ms;
else:
    use GreenLed, period 1000ms;
// turn on blue led once the program has started
use BlueLed, on_time 2000ms;
