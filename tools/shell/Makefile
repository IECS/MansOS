TARGET = shell

MOSROOT=../../mos

# TODO: platform (msp430) specific
ARCH=msp430
PLATFORM=telosb
CPU_MHZ=4

CFLAGS += -W -Wall -g -DDEBUG -I$(MOSROOT) -I$(MOSROOT)/arch/$(ARCH) -I$(MOSROOT)/platforms/$(PLATFORM) -I$(MOSROOT)/chips -DCPU_MHZ=$(CPU_MHZ)

SOURCES = main.cpp $(MOSROOT)/smp/codec.c ihex.cpp

all: 
	$(CXX) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
