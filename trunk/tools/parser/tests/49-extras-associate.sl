// associate humidity sensor reading with LED blink
define HumRedLed RedLed, blink;
use Humidity, associate HumRedLed, turnOnOff, period 10s;
