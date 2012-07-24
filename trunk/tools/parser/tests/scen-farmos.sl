set CalibrationOffset 0;

// calibrate temperature sensor specifically for each node
when Variables.localAddress == 1:
    set CalibrationOffset 3;
elsewhen Variables.localAddress == 2:
    set CalibrationOffset -2;
elsewhen Variables.localAddress == 3:
    set CalibrationOffset 1;
end

define CalibratedLight add(Light, CalibrationOffset);
// read & send out the calibrated sensor
read CalibratedLight;
output Network, protocol CSMA;
