// shows getting a specific record from file

const COMMAND_GET 17;
const ID_TO_GET 3;

NetworkRead CommandWithSeqnum (Command, SequenceNumber);

read Light;
output File (Light, SequenceNumber), filename "LightData.csv";

when CommandWithSeqnum.command = COMMAND_GET:
     output Serial, file "LightData.csv", where CommandWithSeqnum.SequenceNumber = ID_TO_GET;
end
