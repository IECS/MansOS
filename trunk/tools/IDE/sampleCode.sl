use RedLed, period 1s; // test

// comment 1
// comment 2
// comment 3
read Light, period 2s; // comment 3

read Humidity, period 2s;
// Comment 1
// Comment 2
when true: // Comment 3
	// Comment 4
	// Comment 5
	use blackLed; // Comment 6
// Comment 7
// Comment 8
else: // Comment 8.1
	// Comment 8.2
	// Comment 8.3
	use GreenLed; // Comment 8.4
	// Comment 9
	// Comment 10
	when true: // Comment 11
		// Comment 12
		// Comment 13
		use blackLed; // Comment 14
	// Comment 15
	// Comment 16
	else: // Comment 17
		// Comment 18
		// Comment 19
		use GreenLed; // Comment 20
	// Comment 21
	// Comment 22
	end // Comment 23
// Comment 24
// Comment 25
end // Comment 26
