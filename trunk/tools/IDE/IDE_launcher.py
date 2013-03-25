#!/usr/bin/env python

import os
from sys import path, version, executable, argv

def importsOk():
    wxModuleOK = True       # wx widgets
    serialModuleOK = True   # serial port communication
    plyModuleOK = True      # Python Lex Yacc - for compilation
    sealParserOK = True     # SEAL parser
    mansOSOK = True         # MansOS
    motelistOK = True       # Motelist

    try:
        import wx #@UnusedImport
    except ImportError:
        wxModuleOK = False

    try:
        import serial #@UnusedImport
    except ImportError:
        serialModuleOK = False

    try:
        import ply #@UnusedImport
    except ImportError:
        plyModuleOK = False

    try:
        # Add SEAL parser to python path
        sealPath = os.path.join(os.getcwd(), '..', '..', 'tools')
        path.append(sealPath)
        path.append(os.path.join(sealPath, 'seal', 'components'))
        #from seal.seal_parser import SealParser #@UnusedImport
        #import seal.seal_parser #@UnusedImport
        from seal import seal_parser #@UnusedImport
    except ImportError:
        sealParserOK = False
    except OSError:
        sealParserOK = False

    try:
        motelistPath = os.path.join(os.getcwd(), '..', '..', 'tools', "motelist")
        path.append(motelistPath)

        from motelist import Motelist #@UnusedImport
    except:
        motelistOK = False

    versionFile = os.path.join("../..", "doc/VERSION")

    if not os.path.exists(versionFile) or not os.path.isfile(versionFile):
        mansOSOK = False
    else:
        f = open(versionFile, "r")
        print ("Using MansOS:\n\tVersion: {0}\n\tRelease date: {1}".format(\
                                 f.readline().strip(), f.readline().strip()))
        f.close()

    if not (wxModuleOK and serialModuleOK and plyModuleOK and sealParserOK and mansOSOK and motelistOK):
        if os.name == 'posix':
            installStr = "Make sure you have installed required modules. Run:\n\tsudo apt-get install"
        else:
            installStr = "Make sure you have installed modules:"

        print ("Cannot run MansOS IDE:")

        if not wxModuleOK:
            print ("\twx module not found")
            installStr += " python-wxtools"

        if not serialModuleOK:
            print ("\tserial module not found")
            installStr += " python-serial"

        if not plyModuleOK:
            print ("\tPLY module not found")
            installStr += " python-ply"

        if not sealParserOK:
            print ("\tSEAL parser not found!")

        if not motelistOK:
            print ("\Motelist not found!")

        if not mansOSOK:
            print ("\tMansOS not found!")

        print (installStr)

        return False
    return True

def getUserInput(prompt):
    if version[0] >= '3':
        return input(prompt)
    else:
        return raw_input(prompt)

try:
    if not version.startswith("2.7"):
        print ("You are using Python version {0}".format(version[:5]))
        print ("MansOS IDE is tested only under version 2.7, continue at your own risk.")

        inp = getUserInput("Continue? (Y/n)\n")

        if inp.lower().strip() == "n":
            print ("")
            exit(1)
    try:
        # Go to real directory for imports to work
        os.chdir(os.path.dirname(os.path.realpath(__file__)))
        if not importsOk():
            getUserInput("Press enter to exit...")
            exit(1)

        print ("Launching MansOS IDE...")
        os.execl(executable, executable, * ['IDE.pyw'])
    except Exception as e:
        print ("Oops! something went wrong!")
        print (e)
        getUserInput("Press enter to exit...")
except Exception as e:
    print (e)
    getUserInput("Press enter to exit...")
