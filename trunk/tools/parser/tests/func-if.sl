define Foo1 If(1, Light, Humidity); read Foo1;
define Foo2 If(1, Light); read Foo2;
define Foo3 If(1, Light, 2); read Foo3;
define Foo4 If(1, 2, 3); read Foo4;
define Foo5 If(Light, 2, 3); read Foo5;

define Foo6 If(True, 2, 3); read Foo6;

// this imitates "true" conditions (not supported byt the parser...)
set Foo7Condition False;
when 1 < 2: set Foo7Condition True; end;
define Foo7 If(Foo7Condition, 2, 3); read Foo7;
