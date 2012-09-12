// -- on sensor mote
// read soil humidity sensor
// use SoilHumidity;
// send the measurements to radio
// output Radio;

// -- on actuator device
const MIN_HUMIDITY_THRESHOLD 0xBCF;

// take soil humidity out of network
NetworkRead NeighborSoilHumidity(SoilHumidity);

// define the minimal humidity as minimum of values
// in last 60 seconds, max 20 value aray
define MinHumidity min(take(NeighborSoilHumidity, 20, 60s));

when MinHumidity < MIN_HUMIDITY_THRESHOLD:
    // use watering system in this case
    use Watering;
end;
