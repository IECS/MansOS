PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = $(CURDIR)/../..
endif

all: pc-cloud

run: pc-cloud
	make all && ./pc-cloud

pc-cloud: proxy.o
	g++ -o pc-cloud proxy.o -lpthread

proxy.o: main.cpp
	g++ -o proxy.o -c main.cpp -I $(MOSROOT)/mos/mod/net \
		-I $(MOSROOT)/mos/hal/platforms/pc -I $(MOSROOT)/mos/hil \
		-I $(MOSROOT)/mos/kernel \
		-I $(MOSROOT)/mos \
		-D__bool_defined
	
clean:
	rm -rf proxy.o pc-cloud
