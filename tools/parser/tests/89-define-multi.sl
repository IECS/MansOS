//
// Multiple defines with the name name (dependent on conditions)
//

when 1 < 2:
    define DemoSensor Light;
else:
    define DemoSensor Humidity;
end
read DemoSensor;
