// no real supprt for constants after all... but they can be emulated using variables
// set CONST_FOO 13s;
// when SystemTime < CONST_FOO:
//     use RedLed;
// end

const CONST_VALUE 13;

use Constant, value CONST_VALUE;

define ConstSum sum(CONST_VALUE, 1);
read ConstSum;

when CONST_VALUE < 14:
    use RedLed;
end
