#-*-Makefile-*- vim:syntax=make
#
# Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# Makefile for all platforms, invoked from the user project makefile.
#
# The user project must define or verify the following variables:
#	MOSROOT = path/to/MansOS/root/directory/
#	PROJDIR = path/to/project/directory/
#	APPMOD  = software-application-or-module-name
#	SOURCES = list of project source files 
# 
# The target specifies the hardware platform and optional hw modules. 
# For example, 
#	make telosb 
# will create a telosb compatible application;
#	make telosb seepoga 
# will compile for telosb platform with seepoga LCD and sensors module.
#
# 
# Available actions:
#
# make <platform> [module]*
#	- makes for the specified platform and optional module(s)
#
# make <platform> [module]* run
#	- makes the application and executes it (e.g. on pc platform)
#
# make <platform> upload
#	- makes as needed and uploads the application or module
#
# Available platforms are defined by the other makefiles:
# 	Makefile.platforms 
#	platforms/Makefile.<platform>
#	modules/Makefile.<module>
#

#===== Directories =====
MOS = $(MOSROOT)/mos

OUTDIR = $(PROJDIR)/build
OBJDIR = $(PROJDIR)/build

#===== Files

MAKEFILE_PLATFORMS = $(MOS)/make/Makefile.platforms
MAKEFILE_CONFIG = $(MOS)/make/Makefile.config
MAKEFILE_OPTIONS = $(MOS)/make/Makefile.options
MAKEFILE_GCC = $(MOS)/make/Makefile.gcc
MAKEFILE_HIL = $(MOS)/make/Makefile.hil

#===== optimization
OPTIMIZE ?= yes

ifneq (,$(findstring $(OPT_OPTIMIZE), $(MAKECMDGOALS)))
	OPTIMIZE = yes
endif
ifeq (y,$(OPTIMIZE))
	OPTIMIZE = yes
endif
ifeq (y,$(OPTIMIZE_SIZE_EXTRA))
	OPTIMIZE_SIZE_EXTRA = yes
endif

#===== verbosity
VERBOSE ?= $(V)

ifneq (,$(findstring $(OPT_VERBOSE), $(MAKECMDGOALS)))
VERBOSE = 1
endif

ifneq (,$(findstring $(OPT_QUIET), $(MAKECMDGOALS)))
VERBOSE = 
endif

ifneq ($(VERBOSE),)
_QUIET =
_ECHOP = @true
else
_QUIET = @
_ECHOP = @echo
endif

#===== make tools =====
Print                  = $(_ECHOP) "  "
Copy                   = cp
MakeDir                = mkdir -p
Delete                 = rm -fr
List                   = cat

UNAME := $(shell uname)
SHORT_UNAME := $(shell uname | cut -b -6)


#===== Include all necessary and optional files

# user specific options
-include ~/.mansos.siteopts

# Declare targets and options
include $(MAKEFILE_OPTIONS)

# Detect the correct platfrom
include $(MAKEFILE_PLATFORMS)

# Include configuration specific options
include $(MAKEFILE_CONFIG)

# Select and include the makefiles according the target platform

ifneq ($(PLATFORM),)
  # dirty hack: this is needed for cygwin
  PROJDIR = .

  # setup output directories
  OUTDIR = $(PROJDIR)/build/$(PLATFORM)
  OBJDIR = $(PROJDIR)/build/$(PLATFORM)

  # Include platform specific Makefile
  include $(MOS)/make/platforms/Makefile.$(PLATFORM)

  ifeq ($(ARCH),)
  $(error "Selected platform $(PLATFORM) does not define MCU architecture!")
  endif

  # Include architecture specific Makefile
  include $(MOS)/make/arch/Makefile.$(ARCH)
endif

#===== Tools =====

SADLANG = $(MOSROOT)/tools/sadlang/sadlang

MEMDUMP = $(MOS)/make/scripts/memdump.py

# change this to your MSPsim installation directory
MSPSIMDIR = /opt/mspsim
JAVA = java
JAVA_CLASSPATH=$(MSPSIMDIR):$(MSPSIMDIR)/lib/jfreechart-1.0.11.jar:$(MSPSIMDIR)/lib/jcommon-1.0.14.jar
# java class used by MSPsim to simulate this platform
JAVA_CLASS=se.sics.mspsim.platform.sky.SkyNode

ifeq ($(UNAME),Darwin)
# apple wants special handling...
MOTELIST=$(MOS)/make/scripts/motelist.apple
else
ifeq ($(SHORT_UNAME),CYGWIN)
# cygwin too
MOTELIST=motelist
CFLAGS+=-DCYGWIN=1
else
# the normal case 
MOTELIST=$(MOS)/make/scripts/motelist
endif
endif

# Set up the correct programmer port
BSLPORT ?= "$(shell $(MOTELIST) -c | xargs -0 echo | xargs echo | cut -f2 -d, )"

#===== Source files =====

ifeq ($(USE_REPROGRAMMING),y)
# put user and system code in separate sections
LDFLAGS += "-Wl,--section-start=.text=0x5000,--section-start=.usertext=0xf000,--section-start=.start=0x4000,--defsym=_reset_vector__=0x4000"
endif

LINKER_SCRIPT = $(MOS)/make/scripts/link.py

# include the file where platform independent sources are defined
include $(MAKEFILE_HIL)

#===== Includes =====
INCLUDES += -I$(MOS)
INCLUDES += -I$(MOS)/kernel
INCLUDES += -I$(MOS)/lib
INCLUDES += -I$(NET)
INCLUDES += -I$(MOS)/hil
INCLUDES += -I$(MOS)/arch/$(ARCH)
INCLUDES += -I$(MOS)/platforms/$(PLATFORM)
INCLUDES += -I$(MOS)/chips

#=================== SADlang support
GENERATED_SOURCES = $(SADLANG_SOURCES:%.sl=%.c)

#===== Sorting out the Objs and Sources =====

PSOURCES += $(PSOURCES-y) $(PSOURCES-yes)

APP_OBJS = $(SOURCES:%.c= $(OBJDIR)/%.o)
APP_OBJS += $(GENERATED_SOURCES:%.c= $(OBJDIR)/%.o)

# add platform sources and objs
PSRC2 = $(subst $(MOSROOT),$(OUTDIR),$(PSOURCES))
POBJS = $(PSRC2:%.c= %.o)

OBJS = $(APP_OBJS) $(POBJS)
OBJDIRS = $(dir $(OBJS))



# ===== Targets =====
.PHONY: all $(PLATFORM_TARGET) build savetarget objDirs clean cleanall

all: build

$(PLATFORM_TARGET): build savetarget
	$(Print) ""

savetarget:
	$(_QUIET) rm -f $(PROJDIR)/build/Makefile.target
	$(_QUIET) @echo "saving Makefile.target"
	$(_QUIET) @echo >$(PROJDIR)/build/Makefile.target "TARGET = $(TARGET)"

build : $(OUTDIR)/config objDirs $(GENERATED_SOURCES) $(EXECUTABLE)

# This file is reuquired for building. An empty file is created it doesn't exist
$(PROJDIR)/config:
	touch $(PROJDIR)/config

# This logic is used to detect whether config file has changed and to rebuild all if it has
$(OUTDIR)/config: $(PROJDIR)/config
	$(_QUIET) $(Delete) $(OUTDIR)
	$(_QUIET) mkdir -p $(OUTDIR)
	$(_QUIET) cp $(PROJDIR)/config $(OUTDIR)/config

clean: 
	$(Print) "RM $(OUTDIR)"
	$(_QUIET) $(Delete) $(OUTDIR) *.stackdump extflash.dat eeprom

cleanall: clean
	$(_QUIET) $(Delete) $(GENERATED_SOURCES)

# ===== header file dependency tracking =====

_DEPS := $(subst .res,.d,$(subst .o,.d,$(OBJS)))

ifneq ("", "$(filter-out clean cleanall, $(MAKECMDGOALS))")
-include $(_DEPS)
endif

#===== Rules =====

# .c -> .o, system sources
$(OBJDIR)/%.o : $(MOSROOT)/%.c
	$(Print) "CC $<"
	$(_QUIET) $(CC) $(CFLAGS) $(INCLUDES) -MD -MF $(subst .o,.d,$@) -MP -MT $(subst .o,.d,$@) -MT $@ -c $< -o $@

%.c : %.sl
	$(Print) "SADLANG $<"
	$(_QUIET) $(SADLANG) $< -o $@

# .c -> .o, user sources
$(OBJDIR)/%.o : $(PROJDIR)/%.c
	$(Print) "CC $<"
	$(_QUIET) $(CC) $(CFLAGS) $(INCLUDES) -MD -MF $(subst .o,.d,$@) -MP -MT $(subst .o,.d,$@) -MT $@ -c $< -o $@
ifeq ($(USE_REPROGRAMMING),y)
# rename ".text" section to ".usertext"
	$(_QUIET) $(OBJCOPY) --rename-section .text=.usertext $@
endif


# .o -> .elf
# TODO - what to do for msp430?
$(OUTDIR)/$(APPMOD).elf : $(OBJS)
	$(Print) "LD $@"
	$(_QUIET) $(LINKER_SCRIPT) $(ARCH) $@ "$(CFLAGS) $(LDFLAGS)" "$(APP_OBJS)" "$(POBJS)"

# .o -> .exe
$(OUTDIR)/$(APPMOD).exe : $(OBJS)
	$(Print) "LD $@"
	$(_QUIET) $(LINKER_SCRIPT) "pc" $@ "$(CFLAGS) $(LDFLAGS)" "$(APP_OBJS)" "$(POBJS)"


# .c -> .rel
$(OBJDIR)/%.rel : $(MOSROOT)/%.c 
	$(Print) "CC $<"
	$(_QUIET) $(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(OBJDIR)/%.rel : $(PROJDIR)/%.c 
	$(Print) "CC $<"
	$(_QUIET) $(CC) $(CFLAGS) $(INCLUDES) $< -o $@

# .rel -> .ihx (nrf)
$(OUTDIR)/$(APPMOD).ihx : $(subst .o,.rel,$(OBJS))
	$(Print) "LD $@"
	$(_QUIET) $(LINKER) "$(LDFLAGS)" $^ -o $@


# Create target directory
$(OUTDIR):
	$(_QUIET) $(MakeDir) $@

objDirs: 
	$(_QUIET) $(MakeDir) $(OBJDIRS)

