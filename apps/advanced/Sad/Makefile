
.PHONY: all bs mote pc

all: bs mote pc

bs:
	$(MAKE) -C $(<D) bs telosb

mote:
	$(MAKE) -C $(<D) mote telosb

pc:
	$(MAKE) -C $(<D) pc

clean:
	-@find . -type d -name build -exec rm -rf \{\} \; 
