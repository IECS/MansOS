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

from os import chdir, path
from subprocess import Popen, PIPE, STDOUT

from generate_makefile import GenerateMakefile

class DoCompile():
    def __init__(self, API):
        self.API = API
        self.generateMakefile = GenerateMakefile()

    def doCompile(self, sourceFileName, platform, filePath, projectType,
                  cleanAfter = True):

        chdir(path.split(path.realpath(filePath))[0])

        self.generateMakefile.generate(sourceFileName, projectType,
                                       self.API.path + "/../../")
        try:
            compiler = Popen(["make", platform],
                                      stderr = STDOUT,
                                      stdout = PIPE)
            out = compiler.communicate()[0]
            if cleanAfter:
                self.clean()
            if out.rfind("saving Makefile.platform") == -1:
                return [False, out]
            else:
                return [True, out]

        except OSError, e:
            print "execution failed @ compile:", e
            return [e]

    def clean(self):
        clean = Popen(["make", "clean"], stderr = STDOUT, stdout = PIPE)
        clean.communicate()
