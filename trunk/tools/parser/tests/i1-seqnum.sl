// react on a command ("InputCommand" syntax)

const COMMAND_GET 17;

InputCommand CommandWithSeqnum (Command, SequenceNumber);

when CommandWithSeqnum.Command = COMMAND_GET:
   use print, format "got seqnum %d\n", arg1 CommandWithSeqnum.SequenceNumber, once;
end
