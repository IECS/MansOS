// On sensor node:
// const READ_PERIOD 10s;
// read Light, period READ_PERIOD;
// output Network (Light, Address, Timestamp), protocol LiteTDMA;


// On data storage node
const DATA_QUERY_COMMAND 1;

NetworkRead RemoteLightPacket(Light, Address, Timestamp);
Output File (RemoteLightPacket), filename "LightData.bin";

NetworkRead StorageQueryPacket(Command, Address, Timestamp);
when StorageQueryPacket.command = DATA_QUERY_COMMAND:
    Output Network, protocol LiteTDMA, file "LightData.bin",
        where 
            StorageQueryPacket.Address = Address
            and StorageQueryPacket.Timestamp <= TimeStamp;
end
