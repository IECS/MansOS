
// blink LEDs periodically
pattern Led0P (200, 200, 200, 200, 200, 800, 200, 200, 800, 500);
pattern Led1P (200, 200, 200, 200, 200, 600, 400, 400, 600, 500);
pattern Led2P (200, 200, 200, 200, 200, 400, 600, 600, 400, 500);
pattern Led3P (200, 200, 200, 200, 200, 200, 800, 800, 200, 500);

use Led, id 0, pattern Led0P;
use Led, id 1, pattern Led1P;
use Led, id 2, pattern Led2P;
use Led, id 3, pattern Led3P;

read ADS8638, channel 7;
output Serial;
output SdCard;
