#!/usr/bin/env python
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

import os

def main():
    if not importsOk():
        exit(1)
    # Go to real directory for import to work
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    from src import api_core
    import wx

    ex = wx.App(redirect = False)

    API = api_core.ApiCore()
    API.frame.Show()

    ex.MainLoop()

def importsOk():
    wxModuleOK = True      # wx widgets
    serialModuleOK = True  # serial port communication
    plyModuleOK = True     # Python Lex Yacc - for compilation

    try:
        import wx
    except ImportError:
        wxModuleOK = False

    try:
        import serial
    except ImportError:
        serialModuleOK = False

    try:
        import ply
    except ImportError:
        plyModuleOK = False

    if not (wxModuleOK and serialModuleOK and plyModuleOK):
        if os.name == 'posix':
            installStr = "Make sure you have installed required modules. Run:\n\tapt-get install"
        else:
            installStr = "Make sure you have installed modules:"

        print "Cannot run MansOS IDE:"

        if not wxModuleOK:
            print "\twx module not found"
            installStr += " python-wxtools"

        if not serialModuleOK:
            print "\tserial module not found"
            installStr += " python-serial"

        if not plyModuleOK:
            print "\tPLY module not found"
            installStr += " python-ply"

        print installStr
        return False
    return True

if __name__ == '__main__':
    main()
