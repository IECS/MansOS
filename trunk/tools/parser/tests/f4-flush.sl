// shows flushing a file

const COMMAND_FLUSH 17;

read Light;
output File (Light, SequenceNumber), filename "LightData.csv";

NetworkRead RemoteCommand (Command);
when RemoteCommand.Command == COMMAND_FLUSH:
    output Radio, file "LightData.csv";
end
