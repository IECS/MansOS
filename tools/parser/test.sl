// do something number of times,
// in this case, toggle a LED 10 times
// use Led, times 10;

// define Foo Abs(Random);
// when Foo:
//     use print, format "pin status changed!\n";
// end;


// // user button on Tmote Sky
// define MyPin DigitalIn, port 2, pin 7;

// // if status changed during last second, print a message
// when changed(MyPin, 1s):
//    use print, format "user button status changed!\n";
// end


// define Accel1 AnalogIn, channel 1;
// define Accel2 AnalogIn, channel 2;

// define Blurred smoothen(Accel1);
// define Contrasted sharpen(Accel2, 5, 5);

// read Blurred; read Contrasted;


// define a pattern using microsecond intervals
// pattern P (300us, 100us, 300us, 400us, 300us, 200us);
// // then output this pattern to the specific pin
// use DigitalOut, port 1, pin 6, pattern P;



// On sensor node:
// const READ_PERIOD 10s;
// read Light, period READ_PERIOD;
// output Network (Light, Address, Timestamp), protocol LiteTDMA;


// On data storage node
const DATA_QUERY_COMMAND 1;

NetworkRead RemoteLightPacket(Light, Address, Timestamp);
output File (RemoteLightPacket), filename "LightData.bin";

NetworkRead StorageQueryPacket(Command, Address, Timestamp);
when StorageQueryPacket.command = DATA_QUERY_COMMAND:
   output Network, protocol LiteTDMA, file "LightData.bin",
       where 
           StorageQueryPacket.Address = Address
           and StorageQueryPacket.Timestamp <= TimeStamp;
end



//NetworkRead Light;
//output Serial(Light);


// read constant, value 3;
// output radio;

// [untested!] sending a command to radio (XXX: what about radio packet type?)
// define MyCommand Command, value 1;
// read MyCommand;
// output radio(MyCommand);


// read random values from a neighbor, send to serial
// read Random;
// output Radio (Random);
// NetworkRead NetworkRandom (Random);
// output Serial (NetworkRandom);


// NetworkRead NetworkCommand (Command);
// when NetworkCommand.Command == 1:
//    use RedLed;
// end


// NetworkRead NeighborLightPacket(Light, Address, Timestamp);
// output File (NeighborLightPacket), filename "LightData.bin";
