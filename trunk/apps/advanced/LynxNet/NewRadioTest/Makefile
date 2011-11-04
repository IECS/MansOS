#SOURCES = sender.c
SOURCES = receiver.c test_stats.c

# Module is the name of the main module buit by this makefile
APPMOD = RadioTest

CFLAGS += -DCUSTOM_TIMER_INTERRUPT_HANDLERS=1
CFLAGS += -DDPRINT_TO_RADIO=1

#OPTIMIZE = no

# --------------------------------------------------------------------
# Set the key variables
PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = $(PROJDIR)/../../../..
endif

# Include the main makefile
include ${MOSROOT}/mos/make/Makefile
