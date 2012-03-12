// comment 1
// comment 2
// comment 3
read Light, period 2s; // comment 4

read Humidity, period 2s;

// Comment 1
// Comment 2
when true: // Comment 3
	// Comment 4
	// Comment 5
	use BlueLed; // Comment 6
// Comment 7
// Comment 8
else: // Comment 9
	// Comment 10
	// Comment 11
	use GreenLed; // Comment 12
	// Comment 13
	// Comment 14
	when true: // Comment 15
		// Comment 16
		// Comment 17
		use Led; // Comment 18
	// Comment 19
	// Comment 20
	else: // Comment 21
		// Comment 22
		// Comment 23
		use GreenLed; // Comment 24
	// Comment 25
	// Comment 26
	end // Comment 27
// Comment 28
// Comment 29
end // Comment 30

