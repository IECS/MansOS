TARGET ?= telosb

all:
	(cd 0-Lockup; $(MAKE) $(TARGET))
	(cd 0-Overflow-User; $(MAKE) $(TARGET))
	(cd 0-Overflow-Kernel; $(MAKE) $(TARGET))
	(cd 1a-Blink-mdelay; $(MAKE) $(TARGET))
	(cd 1b-Blink-msleep; $(MAKE) $(TARGET))
	(cd 1c-Blink-Multi; $(MAKE) $(TARGET))
	(cd 2-Alarms; $(MAKE) $(TARGET))
	(cd 3-Locking; $(MAKE) $(TARGET))
	(cd 3-Locking-Multi; $(MAKE) $(TARGET))
	(cd 4-Exit; $(MAKE) $(TARGET))
	(cd 5-Radio; $(MAKE) $(TARGET))
	(cd 6-MAC; $(MAKE) $(TARGET))
	(cd 7-Sockets; $(MAKE) $(TARGET))


clean:
	(cd 0-Lockup; $(MAKE) clean)
	(cd 0-Overflow-User; $(MAKE) clean)
	(cd 0-Overflow-Kernel; $(MAKE) clean)
	(cd 1a-Blink-mdelay; $(MAKE) clean)
	(cd 1b-Blink-msleep; $(MAKE) clean)
	(cd 1c-Blink-Multi; $(MAKE) clean)
	(cd 2-Alarms; $(MAKE) clean)
	(cd 3-Locking; $(MAKE) clean)
	(cd 3-Locking-Multi; $(MAKE) clean)
	(cd 4-Exit; $(MAKE) clean)
	(cd 5-Radio; $(MAKE) clean)
	(cd 6-MAC; $(MAKE) clean)
	(cd 7-Sockets; $(MAKE) clean)
