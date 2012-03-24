set temperatureCritical false;
when Sensors.Temperature > 50:
    set temperatureCritical true;
end
when Sensors.Temperature < 40:
    set temperatureCritical false;
end
when temperatureCritical:
    use RedLed, period 100ms;
end
