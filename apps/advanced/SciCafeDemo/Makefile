#-*-Makefile-*- vim:syntax=make
#
# --------------------------------------------------------------------
#  The developer must define at least SOURCES and APPMOD in this file
#
#  In addition, PROJDIR and MOSROOT must be defined, before including 
#  the main Makefile at ${MOSROOT}/mos/make/Makefile
# --------------------------------------------------------------------

# Sources are all project source files, excluding MansOS files
SEAL_SOURCES = main.sl

# Module is the name of the main module built by this makefile
APPMOD = SealApp

# --------------------------------------------------------------------
# Set the key variables
PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = ../../..
endif

# Include the main makefile
include ${MOSROOT}/mos/make/Makefile
