use RedLed, period 2s; 

read Light, period 2s; 

read Humidity, period 2s; 

when System.isDaytime: 
	use RedLed;
else:
	use BlueLed;
end
