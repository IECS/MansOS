//
// "lazy" reading. The function is evaluated onluy oncve per period.
//
read avg(take(random, 10)), lazy;

define LazyRandom avg(take(random, 10)), lazy;
read LazyRandom;
