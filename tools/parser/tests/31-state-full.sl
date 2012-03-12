state temperatureCritical false;
when Sensors.Temperature > 50:
    set temperatureCritical true;
when Sensors.Temperature < 40:
    set temperatureCritical false;
when temperatureCritical:
    use RedLed, period 100ms;
