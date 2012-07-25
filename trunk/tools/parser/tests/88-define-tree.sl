define CombinedSensor sum(Light, Humidity);
define MoreSensor abs(CombinedSensor);
read MoreSensor;
