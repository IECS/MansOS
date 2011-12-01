read Light, period 2s;
read Humidity, period 2s;

sendto Serial {light, humidity, row 3, seat 4}, aggregate;

sendto Radio {light, humidity, row 3, seat 4}, aggregate;
