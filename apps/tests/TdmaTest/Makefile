#-*-Makefile-*- vim:syntax=make

SOURCES = main.c

APPMOD = TdmaTest

# compilation parameters
IS_BASE_STATION ?= 1
IS_MASTER ?= 0
SLAVE_NUMBER ?= 0

CFLAGS += \
	-DCFG_MAC_TDMA_IS_BASE_STATION=$(IS_BASE_STATION) \
	-DCFG_MAC_TDMA_IS_MASTER=$(IS_MASTER) \
	-DCFG_MAC_TDMA_SLAVE_NUMBER=$(SLAVE_NUMBER)

PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = $(PROJDIR)/../../..
endif

include ${MOSROOT}/mos/make/Makefile
