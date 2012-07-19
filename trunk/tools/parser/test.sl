//
// input syntax
//

const COMMAND_GET 17;

input CommandWithSeqnum (command, seqnum);

when CommandWithId.command = COMMAND_GET:
//    use print, format "%d\n", arg1 CommandWithSeqnum.seqnum;
    use print, format "hello world\n";
end
