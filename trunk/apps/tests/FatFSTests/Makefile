TARGET ?= telosb

all:
	(cd 01-Format; $(MAKE) $(TARGET))
	(cd 02-Open; $(MAKE) $(TARGET))
	(cd 03-ReadWrite; $(MAKE) $(TARGET))
	(cd 04-Remove; $(MAKE) $(TARGET))


clean:
	(cd 01-Format; $(MAKE) clean)
	(cd 02-Open; $(MAKE) clean)
	(cd 03-ReadWrite; $(MAKE) clean)
	(cd 04-Remove; $(MAKE) clean)