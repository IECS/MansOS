##############################
# constants & variables      #
##############################

TARGET = sdtest

MOTELIST = motelist
BSL = ../../../mos/make/scripts/bsl.py
BSLPORT ?= "$(shell $(MOTELIST) -c | xargs echo | cut -f2 -d, )"

##############################
# platform specific stuff    #
##############################

CC = msp430-gcc
MODEL = msp430x1611
CFLAGS += -mmcu=$(MODEL) -g -Wall -W -DPLATFORM_TELOSB=1
LDFLAGS += 

##############################
# targets                    #
##############################

# the default target
DEMO=sdtest

.PHONY: all sdtest upload clean

all: upload

sdtest: OBJECTS := main.c sdcard_hal.c mmc.c
sdtest: DEMO := sdtest
sdtest: ihex

ihex: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $(TARGET).elf
	msp430-objcopy -O ihex $(TARGET).elf $(TARGET).ihex

upload: $(DEMO)
	$(BSL) --invert-reset --invert-test -c $(BSLPORT) -r -e -I -p  $(TARGET).ihex

clean:
	rm -f $(TARGET).ihex $(TARGET).elf *.o
