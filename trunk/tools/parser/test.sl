// const CONST_VALUE 13;

// use Constant, value CONST_VALUE;

// define ConstSum sum(CONST_VALUE, 1);
// read ConstSum;

// when CONST_VALUE < 14:
//     use RedLed;
// end

//define AvgRandom Random;

// define AvgRandom avg(Random);
// define StdevRandom stdev(Random);

// read AvgRandom;
// read StdevRandom;

//define LimitedRandom Random, max 10;
// define LimitedRandom Random;
// define StdevRandom stdev(LimitedRandom);
// read StdevRandom;

// define StdevRandom stdev(take(Random, 2));
// read StdevRandom;


// 9 = ((((5 - 3) * 10) % 7) / 2) ^ 2
//define Arithmetic power(divide(modulo(multiply(minus(5, 3), 10), 7), 2), 2);
//read Arithmetic;

// define FilteredRandom filterLess(filterMore(Random, 10000), 20000);
// read FilteredRandom;

// define FilteredRandom min(filterMore(Random, 10000));
// read FilteredRandom;

// define AvgRandom avg(filterLess(Random, 32000));
// read AvgRandom;

define MinRandom min(Random, Random);
read MinRandom;
output serial;
