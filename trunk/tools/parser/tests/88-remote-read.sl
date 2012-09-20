// read random values from a neighbor, send to serial
read Random;
output Radio (Random);
NetworkRead RemoteRandom (Random);
output Serial (RemoteRandom);
