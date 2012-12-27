TARGET ?= telosb

all:
	(cd 01-Format; $(MAKE) $(TARGET))
	(cd 02-Open; $(MAKE) $(TARGET))
	(cd 03-ReadWrite; $(MAKE) $(TARGET))
	(cd 04-CreateRemove; $(MAKE) $(TARGET))
	(cd 05-LongReadWrite; $(MAKE) $(TARGET))
	(cd 06-FilePrint; $(MAKE) $(TARGET))
	(cd 07-FileSeek; $(MAKE) $(TARGET))

clean:
	(cd 01-Format; $(MAKE) clean)
	(cd 02-Open; $(MAKE) clean)
	(cd 03-ReadWrite; $(MAKE) clean)
	(cd 04-CreateRemove; $(MAKE) clean)
	(cd 05-LongReadWrite; $(MAKE) clean)
	(cd 06-FilePrint; $(MAKE) clean)
	(cd 07-FileSeek; $(MAKE) clean)
