// 1.
// define LightAlias Light;
// use LightAlias;

// define Virtual5 5;
// use Virtual5;

// 2.
// define VirtualBar Bar;
// define VirtualVirtualBar VirtualBar;
// use VirtualVirtualBar;

// 3
define Constant13 Constant, value 13;
use Constant13;

// 4
// define Random100 Random, max 100;
// define Random90to100 Random100, min 90;
// use Random90to100;

// 5
// define VirtualSlow SlowRead1, period 500;
// use VirtualSlow;

// read SlowRead1;
// read SlowRead2;
// read SlowRead3;
// read FastRead;

// define Constant5 Constant, value 5;
//define Foo, bar;



// config "USE_ADDRESSING=y";
// config "USE_SPI=y";
// config "DEBUG=y";

//use Light;

// when SlowRead1 > 100:
//     use RedLed;
// end

// return minimum value from the random numbers so far
// define MinRandom min(Random);
// read MinRandom;
