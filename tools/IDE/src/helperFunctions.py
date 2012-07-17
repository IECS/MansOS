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

"""
 -> Functions here are usually running in other threads and they can't be part 
 -> of any wx class, because it is impossible to pickle them. 
 -> Communication to outside uses pipes only!
 -> Output is string while running and int when done: 0 - success, other - fail!
"""

from subprocess import Popen, PIPE, STDOUT
from time import sleep
def doPopen(pipe, args):
    try:
        proc = Popen(args, stderr = STDOUT, stdout = PIPE)
        out = proc.stdout.readline()
        while out:
            pipe.send(out)
            out = proc.stdout.readline()
        proc.wait()
        pipe.send(proc.returncode)
    except OSError, e:
            print e

from serial import Serial, PARITY_NONE, SerialException

def listenSerialPort(pipe, args):
    try:
        ser = Serial(args['serialPort'], args['baudrate'], timeout = 0,
                           parity = PARITY_NONE, rtscts = 1)
        while True:
            s = ser.read(100)
            if len(s) > 0:
                print s
                pipe.send(s)
            sleep(0.01)
        ser.close()
    except SerialException as msg:
        print "\nSerial exception:\n\t", msg
        pipe.send("\nError conecting to device '{}'!\n".format(args['serialPort']))

