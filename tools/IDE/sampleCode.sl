use RedLed, period 2s;

read Light, period 100ms, average 10;

read Humidity, period 2s;

output Serial, period 60s, crc, aggregate;

when time < 5s:
    use GreenLed;
elsewhen isDayTime:
    use RedLed;
else:
    use BlueLed;
end
