use RedLed, period 2s;

read Light, period 2s;

when isdaytime:
  use greenled;
elsewhen 1 < 22:
  use redled;
else:
  use blueled;
end

read Humidity, period 2s;