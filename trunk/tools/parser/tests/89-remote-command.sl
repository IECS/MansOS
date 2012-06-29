// react on a command
const COMMAND_PRINT 17;
when RemoteCommand == COMMAND_PRINT:
    use Print, format "hello world\n", once;
end;
