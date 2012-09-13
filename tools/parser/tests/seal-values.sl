//
// Using SealValue objects in:
// 1) component defines
// 2) use cases
//

const PERIOD 5s;
define IntTemperature AnalogIn, channel variables.ADC_INTERNAL_TEMPERATURE;
define IntVoltage AnalogIn, channel variables.ADC_INTERNAL_VOLTAGE;
read IntTemperature, period PERIOD;
read IntVoltage, period PERIOD;

read AnalogIn, channel variables.jiffies; // nonsense, but should compile
