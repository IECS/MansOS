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

import sys, subprocess, string, re

if len(sys.argv) != 6:
    print 'Usage: ' + sys.argv[0] + ' <arch> <target> <flags> <app_objects> <mos_objects>'
    sys.exit(1)

arch = sys.argv[1]
target = sys.argv[2]
flags = sys.argv[3]
app_objects = sys.argv[4].split()
system_objects = sys.argv[5].split()
unresolved = set(['main'])

# find compiler and objdump executables
if arch == 'pc':
    cc = 'gcc'
    objdump = 'objdump'
elif arch == 'msp430':
    cc = 'msp430-gcc'
    objdump = 'msp430-objdump'
elif arch == 'avr':
    cc = 'avr-gcc'
    objdump = 'avr-objdump'
else:
    print "Error: unknown arhitecture!"
    sys.exit(1)


# From MSP-GCC FAQ, page 23
lib_exported = ['abort', 'abort', 'abs', 'atoi', 'atol', 'atol', 'bcmp', 'bcopy', 'bsearch', 'bzero', 'errno', 'exit', 'ffs', 'labs', 'ldiv', 'malloc', 'memccpy', 'memchr', 'memcmp', 'memcpy', 'memmove', 'memset', 'qsort', 'rand', 'rindex', 'setjmp', 'snprintf', 'sprintf', 'strcasecmp', 'strcat', 'strchr', 'strcmp', 'strcpy', 'strcspn', 'strdup', 'strlcat', 'strlcpy', 'strlen', 'strncat', 'strncmp', 'strncpy', 'strpbrk', 'strrchr', 'strsep', 'strspn', 'strstr', 'strtok', 'strtol', 'strtoul', 'swab', 'vsnprintf', 'vsprintf']

if arch == 'pc':
    lib_exported.extend(['atexit', 'sleep', 'usleep', 'pthread_exit', 'stdout', 'fgetc', 'sem_post', 'sem_trywait', 'fclose', 'pthread_mutex_lock', 'pthread_attr_init', 'printf', '__printf_chk', '__fprintf_chk', '__vsprintf_chk', 'pthread_create', 'fflush', 'fopen', 'feof', 'pthread_cond_init', 'pthread_attr_setschedparam', 'sem_getvalue', 'pthread_equal', 'sem_destroy', 'perror', 'puts', 'sem_wait', '__stack_chk_fail', 'pthread_mutex_init', 'pthread_cond_wait', 'sem_init', 'pthread_mutex_unlock', 'pthread_self', 'htonl', 'read', 'gettimeofday', 'abort', 'connect', 'getsockname', 'close', 'htons', 'setsockopt', 'poll', 'putchar', 'socket', 'fwrite', 'fseek', 'fread', '__xstat', 'rewind', '__errno_location', 'strerror', 'write', 'ntohs', 'stderr', 'socketpair', 'free', '__memcpy_chk', '__vsnprintf_chk', 'fileno', 'ferror', '__memset_chk', 'ftruncate', 'select', 'pthread_join', '__snprintf_chk'])
elif arch == 'msp430':
    lib_exported.extend(['__udivmodsi4', '__udivmodhi4', '__mulsi3','__mulhi3', '__umulhisi3', '__umulsi3hw', '__umulqihi3', '__udivmodqi4', '__divmodsi4', '__mulqihi3', '__divmodhi4', '__stop_progExec__', '_end', '__stack', '__divsf3', '__floatunsisf', '__addsf3', '__U1CTL', '__U0TXBUF', '__TBCCTL0', '__IE1', '__IE2', '__U0RCTL', '__P5OUT', '__DCOCTL', '__ME1', '__ME2', '__U0CTL', '__TBCCR0', '__U1BR1', '__U1BR0', '__U1MCTL', '__U1TCTL', '__WDTCTL', '__U1TXBUF', '__BCSCTL1', '__TACCTL0', '__TACCTL2', '__P3DIR', '__TAR', '__TACCTL1', '__TACTL', '__U0MCTL', '__P3SEL', '__I2CTCTL', '__TBR', '__P5IN', '__U1RCTL', '__U0RXBUF', '__TAIV', '__TBCTL', '__U0TCTL', '__U1RXBUF', '__P3OUT', '__IFG1', '__IFG2', '__P5DIR', '__BCSCTL2', '__TACCR1', '__TACCR0', '__U0BR0', '__U0BR1', '__P2IN', '__P2IE', '__P4IN', '__P2DIR', '__P2OUT', '__ADC12IE', '__P1DIR', '__P1OUT', '__P4DIR', '__FCTL3', '__P1IFG', '__FCTL2', '__P4OUT', '__P2IFG', '__ADC12MEM2', '__FCTL1', '__ADC12MCTL2', '__P1IE', '__ADC12IFG', '__P1IN', '__ADC12CTL1', '__ADC12CTL0', '__P1IES'])
#    lib_exported.extend(['mrf24j40PollForPacket']) # XXX: hack for SADmote
    unresolved.add('alarmTimerInterrupt')          # TODO: don't do this if USE_HARDWARE_TIMERS=n
elif arch == 'avr':
    lib_exported.extend(['__do_copy_data', '__do_clear_bss', '__udivmodsi4', '__udivmodhi4', ' __divmodhi4', '__mulsi3', '__divmodhi4', '_end']);
    unresolved.add('__vector_11') 

def get_symbols(file):
    exported = []
    unresolved = []
    try:
        output = subprocess.Popen([objdump, '-t', file], stdout=subprocess.PIPE).communicate()[0]
    except OSError, e:
        print >>sys.stderr, "objdump execution failed:", e
        sys.exit(1)

    lines = output.split('\n')
    for line in lines:
        # exported functions
        m = re.search('\sg\s*F\s.*\s([a-zA-Z0-9_]*)$', line)
        if not m is None:
            exported.append(m.group(1))
        # exported variables
        m = re.search('\sO\s.*\s([a-zA-Z0-9_]*)$', line)
        if not m is None:
            exported.append(m.group(1))
        # unresolved symbols
        m = re.search('^[0-9a-fA-F]*\s*\*UND\*\s.*\s([a-zA-Z0-9_]*)$', line)
        if not m is None:
            unresolved.append(m.group(1))

    return [exported, unresolved]


def check_objdump_present():
    try:
        output = subprocess.Popen([objdump], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
        return True
    except OSError, e:
        print >>sys.stderr, "Warning: failed to execute obdjump ('" + objdump + "'):", e
        return False


def find_needed_objects(app_o, system_o):
    result = app_o
    if not check_objdump_present():
        # return all files in case objdump is not present
        result.extend(system_o)
        return result

    global unresolved
    exported = set(lib_exported)
    for o in app_o:
        [exported_in_obj, unres_in_obj] = get_symbols(o)
        for fun in unres_in_obj:
            if fun not in exported:
                unresolved.add(fun)
        for fun in exported_in_obj:
            if fun in unresolved:
                unresolved.remove(fun)
            exported.add(fun)

    exported_in_file_dict = dict()
    unres_in_file_dict = dict()
    for o in system_o:
        [exported_in_obj, unres_in_obj] = get_symbols(o)
        exported_in_file_dict[o] = set(exported_in_obj)
        unres_in_file_dict[o] = set(unres_in_obj)

    remaining_system_files = set(system_o)
    progress = True
    while len(unresolved) != 0 and len(remaining_system_files) != 0 and progress:
        #print 'one step, unresolved = '
        #print unresolved

        progress = False

        # find one file that exports some of the unresolved symbols...
        # add it to files that need to be linked,
        # and remove symbols it provides from the 'unresolved' set.
        for o in remaining_system_files:
            exp_from_file = exported_in_file_dict.get(o)
            if len(unresolved.intersection(exp_from_file)) == 0: continue

            #print 'process file ' + o + '\n'

            unresolved = unresolved.difference(exp_from_file)
            remaining_system_files.remove(o)

            unres_from_file = unres_in_file_dict.get(o)
            unresolved = unresolved.union(
                unres_from_file.difference(exported))

            exported = exported.union(exp_from_file)
            result.append(o)
            progress = True
            break

    if len(unresolved) != 0 and arch != 'waspmote':
        print "Warning: not all symbols found! Still unresolved:"
        print unresolved

    #print 'files unused: '
    #print string.join(remaining_system_files)

    return result

used_objects = find_needed_objects(app_objects, system_objects)

arglist = [cc, string.join(used_objects), '-o', target, flags]

#print 'arglist:'
#print string.join(arglist)

try:
    retcode = subprocess.call(string.join(arglist), shell=True)
except OSError, e:
    print >>sys.stderr, "gcc execution failed:", e
    retcode = 1

sys.exit(retcode)

