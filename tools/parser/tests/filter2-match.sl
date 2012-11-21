//
// Match input value as pattern
//
pattern FooPattern (1, 1, 1, 0);
define MyInput DigitalIn, port 2, pin 0;

read MyInput;
when match(invert(MyInput), FooPattern):
    use Print, format "matched!\n";
else:
    use Print, format "not matched!\n";
end
