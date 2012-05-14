# -*- coding: utf-8 -*-
#
# Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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

from os import path

from globals import * #@UnusedWildImport

class GenerateMakefile():
    def __init__(self):
        pass

    def generate(self, fileName = 'main.c', projectType = MANSOS_PROJECT,
                 pathToMansos = '', appName = ''):
        if not path.exists("Makefile") or not path.isfile("Makefile"):
            if projectType == SEAL_PROJECT:
                sourceType = "SEAL_SOURCES"
                if appName == '': appName = 'SealApp'
            else:
                sourceType = "SOURCES"
                if appName == '': appName = 'CApp'
            print "Generating Makefile"
            with open("Makefile", "w") as out:
                out.write("""#-*-Makefile-*- vim:syntax=make
#
# --------------------------------------------------------------------
#  The developer must define at least SOURCES and APPMOD in this file
#
#  In addition, PROJDIR and MOSROOT must be defined, before including 
#  the main Makefile at ${MOSROOT}/mos/make/Makefile
# --------------------------------------------------------------------

# Sources are all project source files, excluding MansOS files
""" + sourceType + " = " + fileName + """

# Module is the name of the main module built by this makefile
APPMOD = """ + appName + """

# --------------------------------------------------------------------
# Set the key variables
PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = """ + path.realpath(pathToMansos) + """
endif

# Include the main makefile
include ${MOSROOT}/mos/make/Makefile
""")
