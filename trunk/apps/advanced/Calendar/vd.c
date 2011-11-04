#include <stdmansos.h>
#include <dprint.h>

//#define PROGMEM

#include "vd_names.h"

typedef unsigned char uint8_t;

// we will use all the names as one string separated by spaces
struct month_t {
    uint8_t daycount;
    const char *names;
};

enum {
	MONTH_COUNT = 12
};

const struct month_t NAMES[MONTH_COUNT] = {
    { 31, (char *) MONTH_1 },
    { 29, (char *) MONTH_2 },
    { 31, (char *) MONTH_3 },
    { 30, (char *) MONTH_4 },
    { 31, (char *) MONTH_5 },
    { 30, (char *) MONTH_6 },
    { 31, (char *) MONTH_7 },
    { 31, (char *) MONTH_8 },
    { 30, (char *) MONTH_9 },
    { 31, (char *) MONTH_10 },
    { 30, (char *) MONTH_11 },
    { 31, (char *) MONTH_12 }
};

void appMain() {
	uint8_t i;
	// go through all months
	for (i = 0; i < MONTH_COUNT; ++i) {
		PRINTF("%i. menesis:\n", (i + 1));
		const char *nameStart = NAMES[i].names;
		uint8_t j;
		// go through all days
		for (j = 0; j < NAMES[i].daycount; ++j) {
	        PRINTF("%i: ", (j + 1));
	        const char *nameEnd = nameStart;
		    // search for end-of-day in month name-day array
		    while (*nameEnd != 0) {
		        ++nameEnd;
		    }
		    PRINTF("%s\n", nameStart);
		    nameStart = nameEnd + 1;
		}
	}
	
	
    return 0;
}
