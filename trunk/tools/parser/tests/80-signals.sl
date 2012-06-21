read Constant;             // default constant value 0
read Constant, value 123;  // specific constant value 123

read Random;
read Random, min 10, max 100;

// "wavePeriod", because "period" is already used for sampling period
read SquareWave, low 10, high 100, wavePeriod 1600;

read TriangleWave, low 10, high 100, wavePeriod 1600;

read SawtoothWave, low 10, high 100, wavePeriod 1600;

read SineWave, low 10, high 100, wavePeriod 1500;
