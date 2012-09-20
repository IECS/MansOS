// react on a command
const COMMAND_PRINT 17;
NetworkRead RemoteCommand (Command);
when RemoteCommand.Command == COMMAND_PRINT:
    use Print, format "hello world\n", once;
end;
