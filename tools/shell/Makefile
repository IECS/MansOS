TARGET = shell

MOSROOT=../../mos

CFLAGS += -W -Wall -g -DDEBUG -I$(MOSROOT) -I$(MOSROOT)/hal/platforms/telosb -I$(MOSROOT)/hpl/chips

SOURCES = main.cpp $(MOSROOT)/smp/codec.c ihex.cpp

all: 
	$(CXX) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
