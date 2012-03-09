use RedLed, period 1s; // test

// comment 1
// comment 2
// comment 3
read Light, period 2s; // comment 3

read Humidity, period 2s;
// Comment
// Comment
when true: // Comment
	// Comment
	// Comment
	use blackLed; // Comment
// Comment
// Comment
else: // Comment
	// Comment// Comment
	// Comment
	use GreenLed;
	// Comment
	// Comment
	when true: // Comment
		// Comment
		// Comment
		use blackLed; // Comment
	// Comment
	// Comment
	else: // Comment
		// Comment
		// Comment
		use GreenLed; // Comment
	end // Comment
end // Comment