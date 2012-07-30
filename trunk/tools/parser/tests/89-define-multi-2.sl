when 1 < 2:
    define DemoSensor Light;
else:
    define DemoSensor Humidity;
end
define TheSensor DemoSensor;
read TheSensor;
