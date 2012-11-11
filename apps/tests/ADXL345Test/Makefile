#-*-Makefile-*- vim:syntax=make
# /*
#  * "Copyright (c) 2008  Leo Selavo and the contributors."
#  * Permission to use and disclaimer are defined in /USE_AND_DISCLAIMER.txt 
#  */
#
# --------------------------------------------------------------------
#	Makefile for the sample application
#
#  The developer must define at least SOURCES and APPMOD in this file
#
#  In addition, PROJDIR and MOSROOT must be defined, before including 
#  the main Makefile at ${MOSROOT}/mos/make/Makefile
# --------------------------------------------------------------------

# Sources are all project source files, excluding MansOS files
SOURCES = main.c

# Module is the name of the main module buit by this makefile
APPMOD = ADXL345Test

# --------------------------------------------------------------------
# Set the key variables
PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = $(PROJDIR)/../../..
endif

# Include the main makefile
include ${MOSROOT}/mos/make/Makefile
