// return minimum value from the random numbers so far
define MinRandom1 min(Random);
read MinRandom1;

// return minimum value from three executions of random number generator
define MinRandom2 min(Random, Random, Random);
read MinRandom2;

// return minimum from last 10 random number generations
define MinRandom3 min(take(Random, 10));
read MinRandom3;
