#!/usr/bin/env python

#
# Copyright (c) 2010-2012 the MansOS team. All rights reserved.
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
# author: Girts

# parse symbols in ELF image, display variable addresses

import sys, subprocess, string, re, commands

PRINT_DATA_MEMORY = True
PRINT_CODE_MEMORY = False

if len(sys.argv) != 3:
    print 'Usage: ' + sys.argv[0] + ' <target> <arch>'
    sys.exit(1)

target = sys.argv[1]
arch = sys.argv[2]

# find compiler and objdump executables
if arch == 'pc':
    objdump = 'objdump'
elif arch == 'msp430':
    objdump = 'msp430-objdump'
elif arch == 'avr':
    objdump = 'avr-objdump'
else:
    print "Error: unknown arhitecture!"
    sys.exit(1)


def readObjFile(file):
    try:
        [res, output] = commands.getstatusoutput(objdump + ' -x ' + file)
    except OSError, e:
        print >>sys.stderr, "objdump execution failed:", e
        sys.exit(1)
    return output.split('\n')


def getSectionSize(lines, section):
    regexp = '^\s*[0-9]+\s+' + section + '\s+([0-9a-f]+)\s'
    for line in lines:
        m = re.search(regexp, line)
        if not m is None:
            return int(m.group(1), 16)
    return 0


def getSymbols(lines, sections, description):
    regexp = '([0-9a-f]{8}).* +[a-z]+ +[a-zA-Z]? ' + sections + '.([0-9a-f]{8}) (.+)'
    sl = {}
    sizes = {}
    for line in lines:
        m = re.search(regexp, line)
        if not m is None:
            addr = m.group(1)
            size = int(m.group(3), 16)
            name = m.group(4)
            if size != 0:
                sl[addr] = name
                sizes[addr] = size

    addrs = sl.keys()
    sortedA = sorted(addrs)
    print description + ':'
    for a in sortedA:
        size = sizes[a]
        print a + ' [', size, '] ->', sl[a]


lines = readObjFile(target)
totalRamSize = getSectionSize(lines, '\.bss') + getSectionSize(lines, '\.data')
totalCodeSize = getSectionSize(lines, '\.text') + getSectionSize(lines, '\.usertext')
if PRINT_DATA_MEMORY:
    getSymbols(lines, '(\.bss|\.data)', 'Variables')
    print '-----------------------------'
    print 'Total size in RAM:', totalRamSize, 'bytes'
    print '-----------------------------'
if PRINT_CODE_MEMORY:
    getSymbols(lines, '(\.text)', 'Functions')
    print '-----------------------------'
    print 'Total size in code memory:', totalCodeSize, 'bytes'
    print '-----------------------------'
