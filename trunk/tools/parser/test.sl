read Light;
output Serial;

// -- binary file
//read Light;
//output File, filename "lightData.bin", binary;

// ---------------------------------

// -- text file
// read Light;
// output File, filename "lightData.csv", text;

// ---------------------------------

// -- automatically generate name and type
// read Light;
// output File;

// ---------------------------------

// -- mutltiple files
// read Light;
// read Humidity;
// output File (Light, SequenceNumber);
// output File (Humidity, SequenceNumber);
