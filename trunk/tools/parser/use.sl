// -------------------------------------
// resolved at runtime
// -------------------------------------

// basic example: print some text depending whether the red LED is on or off
when RedLed:
    use Print, format "RED led is on!\n";
else:
    use Print, format "RED led is off!\n";
end

// TODO: allow to use values in volts, lux etc.!
when InternalVoltage.value < 0x3ffe:
    use Print, format "low battery!\n";
end

// same as above, because "value" is default
when InternalVoltage < 0x3ffe:
     use Print, format "low battery!\n";
end

// "localAddress" is not a declared component, so the best guess is to assume it's in the C code...
when localAddress == 0xdead:
    use Print, once, format "your local address is DEAD ;)\n";
end

// "isError" is a state variable that must be provided by the component;
// however, the detection is tricky, so currenlty only digital sensors use it!
when Humidity.isError:
    use Print, format "humidity sensor error!\n";
else:
    use Print, format "humidity sensor OK!\n;"
end

// -------------------------------------
// resolved at compile time
// -------------------------------------

// not very important and relatively hard to make - really needed?
// (this code must be resolved in compile time for the best efficiency!)
// first approximaltion - true iff the target platform has the component AND it has at least one use case
when Humidity.isPresent:
    use Print, once, format "humidity sensor driver is present!\n";
else:
    use Print, once, format "humidity sensor driver is not present!\n";
end

// "config" is 'other' kind of component - i.e. not sensors, act., or output;
// not usable as such, just in conditions
when Config.reprogramming:
    use Print, once, format "reprogramming code included!\n";
else:
    use Print, once, format "reprogramming code excluded!\n";
end

// "config" is same kind of component - 
// reserved for all target platform constants that are exported to SEAL
when Constants.OsName == "MansOS":
    use Print, once, format "Using MansOS!\n";
else:
    // OS_NAME is a constant that must be defined in C code
    use Print, once, format "Using OS %s!\n", arg1 OS_NAME;
end

