// normal
read sharpen(Random);
// two args
read sharpen(Random, 10);
// three args
read sharpen(Random, 10, 1);

// second arg is named
read sharpen(Random, numSamples 10);

// third arg is named
read sharpen(Random, weight 2);

// all args are named
read sharpen(Random, numSamples 10, weight 3);

// floating point number!
read sharpen(Random, numSamples 10, weight 1.2);

// this is invalid: must be constant
//read sharpen(Random, Random);

// this is invalid: too many args
//read sharpen(1, 2, 3, 4);
