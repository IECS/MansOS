// shows getting a specific block of records from file,
// in this case: unsent data

const COMMAND_FLUSH_UNSENT 17;

read Light;
output File (Light, SequenceNumber, IsSent), filename "LightData.csv";

when RemoteCommand == COMMAND_FLUSH_UNSENT:
    output Radio, file "LightData.csv", where IsSent = False;
end
