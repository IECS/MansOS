// no real supprt for constants after all... but they can be emulated using variables
set CONST_FOO 13s;
when SystemTime < CONST_FOO:
    use RedLed;
end
