const SAMPLING_INTERVAL 5s;
const INTERIOR_REPORT_INTERVAL 5min;
const ENTRANCE_REPORT_INTERVAL 30s;
const SATURATED_VALUE 0xffff;

// Load C-based extensions for this scenario
load "TunnelDataCollector.py"; // component description
load "TunnelDataCollector.c";  // component implementatio

// choose the sampling interval depending on system variables
// (assume that 'moteIsInInterior' is a variable
//  that can be changed via management protocol)
when Variables.moteIsInInterior:
    set reportInterval INTERIOR_REPORT_INTERVAL; // TODO: convert to ms!
else:
    set reportInterval ENTRANCE_REPORT_INTERVAL; // TODO: convert to ms!
end

// there are 4 light sensors; ignore the ones that are saturated
define Light1 filterLess(LightWithId, SATURATED_VALUE), id 1;
define Light2 filterLess(LightWithId, SATURATED_VALUE), id 2;
define Light3 filterLess(LightWithId, SATURATED_VALUE), id 3;
define Light4 filterLess(LightWithId, SATURATED_VALUE), id 4;
// one measurement is an average of all valid light sensors
define LightAvg average(tuple(Light1, Light2, Light3, Light4));
// read the average value with 5 second period
read LightAvg, period SAMPLING_INTERVAL; // TODO: convert to ms!
// use a custom data collector component
output TunnelDataCollector, interval reportInterval;
