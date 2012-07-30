//
// Multiple defines with the same name (dependent on conditions)
//

when 1 < 2:
    define DemoSensor Light;
else:
    define DemoSensor Humidity;
end
read DemoSensor;
